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

	functions.insert_or_assign("clock", new ClockFunction);
	environment->define("clock", "clock");
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

std::string RBInterpreter::visitCallExpr(Call* expr)
{
	std::string callee = evaluate(expr->callee); // function name
	std::vector<std::string> arguments;

	for (Expr* argument : expr->arguments) {
		arguments.push_back(evaluate(argument));
	}

	auto function = functions.find(callee);

	if (function == functions.end())
	{
		reportError(expr->paren->line, "Can only call functions and classes.");
		return "";
	}
	if (arguments.size() != function->second->arity())
	{
		reportError(expr->paren->line, "Expected " + std::to_string(function->second->arity()) + " arguments but got " + std::to_string(arguments.size()) + ".");
	}

	return function->second->call(this, arguments);

}

std::string RBInterpreter::visitGroupingExpr(Grouping* expr)
{
	return evaluate(expr->expression);
}

std::string RBInterpreter::visitLogicalExpr(Logical* expr)
{
	std::string left = evaluate(expr->left);
	if (expr->mOperator->type == TokenType::T_OR)
	{
		if(isTrue(expr->left))
			{return left;}
	}
	else
	{
		if(!isTrue(expr->left))
			{return left;}
	}

	return evaluate(expr->right);
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
		std::string value = evaluate(expr);
		//Literal* exprLiteral = dynamic_cast <Literal*>(expr);
		//if (value != "")//(exprLiteral != nullptr)
		//{
			//if ((exprLiteral->value == "false") || (exprLiteral->value == ""))
		if ((value == "false") || (value == ""))
			{return false;}

		//}
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

std::string RBInterpreter::visitFunction(Function* stmt)
{
	std::vector<std::string> argumentNames;
	for (Token* current : stmt->params)
	{
		argumentNames.push_back(current->lexeme);
	}

	UserFunction* function = new UserFunction(stmt->body, argumentNames, stmt->name, environment);
	
	functions.insert_or_assign(stmt->name->lexeme, function);
	globals->define(stmt->name->lexeme, stmt->name->lexeme);

	return "";
}

std::string RBInterpreter::visitIf(IfStmt* stmt)
{
	if(isTrue(stmt->expression)) {
		stmt->thenBranch->accept(this);
	}
	else if (stmt->elseBranch != nullptr)
	{
		stmt->elseBranch->accept(this);
	}

	return "ifStmt";
}


std::string RBInterpreter::visitPrint(Stmt* stmt)
{
	std::string output = evaluate(stmt->expression);
	UtilityFunctions::print(output.c_str());
	return output;
}

std::string RBInterpreter::visitReturn(Return* stmt)
{
	std::string value = "";
	if (stmt->expression != nullptr)
	{
		value = evaluate(stmt->expression);
	}

	throw value;
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

std::string RBInterpreter::visitWhile(While* stmt)
{
	while (isTrue(stmt->expression))
	{
		stmt->body->accept(this);
	}
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

void Environment::define(std::string name, std::string value)
{
	if(checkPresent(name))
	{
		std::string newText = ("ERROR: Cannot redeclare variable '" + name + "'.");
		UtilityFunctions::print(newText.c_str());
	}
	else
	{
		values.insert_or_assign(name, value);
	}
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

UserFunction::UserFunction(std::vector<Stmt*> iBody, std::vector<std::string> iArgumentNames, Token* iName, Environment* iClosure)
{
	body = iBody;
	argumentNames = iArgumentNames;
	arityNumber = argumentNames.size();
	name = iName->lexeme;
	clusure = iClosure;
}