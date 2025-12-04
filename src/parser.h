#ifndef PARSER_H
#define PARSER_H

#include <godot_cpp/classes/Node.hpp>
#include <string>
#include <vector>
#include <unordered_map>
#include "printer.h"

namespace godot {


class Parser : public Object {
	GDCLASS(Parser, Object)
public:

	Parser() {};
	Parser(std::vector<Token*> iTokens, RBInterpreter* iInterpreter) { tokens = iTokens; interpreter = iInterpreter; };
	~Parser() {};

	std::vector<Token*> tokens;
	RBInterpreter* interpreter;

	int current = 0;

	Expr* parse() {
		try
		{
			expression();
		}
		catch (int error) { synchronize(); return new Expr;  }
	}

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