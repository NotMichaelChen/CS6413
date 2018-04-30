//Michael Chen - mzc223 - CS6413 - Spring 2018 - Boris Aronov

#include "parser_expr.hpp"

#include <iostream>

#include "scan.hpp"
#include "varprint.hpp"
#include "first.hpp"

#include "parser_internal.hpp"
#include "parser_ops.hpp"

/*
 * Parses non-terminals related to expressions
 * All functions return whether their parsed expression is an int or a float. This is needed to verify type correctness
 */

//All return types assume int=true, float=false

ExprResult expr()
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
            ExprResult result = expr();

            if(table.isVarInt(lookahead.ptr) != result.isint)
            {
                std::cerr << "Error: type mismatch on line " << lookahead.line_number << std::endl;
            }

            //Copy the result temporary into the variable
            LocalSymbol localvar = table.getLocalVar(lookahead.ptr);
            output.push_back("COPY " + std::to_string(localvar.memloc) + ", " + std::to_string(result.resultloc));

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

ExprResult expr1()
{
    ExprResult first_term = term();
    while(first_addop(Scanner::getToken().code))
    {
        int op = addop();
        Scanner::Token prev = Scanner::getToken();
        ExprResult compare_term = term();

        if(first_term.isint != compare_term.isint)
        {
            std::cerr << "Error: type mismatch on line " << prev.line_number << std::endl;
        }

        //format add/sub command
        std::string command;
        if(op == Scanner::OP_PLUS)
            command = "ADD";
        else if(op == Scanner::OP_MINUS)
            command = "SUB";

        //Place first two args
        command += first_term.isint ? " " : "F ";
        command += first_term.resultloc;
        command += ",";
        command += compare_term.resultloc;
        
        //get result memory loc and add it to third arg of add/sub
        int resultloc = table.getLocalCounter();
        table.decrementLocalCounter();
        command += ",";
        command += resultloc;

        output.push_back(command);

        //Set first_term to result of this calculation to prepare to add to next term
        first_term = {first_term.isint, resultloc};
    }

    return first_term;
}

ExprResult term()
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
        }
    }

    return first_type;
}

ExprResult factor()
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

//TODO: finish writing intermediate code putting the parameter on the stack
ExprResult functioncall()
{
    //Get the name of the function and print its use
    Scanner::Token name = Scanner::getToken();
    expect(Scanner::ID);
    printFuncUse(name, table);

    //Store the function to know what label to jump to
    GlobalSymbol function = table.getFunction(name.ptr);

    expect(Scanner::LPAR);

    bool type = expr();
    //Check param type
    //At this point, assume function is valid (since it passed through printFuncUse just fine)
    if(type != function.param_is_int)
    {
        std::cerr << "Error: type mismatch on line " << name.line_number << std::endl;
    }

    expect(Scanner::RPAR);



    return table.isFuncInt(name.ptr);
}