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

std::string getTokenStr()
{
    return decode(token_stack.top().code);
}

std::string decode(int code)
{
    switch(code)
    {
        case 0:
            return "EOF";
        case KW_INT:
            return "int";
        case KW_FLOAT:
            return "float";
        case KW_IF:
            return "if";
        case KW_ELSE:
            return "else";
        case KW_WHILE:
            return "while";
        case KW_RETURN:
            return "return";
        case KW_READ:
            return "read";
        case KW_WRITE:
            return "write";
        case OP_PLUS:
            return "+";
        case OP_MINUS:
            return "-";
        case OP_MULT:
            return "*";
        case OP_DIV:
            return "/";
        case OP_ASSIGN:
            return "=";
        case OP_EQ:
            return "==";
        case OP_LT:
            return "<";
        case OP_LE:
            return "<=";
        case OP_GT:
            return ">";
        case OP_GE:
            return ">=";
        case LPAR:
            return "(";
        case RPAR:
            return ")";
        case LBRACE:
            return "{";
        case RBRACE:
            return "}";
        case SEMICOLON:
            return ";";
        case COMMA:
            return ",";
        case ID:
            return "identifier";
        case INT_LIT:
            return "integer literal";
        case FLOAT_LIT:
            return "float literal";
        case STRING_LIT:
            return "string literal";
        case WS_TOKEN:
            return "white space";
        case NL_TOKEN:
            return "new line";
        default:
            return "";
    }
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