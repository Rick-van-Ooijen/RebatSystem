#include "printer.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cctype>

using namespace godot;

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

