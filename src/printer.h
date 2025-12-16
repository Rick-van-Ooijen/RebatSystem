#ifndef PRINTER_H
#define PRINTER_H

#include <godot_cpp/classes/Node.hpp>
#include <string>
#include <vector>
#include <unordered_map>
#include "scanner.h"

namespace godot {



class AstPrinter;

class Expr : public Object {
	GDCLASS(Expr, Object)
public:
	Expr() {};
	~Expr() {};

	virtual std::string accept(AstPrinter* printer) {return "";};
	virtual std::string acceptI(RBInterpreter* interpreter) {return "";};

protected:
	static void _bind_methods() {};
};

class Assign : public Expr {
	GDCLASS(Assign, Expr)
public:

	Token* name;
	Expr* value;

	Assign() {};
	~Assign() {};
	Assign(Token* iName, Expr* iValue) {
		name = iName;
		value = iValue;
	};

	std::string accept(AstPrinter* printer) override;
	std::string acceptI(RBInterpreter* interpreter) override;

protected:
	static void _bind_methods() {};
};

class Binary : public Expr {
	GDCLASS(Binary, Expr)
public:
	Expr* left;
	Token* mOperator;
	Expr* right;

	Binary() {};
	~Binary() {};
	Binary(Expr* iLeft, Token* iOperator, Expr* iRight) {
		left = iLeft;
		mOperator = iOperator;
		right = iRight;
	};

	std::string accept(AstPrinter* printer) override;
	std::string acceptI(RBInterpreter* interpreter) override;

protected:
	static void _bind_methods() {};
};



class Grouping : public Expr {
	GDCLASS(Grouping, Expr)
public:
	Expr* expression;

	Grouping() {};
	~Grouping() {};
	Grouping(Expr* iExpression) {
		expression = iExpression;
	};

	std::string accept(AstPrinter* printer) override;
	std::string acceptI(RBInterpreter* interpreter) override;



protected:
	static void _bind_methods() {};
};

class Logical : public Expr {
	GDCLASS(Logical, Expr)
public:
	Expr* left;
	Token* mOperator;
	Expr* right;

	Logical() {};
	~Logical() {};
	Logical(Expr* iLeft, Token* iOperator, Expr* iRight) {
		left = iLeft;
		mOperator = iOperator;
		right = iRight;
	};

	std::string accept(AstPrinter* printer) override;
	std::string acceptI(RBInterpreter* interpreter) override;


protected:
	static void _bind_methods() {};
};

class Literal : public Expr {
	GDCLASS(Literal, Expr)
public:
	std::string value;

	Literal() {};
	~Literal() {};
	Literal(std::string iValue) {
		value = iValue;
	};

	std::string accept(AstPrinter* printer) override;
	std::string acceptI(RBInterpreter* interpreter) override;


protected:
	static void _bind_methods() {};
};

class Unary : public Expr {
	GDCLASS(Unary, Expr)
public:

	Token* mOperator;
	Expr* right;

	Unary() {};
	~Unary() {};
	Unary(Token* iOperator, Expr* iRight) {
		mOperator = iOperator;
		right = iRight;
	};

	std::string accept(AstPrinter* printer) override;
	std::string acceptI(RBInterpreter* interpreter) override;

protected:
	static void _bind_methods() {};
};

class Variable : public Expr {
	GDCLASS(Variable, Expr)
public:

	Token* name;

	Variable() {};
	~Variable() {};
	Variable(Token* iName) {
		name = iName;
	};

	std::string accept(AstPrinter* printer) override;
	std::string acceptI(RBInterpreter* interpreter) override;

protected:
	static void _bind_methods() {};
};


class AstPrinter : public Object {
	GDCLASS(AstPrinter, Object)
public:

	AstPrinter() {};
	~AstPrinter() {};

	std::string print(Expr* iExpr)
	{ return iExpr->accept(this);}

	std::string visitBinaryExpr(Binary* expr)
		{return ("(" + expr->mOperator->lexeme + " " + print(expr->left) + " " + print(expr->right) + ")");}

	std::string visitGroupingExpr(Grouping* expr)
	{return ("(group " + print(expr->expression) + ")");}

	std::string visitLogicalExpr(Logical* expr)
	{return "";}


	std::string visitLiteralExpr(Literal* expr) {
		if ( expr->value == "")
			{return "nil";}
		return expr->value;
	}

	std::string visitUnaryExpr(Unary* expr)
	{return ("(" + expr->mOperator->lexeme + " " + print(expr->right) + ")");}

	std::string visitVariableExpr(Variable* expr)
	{return "";}

	std::string visitAssignExpr(Assign* expr)
	{return "";}

protected:
	static void _bind_methods() {};
};






}
#endif