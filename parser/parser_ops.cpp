#include "parser_ops.hpp"

#include <iostream>

#include "parser_internal.hpp"
#include "scan.hpp"

void mulop()
{
    if(accept(Scanner::OP_MULT))
        ;
    else if(accept(Scanner::OP_DIV))
        ;
    else
    {
        std::cerr << "Error: syntax error in 'mulop', with token " << Scanner::getTokenStr() << " on line "
                << Scanner::getToken().line_number << std::endl;
        exit(1);
    }
}

void addop()
{
    if(accept(Scanner::OP_PLUS))
        ;
    else if(accept(Scanner::OP_MINUS))
        ;
    else
    {
        std::cerr << "Error: syntax error in 'addop', with token " << Scanner::getTokenStr() << " on line "
                << Scanner::getToken().line_number << std::endl;
        exit(1);
    }
}


void boolop()
{
    if(accept(Scanner::OP_LT))
        ;
    else if(accept(Scanner::OP_GT))
        ;
    else if(accept(Scanner::OP_EQ))
        ;
    else if(accept(Scanner::OP_GE))
        ;
    else if(accept(Scanner::OP_LE))
        ;
    else
    {
        std::cerr << "Error: syntax error in 'boolop', with token " << Scanner::getTokenStr() << " on line "
                << Scanner::getToken().line_number << std::endl;
        exit(1);
    }
}