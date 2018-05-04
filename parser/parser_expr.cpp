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

/*****************************
 * 
 * Formatting/Helper Functions
 * 
 ****************************/

//Formats an expression result onto a command
void formatExpr(std::string& command, ExprResult& arg)
{
    if(arg.resultloc < 0)
    {
        if(arg.isint)
            command += "#" + std::to_string(arg.intliteral);
        else
            command += "#" + std::to_string(arg.floatliteral);
    }
    else
        command += std::to_string(arg.resultloc);
}

//Puts the correct numbers into the command string
void formatExprArgs(std::string& command, ExprResult& lhs, ExprResult& rhs)
{
    //Place first two args
    formatExpr(command, lhs);
    command += ",";
    formatExpr(command, rhs);
}

//Decodes a scanner code into the correct operation
std::string decodeOpcode(int code)
{
    switch(code)
    {
        case Scanner::OP_PLUS:
            return "ADD";
        case Scanner::OP_MINUS:
            return "SUB";
        case Scanner::OP_MULT:
            return "MUL";
        case Scanner::OP_DIV:
            return "DIV";
        default:
            return "ERROR";
    }
}

/************************
 * 
 * Non-terminal Functions
 * 
 ***********************/

ExprResult expr()
{
    Scanner::Token lookahead = Scanner::getToken();

    //Could still be either case
    if(accept(Scanner::ID))
    {
        //Definitely first case
        if(accept(Scanner::OP_ASSIGN))
        {
            LocalSymbol assignedvar = printVarUse(lookahead, table);
            //Don't put back the lookahead tokens, otherwise infinite recursion
            ExprResult result = expr();

            if(table.isVarInt(lookahead.ptr) != result.isint)
            {
                std::cerr << "Error: type mismatch on line " << lookahead.line_number << std::endl;
                type_error = true;
            }

            //Copy the result temporary into the variable
            std::string command = "COPY";
            command += result.isint ? " " : "F ";
            formatExpr(command, result);

            output.push_back(command + ", " + std::to_string(assignedvar.memloc));

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
            type_error = true;
        }

        //format add/sub command
        std::string command = decodeOpcode(op);
        command += first_term.isint ? " " : "F ";

        //Place first two args
        formatExprArgs(command, first_term, compare_term);
        
        //get result memory loc and add it to third arg of add/sub
        int resultloc = table.getLocalCounter();
        table.decrementLocalCounter();
        command += ",";
        command += std::to_string(resultloc);

        output.push_back(command);

        //Set first_term to result of this calculation to prepare to add to next term
        first_term = {first_term.isint, resultloc};
    }

    return first_term;
}

ExprResult term()
{
    //See if a negate op is required
    bool isnegate = accept(Scanner::OP_MINUS);
    ExprResult first_term = factor();

    //Do negate if necessary
    if(isnegate)
    {
        //Format neg operator
        std::string command = "NEG";
        command += first_term.isint ? " " : "F ";
        
        //Create temporary to put neg result in
        int memloc = table.getLocalCounter();
        table.decrementLocalCounter();
        command += std::to_string(first_term.resultloc) + "," + std::to_string(memloc);
        output.push_back(command);

        //Set first_term memloc to result
        first_term.resultloc = memloc;
    }

    while(first_mulop(Scanner::getToken().code))
    {
        int op = mulop();
        isnegate = accept(Scanner::OP_MINUS);
        Scanner::Token prev = Scanner::getToken();
        ExprResult compare_term = factor();

        if(first_term.isint != compare_term.isint)
        {
            std::cerr << "Error: type mismatch on line " << prev.line_number << std::endl;
            type_error = true;
        }

        //Do negate if necessary
        if(isnegate)
        {
            //Format neg operator
            std::string command = "NEG";
            command += compare_term.isint ? " " : "F ";
            
            //Create temporary to put neg result in
            int memloc = table.getLocalCounter();
            table.decrementLocalCounter();
            command += std::to_string(compare_term.resultloc) + "," + std::to_string(memloc);
            output.push_back(command);

            //Set compare_term memloc to result
            compare_term.resultloc = memloc;
        }

        //format mul/div command
        std::string command = decodeOpcode(op);
        command += first_term.isint ? " " : "F ";

        //Place first two args
        formatExprArgs(command, first_term, compare_term);
        
        //get result memory loc and add it to third arg of add/sub
        int resultloc = table.getLocalCounter();
        table.decrementLocalCounter();
        command += ",";
        command += std::to_string(resultloc);

        output.push_back(command);

        //Set first_term to result of this calculation to prepare to add to next term
        first_term = {first_term.isint, resultloc};
    }

    return first_term;
}

ExprResult factor()
{
    Scanner::Token lookahead = Scanner::getToken();
    if(accept(Scanner::INT_LIT))
        return {true, -1, lookahead.value, 0};
    else if(accept(Scanner::FLOAT_LIT))
        return {false, -1, 0, lookahead.valuef};
    else if(accept(Scanner::LPAR))
    {
        ExprResult result = expr();
        expect(Scanner::RPAR);
        return result;
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
            LocalSymbol var = printVarUse(lookahead, table);
            return {table.isVarInt(lookahead.ptr), var.memloc};
        }
    }
    else
    {
        std::cerr << "Error: syntax error in 'factor', with token " << Scanner::getTokenStr() << " on line "
                << Scanner::getToken().line_number << std::endl;
        exit(1);
    }
}

ExprResult functioncall()
{
    //Get the name of the function and print its use
    Scanner::Token name = Scanner::getToken();
    expect(Scanner::ID);
    printFuncUse(name, table);

    //Store the function to know what label to jump to
    GlobalSymbol function = table.getFunction(name.ptr);

    expect(Scanner::LPAR);

    ExprResult param_result = expr();
    //Check param type
    //At this point, assume function is valid (since it passed through printFuncUse just fine)
    if(param_result.isint != function.param_is_int)
    {
        std::cerr << "Error: type mismatch on line " << name.line_number << std::endl;
        type_error = true;
    }

    expect(Scanner::RPAR);

    //Generate "push, call"
    std::string command = "PUSH";
    command += param_result.isint ? " " : "F ";
    formatExpr(command, param_result);
    output.push_back(command);
    output.push_back("CALL " + std::to_string(function.memloc));

    //Generate "pop" to get result
    int resultloc = table.getLocalCounter();
    table.decrementLocalCounter();
    output.push_back("POP " + std::to_string(resultloc));

    return {function.is_int, resultloc};
}

//Takes label to either the else block, or the end of the if/while statement
void boolexpr(int endlabel)
{
    //Used only for line number
    Scanner::Token prev = Scanner::getToken();

    ExprResult lhs = expr();
    int code = boolop();
    ExprResult rhs = expr();

    if(lhs.isint != rhs.isint)
    {
        std::cerr << "Error: type mismatch on line " << prev.line_number << std::endl;
        type_error = true;
    }

    std::string command;

    //In both if/while cases, we only want to jump when the codition is false, so invert command
    switch(code)
    {
        case Scanner::OP_LT:
            command = "JGE";
            break;
        case Scanner::OP_GT:
            command = "JLE";
            break;
        case Scanner::OP_EQ:
            command = "JNE";
            break;
        case Scanner::OP_GE:
            command = "JLT";
            break;
        case Scanner::OP_LE:
            command = "JGT";
            break;
        default:
            command = "";
            break;
    }

    command += lhs.isint ? " " : "F ";
    formatExprArgs(command, lhs, rhs);

    //Add label to jump to
    command += "," + std::to_string(endlabel);

    output.push_back(command);
}