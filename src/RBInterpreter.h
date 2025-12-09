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
class Unary;
class Stmt;

class RBInterpreter : public Node {
	GDCLASS(RBInterpreter, Node)
	
	private:
	
	
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

	std::string visitUnaryExpr(Unary* expr);

	std::string visitExpression(Stmt* stmt);

	std::string visitPrint(Stmt* stmt);

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

};





}
#endif