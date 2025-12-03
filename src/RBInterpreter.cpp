#include "RBInterpreter.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cctype>

using namespace godot;

void RBInterpreter::_bind_methods() {

	ClassDB::bind_method(D_METHOD("runFile", "input"), &RBInterpreter::main);

}

RBInterpreter::RBInterpreter() {
	// Initialize any variables here.
}

RBInterpreter::~RBInterpreter() {
	// Add your cleanup here.
}

void RBInterpreter::_process(double delta) {
	//
}


void RBInterpreter::main(String arg)
{
	CharString temp = ((String)arg).utf8().get_data();
	char* value = (char*)(temp.get_data());
	std::string input = std::string(value);

	runFile(input);



}



void RBInterpreter::runFile(std::string path)
{
	//opens the file into content
	std::ifstream currentStream(path);
						
	std::stringstream buffer;
	buffer << currentStream.rdbuf();
	std::string content = buffer.str();


	Scanner scanner = Scanner(content, this);
	std::vector<Token*> tokens = scanner.scanTokens(content);

	for(Token* currentToken : tokens)
	{
		UtilityFunctions::print((*currentToken).toString().c_str());
	}

	
	Parser parser = Parser(tokens, this);

	/*// hack to create a syntax tree to print.
	
	Binary expression = Binary(
	new Unary(new Token(TokenType::T_MINUS, "-", "", 1), new Literal("123")),
	new Token(TokenType::T_STAR, "*", "", 1),
	new Grouping(new Literal("45.67")));*/

	AstPrinter printer;
	std::string text = printer.print(parser.parse());
	UtilityFunctions::print(text.c_str());

}



void RBInterpreter::reportError(int line, std::string message)
{
	std::string newText = ("Line (" + std::to_string(line) + ") ERROR: " + message);
	UtilityFunctions::print(newText.c_str());

}


std::vector<Token*> Scanner::scanTokens(std::string source)
{
	// go through each character
	while(current < source.length())
	{

		// take the character
		char c = source[current];

		// increase the counter to the next character not yet read.
		// the one read for c has been stepped past and consumed.
		current++;
		
		// make a token
		switch (c) {
			case '(': {addToken(TokenType::T_LEFT_PAREN);} break;
			case ')': {addToken(TokenType::T_RIGHT_PAREN);} break;
			case '{': {addToken(TokenType::T_LEFT_BRACE);} break;
			case '}': {addToken(TokenType::T_RIGHT_BRACE);} break;
			case ',': {addToken(TokenType::T_COMMA);} break;
			case '.': {addToken(TokenType::T_DOT);} break;
			case '-': {addToken(TokenType::T_MINUS);} break;
			case '+': {addToken(TokenType::T_PLUS);} break;
			case ';': {addToken(TokenType::T_SEMICOLON);} break;
			case '*': {addToken(TokenType::T_STAR);} break;
			case '!':
			{
				// if the next character matches with a two-character lexeme, add that token and increase the counter by one so the second character isn't read twice.
				if(source[current] == '=')
				{	current++;
					addToken(TokenType::T_BANG_EQUAL);}
				else {addToken(TokenType::T_BANG);};
			} break;
			case '=':
			{
				if(source[current] == '=')
				{	current++;
					addToken(TokenType::T_EQUAL_EQUAL);}
				else {addToken(TokenType::T_EQUAL);};
			} break;
			case '<':
			{
				if(source[current] == '=')
				{	current++;
					addToken(TokenType::T_LESS_EQUAL);}
				else {addToken(TokenType::T_LESS);};
			} break;
			case '>':
			{
				if(source[current] == '=')
				{	current++;
					addToken(TokenType::T_GREATER_EQUAL);}
				else {addToken(TokenType::T_GREATER);};
			} break;
			case '/':
			{
				if(source[current] == '/') // comment
				{
					// loop to end of line, so none of the characters will be read in further loops.
					while (source[current] != '\n' && !(current >= source.length())) 
					{
						current++;
					}
				}
				else {addToken(TokenType::T_SLASH);};
			} break;
			case ' ': {} break; // whitespace: do nothing
			case '\r': {} break;
			case '\t': {} break;
			case '\n': // linebreak: increase the counter to track what line we're on.
			{
				line++;
			} break;
			case '"': {string();} break;
			default:
			{
				if (std::isdigit(c)) // for numbers
				{
					number();
				}
				else if (std::isalpha(c)) // for identifiers
				{
					identifier();
				}
				else
				{
					interpreter->reportError(line, "Unexpected character.");
				}
			}
		}
	
		start = current;
	}

	tokens.push_back(new Token(TokenType::T_EOF, "", "", line));
	return tokens;
}


void Scanner::addToken(int type, std::string literal)
{
	std::string text = source.substr(start, (current - start));

	tokens.push_back( new Token(type, text, literal, line) );
}

void Scanner::string()
{
	// look further until we run into the next quotation mark.
	while (source[current] != '"' && !(current >= source.length())) 
	{
		if (char(source[current]) == '\n')
		{
			line++;
		}
		current++;
	}

	// error if we run into the end instead of a second quotation mark
	if(current >= source.length())
	{
		interpreter->reportError(line, "Unterminated string.");
	}

	
	std::string value = source.substr(start + 1, current - start - 1);
	current++;
	addToken(TokenType::T_STRING, value);
	
}

void Scanner::number()
{
	// as long as digits are there we continue.
	while (std::isdigit(source[current]) && source[current] != '\n') 
	{
		current++;
	}

	// if we find a dot and after it another digit, we go through every digit after the dot as well
	if (source[current] == '.' && std::isdigit(source[current+1]))
	{
		current++;
		while (std::isdigit(source[current])) 
		{
			current++;
		}
	}

	std::string value = source.substr(start, current - start);
	addToken(TokenType::T_NUMBER, value);
}

void Scanner::identifier()
{
	// continue through all alphanumeric characters
	while (std::isalnum(source[current]) && source[current] != '\n') 
	{
		current++;
	}
	
	std::string text = source.substr(start, current - start);
	
	// find the correct type based on keyword
	int type = keywords.find(text)->second;
	if (type >= 39)
	{
		type = TokenType::T_IDENTIFIER;
	}

	addToken(type, text);
}


std::string Binary::accept(AstPrinter* printer)
{
	return printer->visitBinaryExpr(this);
}

std::string Grouping::accept(AstPrinter* printer)
{
	return printer->visitGroupingExpr(this);
}

std::string Literal::accept(AstPrinter* printer)
{
	return printer->visitLiteralExpr(this);
}

std::string Unary::accept(AstPrinter* printer)
{
	return printer->visitUnaryExpr(this);
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

	if (match({TokenType::T_LEFT_PAREN})) {
		Expr* expr = new Expr;
		consume(TokenType::T_RIGHT_PAREN, "Expected ')' after expression.");
		return new Grouping(expr);
	};

	interpreter->reportError(tokens[current-1]->line, "Expected expression.");
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

