#include "scan.hpp"

#include <cstdio>
#include <iostream>

#include "common.h"

YYSTYPE yylval;
int next_token;
std::stack<Scanner::Token> token_stack;

extern FILE* yyin;

namespace Scanner
{

void loadFile(std::string filename)
{
    if (!(yyin=fopen(filename.c_str(),"r")))
    {
        std::cerr << "Cannot open '" << filename << "'.\n";
        exit(1);
    }
}

void nextToken()
{
    if(!token_stack.empty())
        token_stack.pop();
    
    if(token_stack.empty())
    {
        Token t;
        t.code = yylex();
        t.line_number = line_no;
        //Only need to set one member of the union
        t.value = yylval.value;

        token_stack.push(t);
    }
}

Token getToken()
{
    if(token_stack.empty())
        std::cerr << "Error: attempt to access token without calling nextToken" << std::endl;
    return token_stack.top();
}

void putToken(Token token)
{
    token_stack.push(token);
}

void rewind(std::stack<Token>& rewind_by)
{
    while(!rewind_by.empty())
    {
        token_stack.push(rewind_by.top());
        rewind_by.pop();
    }
}

}