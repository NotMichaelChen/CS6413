//Michael Chen - mzc223 - CS6413 - Spring 2018 - Boris Aronov

#include "first.hpp"

#include "scan.hpp"

/*
 * Contains functions that return whether the given symbol is part of the FIRST set of a non-terminal
 */

bool first_stmt(int symbol)
{
    return symbol == Scanner::KW_IF ||
        symbol == Scanner::KW_WHILE ||
        symbol == Scanner::KW_READ ||
        symbol == Scanner::KW_WRITE ||
        symbol == Scanner::KW_RETURN ||
        symbol == Scanner::LBRACE ||
        first_expr(symbol);
}

bool first_expr(int symbol)
{
    return symbol == Scanner::ID ||
        first_expr1(symbol);
}

bool first_mulop(int symbol)
{
    return symbol == Scanner::OP_MULT ||
        symbol == Scanner::OP_DIV;
}

bool first_expr1(int symbol)
{
    return symbol == Scanner::OP_MINUS ||
        symbol == Scanner::ID ||
        symbol == Scanner::INT_LIT ||
        symbol == Scanner::FLOAT_LIT ||
        symbol == Scanner::LPAR;
}

bool first_addop(int symbol)
{
    return symbol == Scanner::OP_PLUS ||
        symbol == Scanner::OP_MINUS;
}

