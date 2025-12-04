#include "RBInterpreter.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cctype>
#include "parser.h"


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

