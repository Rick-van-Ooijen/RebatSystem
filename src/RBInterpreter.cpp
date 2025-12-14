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

	//AstPrinter printer;
	//std::string text = printer.print(parser.parse());
	//UtilityFunctions::print(text.c_str());


	std::vector<Stmt*> statements = parser.parse();

	//evaluate(parser.parse());
	std::string output = "statements:" + std::to_string(statements.size());
	
	for (Stmt* statement : statements)
	{
		if (statement != nullptr)
		{
			std::string text = statement->accept(this);
			output = output + "\n" + text;
		}
	}

	UtilityFunctions::print(output.c_str());

}



void RBInterpreter::reportError(int line, std::string message)
{
	std::string newText = ("Line (" + std::to_string(line) + ") ERROR: " + message);
	UtilityFunctions::print(newText.c_str());

}

std::string RBInterpreter::evaluate(Expr* expr)
{
	if (expr != nullptr)
	{
		std::string newString = expr->acceptI(this);
		return newString;
	}
	else
	{
		UtilityFunctions::print("expr == nullptr");
	}
	return "";
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

std::string RBInterpreter::visitAssignExpr(Assign* expr)
{
	std::string value = evaluate(expr->value);

	if(environment->assign(expr->name->literal, value))
	{
		return value;
	}

	reportError(expr->name->line, ("Undefined variable '" + expr->name->lexeme + "'."));
	return "";
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



std::string RBInterpreter::visitVariableExpr(Variable* expr)
{
	return environment->get(expr->name);
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

std::string RBInterpreter::visitBlock(Block* stmt)
{
	return executeBlock(stmt->statements, new Environment(environment));
}

std::string RBInterpreter::visitExpression(Stmt* stmt)
{
	return evaluate(stmt->expression);
}


std::string RBInterpreter::visitPrint(Stmt* stmt)
{
	std::string output = evaluate(stmt->expression);
	UtilityFunctions::print(output.c_str());
	return output;
}

std::string RBInterpreter::visitVar(Var* stmt)
{
	std::string value = "";

	if(stmt->expression != nullptr)
	{
		value = evaluate(stmt->expression);
	}

	environment->define(stmt->name->lexeme, value);

	return "";
}

std::string Environment::get(Token* name)
{
	auto found = values.find(name->literal);

	if (found != values.end())
	{
		return (found->second);
	}

	if (enclosing != nullptr)
	{
		return enclosing->get(name);
	}

	std::string newText = ("Line (" + std::to_string(name->line) + ") ERROR: Undefined variable '" + name->lexeme + "'.");
	UtilityFunctions::print(newText.c_str());
	return "";

}

std::string RBInterpreter::executeBlock(std::vector<Stmt*> statements, Environment* iEnvironment)
{
	Environment* previous = environment;

	try
	{
		environment = iEnvironment;
		for (Stmt* statement : statements)
		{
			statement->accept(this);
		}
	}
	catch (int error) { ; }

	environment = previous;

	return "block";
}