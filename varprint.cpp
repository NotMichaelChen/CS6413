//Michael Chen - mzc223 - CS6413 - Spring 2018 - Boris Aronov

#include "varprint.hpp"

#include <iostream>

bool printVarUse(Scanner::Token tok, SymbolTable& table)
{
    bool isint;

    LocalSymbol localvar = table.getLocal(tok.ptr);
    if(localvar.line_number >= 0)
    {
        std::cout << "Local variable " << tok.ptr << " declared in line " << localvar.line_number
            << " used in line " << tok.line_number << std::endl;
        isint = localvar.is_int;
    }
    else
    {
        GlobalSymbol globalvar = table.getGlobal(tok.ptr);

        if(globalvar.line_number >= 0)
        {
            if(!globalvar.is_function)
            {
                std::cout << "Global variable " << tok.ptr << " declared in line " << globalvar.line_number
                    << " used in line " << tok.line_number << std::endl;
                isint = globalvar.is_int;
            }
            else
            {
                std::string isdec = globalvar.is_decl ? "declared " : "defined ";
                std::cerr << "Error: using function " << tok.ptr << " on line " << tok.line_number
                    << " as a variable (" << isdec << "in line " << globalvar.line_number << ')' << std::endl;
                exit(1);
            }
        }
        else
        {
            std::cerr << "Error: identifier " << tok.ptr << " on line " << tok.line_number
                << " not declared" << std::endl;
            exit(1);
        }
    }

    return isint;
}

void printVarDeclare(std::vector<Scanner::Token>& toks, SymbolTable& table, bool isint, bool isglobal)
{
    std::string typestr = isint ? "int" : "float";
    for(size_t i = 0; i < toks.size(); i++)
    {
        if(isglobal)
        {
            if(!table.insertGlobal(toks[i].ptr, false, false, false, isint, toks[i].line_number))
                exit(1);
            std::cout << "Global " << typestr << " variable " << toks[i].ptr << " declared in line " << toks[i].line_number
                << std::endl;
        }
        else
        {
            if(!table.insertLocal(toks[i].ptr, isint, toks[i].line_number))
                exit(1);
            std::cout << "Local " << typestr << " variable " << toks[i].ptr << " declared in line " << toks[i].line_number
                << std::endl;
        }
    }
}

void printFuncUse(Scanner::Token tok, SymbolTable& table)
{
    GlobalSymbol func = table.getGlobal(tok.ptr);
    if(func.line_number < 0)
    {
        std::cerr << "Error: calling function " << tok.ptr << " on line " << tok.line_number
            << " when it hasn't been declared" << std::endl;
        exit(1);
    }
    else if(!func.is_function)
    {
        std::cerr << "Error: using variable " << tok.ptr << " on line " << tok.line_number
            << " as a function (declared in line " << func.line_number << std::endl;
        exit(1);
    }
    std::string isdec = func.is_decl ? " declared " : " defined ";
    std::cout << "Function " << tok.ptr << isdec << "in line " << func.line_number << " used in line " << tok.line_number
        << std::endl;
}