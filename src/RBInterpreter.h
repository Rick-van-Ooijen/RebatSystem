#ifndef RBINTERPRETER_H
#define RBINTERPRETER_H

#include <godot_cpp/classes/Node.hpp>
#include <string>
#include <vector>
#include <unordered_map>
#include <ctime>

namespace godot {

class Expr;
class Binary;
class Call;
class Grouping;
class Logical;
class Literal;
class Assign;
class Unary;
class Variable;
class Stmt;
class Token;
class Var;
class Block;
class IfStmt;
class While;
class LoxCallable;
class Function;
class Return;

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
			if (enclosing != nullptr)
				{return enclosing->assign(name, value);}
			return false;
		}
	}

	bool checkPresent(std::string name)
	{
		if (values.find(name) != values.end())
		{
			return true;
		}
		else
		{
			if (enclosing != nullptr)
				{return enclosing->checkPresent(name);}
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
	std::unordered_map<std::string, LoxCallable*> functions;
	
	protected:
	static void _bind_methods();
	
	void runFile(std::string path);
	
	public:
	Environment* globals = environment;
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

	std::string visitCallExpr(Call* expr);

	std::string visitGroupingExpr(Grouping* expr);

	std::string visitLogicalExpr(Logical* expr);

	std::string visitLiteralExpr(Literal* expr);

	std::string visitAssignExpr(Assign* expr);

	std::string visitUnaryExpr(Unary* expr);

	std::string visitVariableExpr(Variable* expr);

	std::string visitBlock(Block* stmt);

	std::string visitExpression(Stmt* stmt);

	std::string visitFunction(Function* stmt);

	std::string visitIf(IfStmt* stmt);

	std::string visitPrint(Stmt* stmt);

	std::string visitReturn(Return* stmt);

	std::string visitVar(Var* stmt);

	std::string visitWhile(While* stmt);

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


class LoxCallable :  public Object {
	GDCLASS(LoxCallable, Object)

public:
	LoxCallable() {};
	~LoxCallable() {};
	int arityNumber = 0;

	virtual std::string call(RBInterpreter* iInterpreter, std::vector<std::string> arguments) {return "";};

	int arity() {return arityNumber;};

	virtual std::string toString() {return "";};


protected:
	static void _bind_methods() {};
};

class UserFunction :  public LoxCallable {
	GDCLASS(UserFunction, LoxCallable)

public:
	UserFunction() {};
	UserFunction(std::vector<Stmt*> iBody, std::vector<std::string> iArgumentNames, Token* iName, Environment* iClosure);
	~UserFunction() {};

	std::vector<Stmt*> body;
	std::vector<std::string> argumentNames;
	Environment* clusure;
	
	std::string name;

	std::string call(RBInterpreter* iInterpreter, std::vector<std::string> arguments) override
	{
		
		Environment* environment = new Environment(clusure);
		for (int i = 0; i < arguments.size(); i++)
		{
			environment->define(argumentNames[i], arguments[i]);
		}

		try
		{
			iInterpreter->executeBlock(body, environment);
		}
		catch (std::string error)
		{
			return error;
		}
		
		return "";
	}

	std::string toString() override { return "<fn " + name + ">"; }


protected:
	static void _bind_methods() {};
};

class ClockFunction :  public LoxCallable {
	GDCLASS(ClockFunction, LoxCallable)

public:
	ClockFunction() {};
	~ClockFunction() {};

	std::string call(RBInterpreter* iInterpreter, std::vector<std::string> arguments) override
	{
		//std::time_t result = std::time(nullptr);
		time_t epoch = 0;
		std::string time = std::to_string((intmax_t)epoch);
		return time;
	}

	std::string toString() override { return "<native fn>"; }



protected:
	static void _bind_methods() {};
};


}
#endif