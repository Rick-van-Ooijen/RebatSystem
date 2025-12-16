#ifndef RBINTERPRETER_H
#define RBINTERPRETER_H

#include <godot_cpp/classes/Node.hpp>
#include <string>
#include <vector>
#include <unordered_map>

namespace godot {

class Expr;
class Binary;
class Grouping;
class Literal;
class Assign;
class Unary;
class Variable;
class Stmt;
class Token;
class Var;
class Block;
class IfStmt;

class Environment :  public Object {
	GDCLASS(Environment, Object)

public:
	Environment() {};
	Environment(Environment* iEnclosing) { enclosing = iEnclosing;};
	~Environment() {};

	Environment* enclosing = nullptr;
	std::unordered_map<std::string, std::string> values;

	std::string get(Token* name);

	void define(std::string name, std::string value)
	{
		values.insert_or_assign(name, value);
	}

	bool assign(std::string name, std::string value)
	{
		if (values.find(name) != values.end())
		{
			values.insert_or_assign(name, value);
			return true;
		}
		else
		{
			return false;
		}
	}

protected:
	static void _bind_methods() {};
};


class RBInterpreter : public Node {
	GDCLASS(RBInterpreter, Node)
	
	private:
	
	Environment* environment = new Environment;
	
	protected:
	static void _bind_methods();
	
	void runFile(std::string path);
	
	public:
	RBInterpreter();
	~RBInterpreter();
	
	void _process(double delta);
	void reportError(int line, std::string message);
	
	//see if this is needed in cpp, otherwise remove
	void main(String arg);
	void run(std::string input);


	std::string evaluate(Expr* expr);

	std::string print(Expr* iExpr);

	std::string visitBinaryExpr(Binary* expr);

	std::string visitGroupingExpr(Grouping* expr);

	std::string visitLiteralExpr(Literal* expr);

	std::string visitAssignExpr(Assign* expr);

	std::string visitUnaryExpr(Unary* expr);

	std::string visitVariableExpr(Variable* expr);

	std::string visitBlock(Block* stmt);

	std::string visitExpression(Stmt* stmt);

	std::string visitIf(IfStmt* stmt);

	std::string visitPrint(Stmt* stmt);

	std::string visitVar(Var* stmt);

	bool isTrue(Expr* expr);

	bool isNum(std::string input) {
		bool hasDot = false;
		bool couldHaveMinus = true;

		for (char ch : input) {
			if (!(isdigit(ch)))
			{

				// check for dots.
				if((ch == '.') && !(ch == '-' && couldHaveMinus))
				{
					if (hasDot)
					{return false;}

					hasDot = true;
				}
				else
				{
					// if it's a '-' at the start, that's still a valid number
					if(!(ch == '-' && couldHaveMinus))
					{
						return false;
					}
				}

			}

			couldHaveMinus = false;

		}
		return true;
	}
	

	std::string boolToStr(bool input) {
		if (input)
			{return "true";}
		else
			{return "false";}
	};

	std::string executeBlock(std::vector<Stmt*> statements, Environment* iEnvironment);

};





}
#endif