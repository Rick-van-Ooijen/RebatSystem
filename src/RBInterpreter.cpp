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

	//AstPrinter printer;
	//std::string text = printer.print(parser.parse());
	//UtilityFunctions::print(text.c_str());


	std::string output = evaluate(parser.parse());
	UtilityFunctions::print(output.c_str());

}



void RBInterpreter::reportError(int line, std::string message)
{
	std::string newText = ("Line (" + std::to_string(line) + ") ERROR: " + message);
	UtilityFunctions::print(newText.c_str());

}

std::string RBInterpreter::evaluate(Expr* expr)
{
	return expr->accept(this);
}

std::string RBInterpreter::visitBinaryExpr(Binary* expr)
{
	std::string left = evaluate(expr->left);
	std::string right = evaluate(expr->right);
	bool areNumbers = (isNum(left) && isNum(right));

	switch(expr->mOperator->type) {
		case TokenType::T_MINUS:
		{
			if (areNumbers)
			{
				double result = stod(left) - stod(right);
				return std::to_string(result);
			}
			else
			{
				reportError(expr->mOperator->line, "Operand must be a number.");
			}
		} break;
		case TokenType::T_SLASH:
		{
			if (areNumbers)
			{
				double result = stod(left) / stod(right);
				return std::to_string(result);
			}
			else
			{
				reportError(expr->mOperator->line, "Operand must be a number.");
			}
		} break;
		case TokenType::T_STAR:
		{
			if (areNumbers)
			{
				double result = stod(left) * stod(right);
				return std::to_string(result);
			}
			else
			{
				reportError(expr->mOperator->line, "Operand must be a number.");
			}
		} break;
		case TokenType::T_PLUS:
		{

			if (areNumbers)
			{
				double result = std::stod(left) + std::stod(right);
				return std::to_string(result);
			}
			else
			{
				return left + right;
			}

		} break;
		case TokenType::T_GREATER:
		{
			if (areNumbers)
			{
				bool result = (std::stod(left) > std::stod(right));
				return boolToStr(result);
			}
			else
			{
				reportError(expr->mOperator->line, "Operand must be a number.");
			}
		} break;
		case TokenType::T_GREATER_EQUAL:
		{
			if (areNumbers)
			{
				bool result = (std::stod(left) >= std::stod(right));
				return boolToStr(result);
			}
			else
			{
				reportError(expr->mOperator->line, "Operand must be a number.");
			}
		} break;
		case TokenType::T_LESS:
		{
			if (areNumbers)
			{
				bool result = (std::stod(left) < std::stod(right));
				return boolToStr(result);
			}
			else
			{
				reportError(expr->mOperator->line, "Operand must be a number.");
			}
		} break;
		case TokenType::T_LESS_EQUAL:
		{
			if (areNumbers)
			{
				bool result = (std::stod(left) <= std::stod(right));
				return boolToStr(result);
			}
			else
			{
				reportError(expr->mOperator->line, "Operand must be a number.");
			}
		} break;
		case TokenType::T_BANG_EQUAL:
		{
			bool result = (left != right);
			return boolToStr(result);
		} break;
		case TokenType::T_EQUAL_EQUAL:
		{
			bool result = (left == right);
			return boolToStr(result);
		} break;
		default:
			return "";
	}

	return "";
}

std::string RBInterpreter::visitGroupingExpr(Grouping* expr)
{
	return evaluate(expr->expression);
}

std::string RBInterpreter::visitLiteralExpr(Literal* expr)
{
	return expr->value;
}

std::string RBInterpreter::visitUnaryExpr(Unary* expr)
{
	std::string right = evaluate(expr->right);

	switch(expr->mOperator->type) {
		case TokenType::T_MINUS:
		{
			return ("-" + right);
		} break;
		case TokenType::T_BANG:
		{
			return( boolToStr(isTrue(expr->right)) );
		} break;
		default:
			return "";
	}

	return "";
}

bool RBInterpreter::isTrue(Expr* expr)
{
	if (expr == nullptr)
	{
		return false;
	}
	else
	{
		Literal* exprLiteral = dynamic_cast <Literal*>(expr);
		if (exprLiteral != nullptr)
		{
			if ((exprLiteral->value == "false") || (exprLiteral->value == ""))
				{return false;}

		}
	}
	return true;
}