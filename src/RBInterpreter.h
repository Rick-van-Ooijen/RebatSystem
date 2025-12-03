#ifndef RBINTERPRETER_H
#define RBINTERPRETER_H

#include <godot_cpp/classes/Node.hpp>
#include <string>
#include <vector>
#include <unordered_map>

namespace godot {

enum TokenType {
	// Single-character tokens.
	T_LEFT_PAREN, T_RIGHT_PAREN, T_LEFT_BRACE, T_RIGHT_BRACE,
	T_COMMA, T_DOT, T_MINUS, T_PLUS, T_SEMICOLON, T_SLASH, T_STAR,
	// One or two character tokens.
	T_BANG, T_BANG_EQUAL,
	T_EQUAL, T_EQUAL_EQUAL,
	T_GREATER, T_GREATER_EQUAL,
	T_LESS, T_LESS_EQUAL,
	// Literals.
	T_IDENTIFIER, T_STRING, T_NUMBER,
	// Keywords.
	T_AND, T_CLASS, T_ELSE, T_FALSE, T_FUN, T_FOR, T_IF, T_NIL, T_OR,
	T_PRINT, T_RETURN, T_SUPER, T_THIS, T_TRUE, T_VAR, T_WHILE,
	T_EOF
};




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
};

class Token : public Object{
	GDCLASS(Token, Object)
public:
	int type;
	std::string lexeme;
	std::string literal;
	int line;

	Token() {};
	~Token() {};
	Token(int iType, std::string iLexeme, std::string iLiteral, int iLine) {
	type = iType;
	lexeme = iLexeme;
	literal = iLiteral;
	line = iLine;
	}

	std::string toString() {
		std::string output = ("line: " +std::to_string(line) + "  type: " + TypeNames[type] + "   lex: " + lexeme + "   literal: " + literal);
		return output;}

protected:
	static void _bind_methods() {};


private:
	const std::string TypeNames[39] = {
	// Single-character tokens.
	"LEFT_PAREN", "RIGHT_PAREN", "LEFT_BRACE", "RIGHT_BRACE",
	"COMMA", "DOT", "MINUS", "PLUS", "SEMICOLON", "SLASH", "STAR",
	// One or two character tokens.
	"BANG", "BANG_EQUAL",
	"EQUAL", "EQUAL_EQUAL",
	"GREATER", "GREATER_EQUAL",
	"LESS", "LESS_EQUAL",
	// Literals.
	"IDENTIFIER", "STRING", "NUMBER",
	// Keywords.
	"AND", "CLASS", "ELSE", "FALSE", "FUN", "FOR", "IF", "NIL", "OR",
	"PRINT", "RETURN", "SUPER", "THIS", "TRUE", "VAR", "WHILE",
	"EOF"
};
};

class Scanner : public Object{
	GDCLASS(Scanner, Object)
public:
	std::vector<Token> tokens;
	int start = 0;
	int current = 0;
	int line = 1;

	std::string source;
	RBInterpreter* interpreter;

	Scanner() {};
	~Scanner() {};
	Scanner(std::string iSource, RBInterpreter* iInterpreter) {source = iSource; interpreter = iInterpreter;}

	std::vector<Token> scanTokens(std::string source);
	void string();
	void number();
	void identifier();


	void addToken(int type) {addToken(type, "");};
	void addToken(int type, std::string literal);

protected:
	static void _bind_methods() {};


std::unordered_map<std::string, int> keywords = {
		{"and", TokenType::T_AND},
		{"class", TokenType::T_CLASS},
		{"else", TokenType::T_ELSE},
		{"false", TokenType::T_FALSE},
		{"for", TokenType::T_FOR},
		{"fun", TokenType::T_FUN},
		{"if", TokenType::T_IF},
		{"nil", TokenType::T_NIL},
		{"or", TokenType::T_OR},
		{"print", TokenType::T_PRINT},
		{"return", TokenType::T_RETURN},
		{"super", TokenType::T_SUPER},
		{"this", TokenType::T_THIS},
		{"true", TokenType::T_TRUE},
		{"var", TokenType::T_VAR},
		{"while", TokenType::T_WHILE}
	};
};

class AstPrinter;

class Expr : public Object {
	GDCLASS(Expr, Object)
public:
	Expr() {};
	~Expr() {};

	virtual std::string accept(AstPrinter* printer) {return "";};

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

protected:
	static void _bind_methods() {};
};




class AstPrinter : public Object {
	GDCLASS(AstPrinter, Object)
public:

	AstPrinter() {};
	~AstPrinter() {};

	std::string print(Expr* iExpr)
	{
		return iExpr->accept(this);
	}

	std::string visitBinaryExpr(Binary* expr) {
		return ("(" + expr->mOperator->lexeme + " " + print(expr->left) + " " + print(expr->right) + ")");
	}

	std::string visitGroupingExpr(Grouping* expr) {
		return ("(group " + print(expr->expression) + ")");
	}

	std::string visitLiteralExpr(Literal* expr) {
		if ( expr->value == "")
		{
			return "nil";
		}
		return expr->value;
	}

	std::string visitUnaryExpr(Unary* expr) {
		return ("(" + expr->mOperator->lexeme + " " + print(expr->right) + ")");
	}


protected:
	static void _bind_methods() {};
};


class Parser : public Object {
	GDCLASS(Parser, Object)
public:

	Parser() {};
	Parser(std::vector<Token> iTokens) { tokens = iTokens; };
	~Parser() {};

	std::vector<Token> tokens;
	int current = 0;

	Expr* expression() {
		return equality();
	}

	Expr* equality() {
		Expr* expr = new Expr;//comparison();
		while (match(std::vector<int>(TokenType::T_BANG_EQUAL, TokenType::T_EQUAL_EQUAL))) {
			Token* lOperator = &tokens[current-1];
			Expr* right = new Expr;//comparison(); //we get later
			expr = new Binary(expr, lOperator, right);
		}
		return expr;
	}


	bool match(std::vector<int> types) {
		for (int type : types) {
			if (type == (tokens[current].type)) {
				current++;
				return true;
			}
		}
		return false;
	}



protected:
	static void _bind_methods() {};
};





}
#endif