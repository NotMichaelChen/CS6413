//Michael Chen - mzc223 - CS6413 - Spring 2018 - Boris Aronov

#include "parser_ops.hpp"

#include <iostream>

#include "parser_internal.hpp"
#include "scan.hpp"

/*
 * Contains non-terminals that match operations
 */

int mulop()
{
    if(accept(Scanner::OP_MULT))
        return Scanner::OP_MULT;
    else if(accept(Scanner::OP_DIV))
        return Scanner::OP_DIV;
    else
    {
        std::cerr << "Error: syntax error in 'mulop', with token " << Scanner::getTokenStr() << " on line "
                << Scanner::getToken().line_number << std::endl;
        exit(1);
    }
}

int addop()
{
    if(accept(Scanner::OP_PLUS))
        return Scanner::OP_PLUS;
    else if(accept(Scanner::OP_MINUS))
        return Scanner::OP_MINUS;
    else
    {
        std::cerr << "Error: syntax error in 'addop', with token " << Scanner::getTokenStr() << " on line "
                << Scanner::getToken().line_number << std::endl;
        exit(1);
    }
}


int boolop()
{
    if(accept(Scanner::OP_LT))
        return Scanner::OP_LT;
    else if(accept(Scanner::OP_GT))
        return Scanner::OP_GT;
    else if(accept(Scanner::OP_EQ))
        return Scanner::OP_EQ;
    else if(accept(Scanner::OP_GE))
        return Scanner::OP_GE;
    else if(accept(Scanner::OP_LE))
        return Scanner::OP_LE;
    else
    {
        std::cerr << "Error: syntax error in 'boolop', with token " << Scanner::getTokenStr() << " on line "
                << Scanner::getToken().line_number << std::endl;
        exit(1);
    }
}