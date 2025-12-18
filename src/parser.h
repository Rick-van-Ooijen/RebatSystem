#ifndef PARSER_H
#define PARSER_H

#include <godot_cpp/classes/Node.hpp>
#include <string>
#include <vector>
#include <unordered_map>
#include "printer.h"

namespace godot {

class Stmt : public Object {
	GDCLASS(Stmt, Object)
public:
	Expr* expression;

	Stmt() {};
	~Stmt() {};

	virtual std::string accept(RBInterpreter* interpreter) {return "";};


protected:
	static void _bind_methods() {};
};

class Block : public Stmt {
	GDCLASS(Block, Stmt)
public:

	std::vector<Stmt*> statements;

	Block() {};
	~Block() {};
	Block(std::vector<Stmt*> iStatements) {
		statements = iStatements;
	};

	std::string accept(RBInterpreter* interpreter) override;

protected:
	static void _bind_methods() {};
};

class Expression : public Stmt {
	GDCLASS(Expression, Stmt)
public:

	Expression() {};
	~Expression() {};
	Expression(Expr* iExpression) {
		expression = iExpression;
	};

	std::string accept(RBInterpreter* interpreter) override;

protected:
	static void _bind_methods() {};
};

class IfStmt : public Stmt {
	GDCLASS(IfStmt, Stmt)
public:

	Stmt* thenBranch;
	Stmt* elseBranch;

	IfStmt() {};
	~IfStmt() {};
	IfStmt(Expr* iExpression, Stmt* iThenBranch, Stmt* iElseBranch ) {
		expression = iExpression;
		thenBranch = iThenBranch;
		elseBranch = iElseBranch;
	};

	std::string accept(RBInterpreter* interpreter) override;

protected:
	static void _bind_methods() {};
};

class Print : public Stmt {
	GDCLASS(Print, Stmt)
public:

	Print() {};
	~Print() {};
	Print(Expr* iExpression) {
		expression = iExpression;
	};

	std::string accept(RBInterpreter* interpreter) override;

protected:
	static void _bind_methods() {};
};

class Var : public Stmt {
	GDCLASS(Var, Stmt)
public:
	Token* name;

	Var() {};
	~Var() {};
	Var(Expr* iInitializer, Token* iName) {
		expression = iInitializer;
		name = iName;
	};

	std::string accept(RBInterpreter* interpreter) override;

protected:
	static void _bind_methods() {};
};

class While : public Stmt {
	GDCLASS(While, Stmt)
public:
	Stmt* body;

	While() {};
	~While() {};
	While(Expr* iCondition, Stmt* iBody) {
		expression = iCondition;
		body = iBody;
	};

	std::string accept(RBInterpreter* interpreter) override;

protected:
	static void _bind_methods() {};
};


class Parser : public Object {
	GDCLASS(Parser, Object)
public:

	Parser() {};
	Parser(std::vector<Token*> iTokens, RBInterpreter* iInterpreter) { tokens = iTokens; interpreter = iInterpreter; };
	~Parser() {};

	std::vector<Token*> tokens;
	RBInterpreter* interpreter;

	int current = 0;

	std::vector<Stmt*> parse();

	Stmt* declaration() {
		if (match({TokenType::T_VAR}))
			{ return varDeclaration(); }
		
		return statement();
	}

	Stmt* varDeclaration()
	{
		Token* name = consume(TokenType::T_IDENTIFIER, "Expect variable name.");

		Expr* initializer;
		if (match({TokenType::T_EQUAL})) {
			initializer = expression();
		}

		consume(TokenType::T_SEMICOLON, "Expect ';' after variable declaration.");
		return new Var(initializer, name);
	}
	
	Stmt* statement() {
		if (match({TokenType::T_IF}))
			{ return ifStatement(); }
		
		if (match({TokenType::T_PRINT}))
			{ return printStatement(); }
			
		if (match({TokenType::T_FOR}))
			{ return forStatement(); }
			
		if (match({TokenType::T_WHILE}))
			{return whileStatement();}

		if (match({TokenType::T_LEFT_BRACE}))
			{ return new Block(block()); }

		return expressionStatement();
	}

	Stmt* ifStatement() {
		consume(TokenType::T_LEFT_PAREN, "Expect '(' after 'if'.");
		Expr* condition = expression();
		consume(TokenType::T_RIGHT_PAREN, "Expect ')' after if condition.");

		Stmt* thenBranch = statement();
		Stmt* elseBranch = nullptr;
		if (match({TokenType::T_ELSE})) {
			current++;
			elseBranch = statement();
		}
		return new IfStmt(condition, thenBranch, elseBranch);
	}

	Stmt* forStatement() {
		consume(TokenType::T_LEFT_PAREN, "Expect '(' after 'for'.");

		Stmt* initializer;
		if (match({TokenType::T_SEMICOLON}))
		{
			initializer = nullptr;
		}
		else if (match({TokenType::T_VAR}))
		{
			initializer = varDeclaration();
		}
		else
		{
			initializer = expressionStatement();
		}

		Expr* condition = nullptr;
		if (tokens[current]->type != TokenType::T_SEMICOLON)
		{
			condition = expression();
		}
		consume(TokenType::T_SEMICOLON, "Expect ';' after loop condition.");

		Expr* increment = nullptr;
		if (tokens[current]->type != TokenType::T_RIGHT_PAREN)
		{
			increment = expression();
		}
		consume(TokenType::T_RIGHT_PAREN, "Expect ')' after for clauses.");
		
		Stmt* body = statement();

		if (increment != nullptr) {
			body = new Block({ body, new Expression(increment)});
		}

		if (condition == nullptr) {condition = new Literal("true");}	// While true is risky without breaks
		body = new While(condition, body);

		if (initializer != nullptr) {
			body = new Block({ initializer, body });
		}

		return body;

	}

	Stmt* whileStatement() {
		consume(TokenType::T_LEFT_PAREN, "Expect '(' after 'while'.");
		Expr* condition = expression();
		consume(TokenType::T_RIGHT_PAREN, "Expect ')' after condition.");
		Stmt* body = statement();
		return new While(condition, body);
	}

	Stmt* printStatement() {
		Expr* value = expression();
		consume(TokenType::T_SEMICOLON, "Expect ';' after value.");
		return new Print(value);
	}

	Stmt* expressionStatement() {
		Expr* value = expression();
		consume(TokenType::T_SEMICOLON, "Expect ';' after value.");
		return new Expression(value);
	}

	Expr* expression();
	Expr* or();
	Expr* and();
	Expr* assignment();
	Expr* equality();
	Expr* comparison();
	Expr* term();
	Expr* factor();
	Expr* unary();
	Expr* call();
	Expr* finishCall(Expr* callee);
	Expr* primary();
	void synchronize();
	bool match(std::vector<int> types);
	Token* consume(int type, std::string message);
	void error(Token* token, std::string message);

	std::vector<Stmt*> block();


protected:
	static void _bind_methods() {};
};



}
#endif