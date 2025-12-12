#include "parser.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cctype>

using namespace godot;

std::vector<Stmt*> Parser::parse()
{
	std::vector<Stmt*> statements;
	while (current < tokens.size()) {

		try
		{
			Stmt* stmt = declaration();
			if (stmt != nullptr)
			{
				statements.push_back(stmt);
			}
		}
		catch (int error) { synchronize(); }

		//UtilityFunctions::print(("current: " + std::to_string(current)).c_str());
		//UtilityFunctions::print(("tokens size: " + std::to_string(tokens.size())).c_str());

	}

	return statements;
}

Expr* Parser::expression() {
	return equality();
}

Expr* Parser::equality() {
	Expr* expr = comparison();
	while (match({TokenType::T_BANG_EQUAL, TokenType::T_EQUAL_EQUAL})) {
		Token* lOperator = tokens[current-1];
		Expr* right = comparison();
		expr = new Binary(expr, lOperator, right);
	}
	return expr;
}

Expr* Parser::comparison() {
	Expr* expr = term();
	while (match( {TokenType::T_GREATER, TokenType::T_GREATER_EQUAL, TokenType::T_LESS, TokenType::T_LESS_EQUAL} )) {
		Token* lOperator = tokens[current-1];
		Expr* right = term();
		expr = new Binary(expr, lOperator, right);
	}
	return expr;
}

Expr* Parser::term() { //addition or subtraction
	Expr* expr = factor();
	while (match( {TokenType::T_MINUS, TokenType::T_PLUS} )) {
		Token* lOperator = tokens[current-1];
		Expr* right = factor();
		expr = new Binary(expr, lOperator, right);
	}
	return expr;
}

Expr* Parser::factor() { // multiplication or division
	Expr* expr = unary();
	while (match( {TokenType::T_SLASH, TokenType::T_STAR} )) {
		Token* lOperator = tokens[current-1];
		Expr* right = unary();
		expr = new Binary(expr, lOperator, right);
	}
	return expr;
}

Expr* Parser::unary() {
	if (match( {TokenType::T_BANG, TokenType::T_MINUS} )) {
		Token* lOperator = tokens[current-1];
		Expr* right = unary();
		return new Unary(lOperator, right);
	}
	return primary();
}

Expr* Parser::primary() {
	if (match({TokenType::T_FALSE})) {return new Literal("false");};
	if (match({TokenType::T_TRUE})) {return new Literal("true");};
	if (match({TokenType::T_NIL})) {return new Literal("nil");};

	if (match({TokenType::T_NUMBER, TokenType::T_STRING})) {
		return new Literal(tokens[current-1]->literal);
	};

	if (match({TokenType::T_IDENTIFIER})) {
		return new Variable(tokens[current-1]);
	}

	if (match({TokenType::T_LEFT_PAREN})) {
		Expr* expr = new Expr;
		consume(TokenType::T_RIGHT_PAREN, "Expected ')' after expression.");
		return new Grouping(expr);
	};

	if (current < (tokens.size()-1))
	{
		interpreter->reportError(tokens[current-1]->line, "Expected expression.");
	}
	throw 0;


}

void Parser::synchronize() {
	current++;
	while (current < tokens.size())
	{
		if (tokens[current-1]->type == TokenType::T_SEMICOLON)
		{
			return;
		}
		switch (tokens[current]->type)
		{
			case TokenType::T_CLASS:
				return;
				break;
			case TokenType::T_FUN:
				return;
				break;
			case TokenType::T_VAR:
				return;
				break;
			case TokenType::T_FOR:
				return;
				break;
			case TokenType::T_IF:
				return;
				break;
			case TokenType::T_WHILE:
				return;
				break;
			case TokenType::T_PRINT:
				return;
				break;
			case TokenType::T_RETURN:
				return;
				break;;
			default:
			{
				current++;
			}
		}
	}
}


bool Parser::match(std::vector<int> types) {
	for (int type : types) {
		if (type == (tokens[current]->type)) {
			current++;
			return true;
		}
	}
	return false;
}


Token* Parser::consume(int type, std::string message) {
	if (tokens[current]->type == type) {current++; return tokens[current-1];}
	error(tokens[current], message);
	
}

void Parser::error(Token* token, std::string message) {
	std::string errorMessage = std::to_string(token->line);
	if (token->type == TokenType::T_EOF) {
		errorMessage += " at end";
	}
	else {
		errorMessage += (" at '" + token->lexeme + "'");
	}
	errorMessage += message;

	interpreter->reportError(token->line, errorMessage);
}

std::string Expression::accept(RBInterpreter* interpreter)
{
	return interpreter->visitExpression(this);
}
std::string Print::accept(RBInterpreter* interpreter)
{
	return interpreter->visitPrint(this);
}

std::string Var::accept(RBInterpreter* interpreter)
{
	return interpreter->visitVar(this);
}