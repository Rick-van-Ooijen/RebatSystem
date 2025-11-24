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
	std::vector<Token> tokens = scanner.scanTokens(content);

	for(Token currentToken : tokens)
	{
		UtilityFunctions::print(currentToken.toString().c_str());
	}

}



void RBInterpreter::reportError(int line, std::string message)
{
	std::string newText = ("Line (" + std::to_string(line) + ") ERROR: " + message);
	UtilityFunctions::print(newText.c_str());

}


std::vector<Token> Scanner::scanTokens(std::string source)
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

	tokens.push_back(Token(TokenType::T_EOF, "", "", line));
	return tokens;
}


void Scanner::addToken(int type, std::string literal)
{
	std::string text = source.substr(start, (current - start));

	tokens.push_back(Token(type, text, literal, line));
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