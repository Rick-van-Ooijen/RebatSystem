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

	Stmt* statement() {
		if (match({TokenType::T_PRINT}))
		{ return printStatement(); }

		return expressionStatement();
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

/*	Expr* parse() {
		try
		{
			expression();
		}
		catch (int error) { synchronize(); return new Expr;  }
	}*/

	Expr* expression();
	Expr* equality();
	Expr* comparison();
	Expr* term();
	Expr* factor();
	Expr* unary();
	Expr* primary();
	void synchronize();
	bool match(std::vector<int> types);
	Token* consume(int type, std::string message);
	void error(Token* token, std::string message);

protected:
	static void _bind_methods() {};
};



}
#endif