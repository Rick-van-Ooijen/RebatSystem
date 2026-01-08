#ifndef RBINTERPRETER_H
#define RBINTERPRETER_H

#include <godot_cpp/classes/Node.hpp>
#include <string>
#include <vector>
#include <unordered_map>
#include <ctime>
#include <chrono>

namespace godot {

class RBInterpreter;
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
class LoxClass;
class LoxInstance;
class Function;
class Return;
class Class;
class Get;
class Set;

class Environment :  public Object {
	GDCLASS(Environment, Object)

public:
	Environment() {};
	Environment(Environment* iEnclosing) { enclosing = iEnclosing;};
	~Environment() {};

	Environment* enclosing = nullptr;
	std::unordered_map<std::string, std::string> values;

	std::string get(Token* name, RBInterpreter* interpreter);

	void define(std::string name, std::string value);

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
	
	
	protected:
	static void _bind_methods();
	
	void runFile(std::string path);
	
	public:
	
	Environment* environment = new Environment;
	std::unordered_map<std::string, LoxCallable*> functions;
	std::unordered_map<std::string, LoxClass*> classes;
	std::unordered_map<std::string, LoxInstance*> instances;

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

	std::string visitGetExpr(Get* expr);

	std::string visitGroupingExpr(Grouping* expr);

	std::string visitLogicalExpr(Logical* expr);

	std::string visitSetExpr(Set* expr);

	std::string visitLiteralExpr(Literal* expr);

	std::string visitAssignExpr(Assign* expr);

	std::string visitUnaryExpr(Unary* expr);

	std::string visitVariableExpr(Variable* expr);

	std::string visitBlock(Block* stmt);

	std::string visitClass(Class* stmt);

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
		auto start = std::chrono::system_clock::now();

		std::time_t end_time = std::chrono::system_clock::to_time_t(start);

		
		std::string time = std::ctime(&end_time);
		return time;
	}

	std::string toString() override { return "<native fn>"; }



protected:
	static void _bind_methods() {};
};


class LoxClass :  public Object {
	GDCLASS(LoxClass, Object)

public:
	std::string name;
	std::unordered_map<std::string, LoxCallable*> methods;

	LoxClass() {};
	LoxClass(std::string iName, std::unordered_map<std::string, LoxCallable*> iMethods) {
		name = iName;
		methods = iMethods;	
	};
	~LoxClass() {};

	std::string toString() { return name; };
	LoxInstance* call(RBInterpreter* interpreter, std::vector<std::string> arguments);

	LoxCallable* findMethod(std::string name){
		auto found = methods.find(name);

		if (found != methods.end())
		{
			return found->second;
		}
		else
		{
			return nullptr;
		}

	}


protected:
	static void _bind_methods() {};
};

class LoxInstance :  public Object {
	GDCLASS(LoxInstance, Object)

public:
	LoxClass* klass;
	std::unordered_map<std::string, std::string> fields;


	LoxInstance() {};
	LoxInstance(LoxClass* iClass) { klass = iClass; };
	~LoxInstance() {};

	std::string toString() { return (klass->name + " instance");};

	std::string get(Token* name);

	void set(Token* name, std::string value);

	LoxCallable* method(std::string name){

		return klass->findMethod(name);

	}


protected:
	static void _bind_methods() {};
};
//LoxClass
//loxObject;
//classes
//new class
//like callable
//has callables

}
#endif