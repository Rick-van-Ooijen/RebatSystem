#include "printer.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cctype>

using namespace godot;

// for the printer
std::string Binary::accept(AstPrinter* printer)
{
	return printer->visitBinaryExpr(this);
}

std::string Call::accept(AstPrinter* printer)
{
	return printer->visitCallExpr(this);
}

std::string Grouping::accept(AstPrinter* printer)
{
	return printer->visitGroupingExpr(this);
}

std::string Logical::accept(AstPrinter* printer)
{
	return printer->visitLogicalExpr(this);
}

std::string Literal::accept(AstPrinter* printer)
{
	return printer->visitLiteralExpr(this);
}

std::string Unary::accept(AstPrinter* printer)
{
	return printer->visitUnaryExpr(this);
}

std::string Variable::accept(AstPrinter* printer)
{
	return printer->visitVariableExpr(this);
}

std::string Assign::accept(AstPrinter* printer)
{
	return printer->visitAssignExpr(this);
}

// for the interpreter
std::string Binary::acceptI(RBInterpreter* interpreter)
{
	return interpreter->visitBinaryExpr(this);
}

std::string Call::acceptI(RBInterpreter* interpreter)
{
	return interpreter->visitCallExpr(this);
}

std::string Grouping::acceptI(RBInterpreter* interpreter)
{
	return interpreter->visitGroupingExpr(this);
}

std::string Logical::acceptI(RBInterpreter* interpreter)
{
	return interpreter->visitLogicalExpr(this);
}

std::string Literal::acceptI(RBInterpreter* interpreter)
{
	return interpreter->visitLiteralExpr(this);
}

std::string Unary::acceptI(RBInterpreter* interpreter)
{
	return interpreter->visitUnaryExpr(this);
}

std::string Variable::acceptI(RBInterpreter* interpreter)
{
	return interpreter->visitVariableExpr(this);
}

std::string Assign::acceptI(RBInterpreter* interpreter)
{
	return interpreter->visitAssignExpr(this);
}