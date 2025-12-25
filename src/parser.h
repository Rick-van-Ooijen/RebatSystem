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

class Function : public Stmt {
	GDCLASS(Function, Stmt)
public:
	Token* name;
	std::vector<Token*> params;
	std::vector<Stmt*> body;

	Function() {};
	~Function() {};
	Function(Token* iName, std::vector<Token*> iParams, std::vector<Stmt*> iBody) {
		name = iName;
		params = iParams;
		body = iBody;
		expression = nullptr;
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

class Return : public Stmt {
	GDCLASS(Return, Stmt)
public:
	Token* keyword;


	Return() {};
	~Return() {};
	Return(Token* iKeyword, Expr* value) {
		keyword = iKeyword;
		expression = value;
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
		if (match({TokenType::T_FUN}))
			{ return function("function");}
		if (match({TokenType::T_VAR}))
			{ return varDeclaration(); }
		
		return statement();
	}

	Stmt* function(std::string kind) {
		Token* name = consume(TokenType::T_IDENTIFIER, ("Expect " + kind + " name."));
		consume(TokenType::T_LEFT_PAREN, ("Expect '(' after " + kind + " name."));

		std::vector<Token*> parameters;

		if(tokens[current]->type != TokenType::T_RIGHT_PAREN)
		{
			do {
				if (parameters.size() >= 255)
				{
					error(tokens[current], "Can't have more than 255 parameters.");
				}
				parameters.push_back(consume(TokenType::T_IDENTIFIER, "Expect parameter name."));

			}
			while(tokens[current]->type == TokenType::T_COMMA);
		}

	consume(TokenType::T_RIGHT_PAREN, "Expect ')' after parameters.");
	
	consume(TokenType::T_LEFT_BRACE, ("Expect '{' before " + kind + " body."));

	std::vector<Stmt*> body = block();

	return new Function(name, parameters, body);
	
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

		if (match({TokenType::T_RETURN}))
			{ return returnStatement(); }
			
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

	Stmt* returnStatement() {
		Token* keyword = tokens[current-1];
		Expr* value = nullptr;
		if (tokens[current]->type != TokenType::T_SEMICOLON)
		{
			value = expression();
		}
		consume(TokenType::T_SEMICOLON, "Expect ';' after return value.");
		return new Return(keyword, value);
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