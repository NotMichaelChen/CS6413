#include "parser_expr.hpp"

#include <iostream>

#include "scan.hpp"
#include "varprint.hpp"
#include "first.hpp"

#include "parser_internal.hpp"
#include "parser_ops.hpp"

//All return types assume int=true, float=false

bool expr()
{
    Scanner::Token lookahead = Scanner::getToken();
    //Could still be either case
    if(accept(Scanner::ID))
    {
        //Definitely first case
        if(accept(Scanner::OP_ASSIGN))
        {
            printVarUse(lookahead, table);
            //Don't put back the lookahead tokens, otherwise infinite recursion
            bool result = expr();

            if(table.isVarInt(lookahead.ptr) != result)
            {
                std::cerr << "Error: type mismatch on line " << lookahead.line_number << std::endl;
                exit(1);
            }

            return result;

        }
        //Assume second case
        else
        {
            Scanner::putToken(lookahead);
            return expr1();
        }
    }
    //Definitely expr1 case
    else if(first_expr1(Scanner::getToken().code))
    {
        return expr1();
    }
    else
    {
        std::cerr << "Error: syntax error in 'expr', with token " << Scanner::getTokenStr() << " on line "
                << Scanner::getToken().line_number << std::endl;
        exit(1);
    }
}

bool expr1()
{
    bool first_type = term();
    while(first_addop(Scanner::getToken().code))
    {
        addop();
        Scanner::Token prev = Scanner::getToken();
        bool compare_type = term();

        if(first_type != compare_type)
        {
            std::cerr << "Error: type mismatch on line " << prev.line_number << std::endl;
            exit(1);
        }
    }

    return first_type;
}

bool term()
{
    //Simply attempt to accept a minus, if you can't then just skip
    accept(Scanner::OP_MINUS);
    bool first_type = factor();
    while(first_mulop(Scanner::getToken().code))
    {
        mulop();
        accept(Scanner::OP_MINUS);
        Scanner::Token prev = Scanner::getToken();
        bool compare_type = factor();

        if(first_type != compare_type)
        {
            std::cerr << "Error: type mismatch on line " << prev.line_number << std::endl;
            exit(1);
        }
    }

    return first_type;
}

bool factor()
{
    Scanner::Token lookahead = Scanner::getToken();
    if(accept(Scanner::INT_LIT))
        return true;
    else if(accept(Scanner::FLOAT_LIT))
        return false;
    else if(accept(Scanner::LPAR))
    {
        bool type = expr();
        expect(Scanner::RPAR);
        return type;
    }
    //Disambiguate between ID and function-call
    else if(accept(Scanner::ID))
    {
        if(Scanner::getToken().code == Scanner::LPAR)
        {
            //Put back ID for function-call
            Scanner::putToken(lookahead);
            return functioncall();
        }
        else
        {
            printVarUse(lookahead, table);
            return table.isVarInt(lookahead.ptr);
        }
    }
    else
    {
        std::cerr << "Error: syntax error in 'factor', with token " << Scanner::getTokenStr() << " on line "
                << Scanner::getToken().line_number << std::endl;
        exit(1);
    }
}

bool functioncall()
{
    Scanner::Token name = Scanner::getToken();
    expect(Scanner::ID);
    printFuncUse(name, table);

    expect(Scanner::LPAR);
    bool type = expr();
    expect(Scanner::RPAR);

    return type;
}