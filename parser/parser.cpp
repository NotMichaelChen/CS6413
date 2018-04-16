//Michael Chen - mzc223 - CS6413 - Spring 2018 - Boris Aronov

#include "parser.hpp"

#include <iostream>
#include <stack>
#include <vector>

#include "scan.hpp"
#include "symbol.hpp"
#include "first.hpp"
#include "varprint.hpp"

#include "parser_internal.hpp"
#include "parser_ops.hpp"

//One function for each non-terminal
void program();
void decl(bool global);
void kind();
void varlist(bool dec, bool global, bool isint);
void functiondecl();
void functiondef();
void body();
void stmt();
//expr-list is unused
void writeexprlist();
void factor();
void boolexpr();
void functioncall();
void term();
void expr1();
void expr();

SymbolTable table;

void parse(std::string filename)
{
    Scanner::loadFile(filename);
    std::cout << "File loaded, time to scan" << std::endl;

    //Implicitly does the "zero or more of" required of program
    Scanner::nextToken();
    while(Scanner::getToken().code)
        program();
}

// All functions assume the "token pointer" is pointed at the first token they're supposed to read, so don't advance

//To disambiguate function-def, decl, function-decl, must look ahead
void program()
{
    Scanner::enableBuffering();

    //First disambiguation after reading "kind" and "ID"
    kind();
    expect(Scanner::ID);
    //Either function-decl or function-def
    if(accept(Scanner::LPAR))
    {
        kind();

        //Must be function-decl
        if(accept(Scanner::RPAR))
        {
            Scanner::rewind();
            functiondecl();
        }
        //Must be function-def
        else if(accept(Scanner::ID))
        {
            Scanner::rewind();
            functiondef();
        }
        else
        {
            std::cerr << "Error: syntax error in 'program', with token " << Scanner::getTokenStr() << " on line "
                << Scanner::getToken().line_number << std::endl;
            exit(1);
        }
    }
    //Must be decl
    else
    {
        Scanner::rewind();
        decl(true);
    }
}

void decl(bool global)
{
    Scanner::Token type = Scanner::getToken();
    kind();
    varlist(true, global, type.code == Scanner::KW_INT);

    expect(Scanner::SEMICOLON);
}

void kind()
{
    if(accept(Scanner::KW_INT))
        ;
    else if(accept(Scanner::KW_FLOAT))
        ;
    else
    {
        std::cerr << "Error: syntax error in 'kind', with token " << Scanner::getTokenStr() << " on line "
                << Scanner::getToken().line_number << std::endl;
        exit(1);
    }
}

void varlist(bool dec, bool global, bool isint)
{
    if(dec)
    {
        std::vector<Scanner::Token> vars;

        vars.push_back(Scanner::getToken());
        expect(Scanner::ID);

        while(accept(Scanner::COMMA))
        {
            vars.push_back(Scanner::getToken());
            expect(Scanner::ID);
        }

        printVarDeclare(vars, table, isint, global);
    }
    else
    {
        Scanner::Token idtok = Scanner::getToken();
        expect(Scanner::ID);
        printVarUse(idtok, table);
        
        while(accept(Scanner::COMMA))
        {
            idtok = Scanner::getToken();
            expect(Scanner::ID);
            printVarUse(idtok, table);
        }
    }
}

void functiondecl()
{
    Scanner::Token type = Scanner::getToken();
    kind();
    Scanner::Token name = Scanner::getToken();
    expect(Scanner::ID);
    expect(Scanner::LPAR);
    Scanner::Token arg = Scanner::getToken();
    kind();
    expect(Scanner::RPAR);

    expect(Scanner::SEMICOLON);

    //Functions are always global
    if(!table.insertGlobal(name.ptr, true, true, (arg.code == Scanner::KW_INT), (type.code == Scanner::KW_INT),
        name.line_number))
    {
        exit(1);
    }
    std::cout << "Function " << Scanner::decode(type.code) << ' ' << name.ptr << '(' << Scanner::decode(arg.code)
        << ") declared in line " << name.line_number << std::endl;
}

void functiondef()
{
    Scanner::Token type = Scanner::getToken();
    kind();
    Scanner::Token name = Scanner::getToken();
    expect(Scanner::ID);
    expect(Scanner::LPAR);
    Scanner::Token argtype = Scanner::getToken();
    kind();
    Scanner::Token argname = Scanner::getToken();
    expect(Scanner::ID);
    expect(Scanner::RPAR);

    //Insert function def
    if(!table.insertGlobal(name.ptr, true, false, (argtype.code == Scanner::KW_INT), (type.code == Scanner::KW_INT),
        name.line_number))
    {
        exit(1);
    }
    std::cout << "Function " << Scanner::decode(type.code) << ' ' << name.ptr << '(' << Scanner::decode(argtype.code)
        << ") defined in line " << name.line_number << std::endl;
    //Clear local table before entering body
    table.clearLocal();
    //Insert local param variable
    table.insertLocal(argname.ptr, (argtype.code == Scanner::KW_INT), argname.line_number);
    std::cout << "Local " << Scanner::decode(argtype.code) << " variable " << argname.ptr << " declared in line "
        << argname.line_number << std::endl;

    body();
}

//Assume that all variable declarations comes before all statements
void body()
{
    expect(Scanner::LBRACE);

    while(Scanner::getToken().code == Scanner::KW_INT || Scanner::getToken().code == Scanner::KW_FLOAT)
    {
        decl(false);
    }

    int token = Scanner::getToken().code;
    while(first_stmt(token))
    {
        stmt();
        token = Scanner::getToken().code;
    }

    expect(Scanner::RBRACE);
}

//Assume that control statements can only execute one other statement
void stmt()
{
    if(accept(Scanner::KW_IF))
    {
        expect(Scanner::LPAR);
        boolexpr();
        expect(Scanner::RPAR);
        stmt();
        if(accept(Scanner::KW_ELSE))
        {
            stmt();
        }
    }
    else if(accept(Scanner::KW_WHILE))
    {
        expect(Scanner::LPAR);
        boolexpr();
        expect(Scanner::RPAR);
        stmt();
    }
    else if(accept(Scanner::KW_READ))
    {
        varlist(false, false, false);
        expect(Scanner::SEMICOLON);
    }
    else if(accept(Scanner::KW_WRITE))
    {
        writeexprlist();
        expect(Scanner::SEMICOLON);
    }
    else if(accept(Scanner::KW_RETURN))
    {
        expr();
        expect(Scanner::SEMICOLON);
    }
    else if(accept(Scanner::LBRACE))
    {
        stmt();
        while(!accept(Scanner::RBRACE))
        {
            stmt();
        }
    }
    //Assume it's an expression case
    else
    {
        expr();
        expect(Scanner::SEMICOLON);
    }
}

//Skip expr-list

void writeexprlist()
{
    if(accept(Scanner::STRING_LIT))
        ;
    else if(first_expr(Scanner::getToken().code))
        expr();
    else
    {
        std::cerr << "Error: syntax error in 'writeexprlist', with token " << Scanner::getTokenStr() << " on line "
                << Scanner::getToken().line_number << std::endl;
        exit(1);
    }

    while(accept(Scanner::COMMA))
    {
        if(accept(Scanner::STRING_LIT))
            ;
        else if(first_expr(Scanner::getToken().code))
            expr();
        else
        {
            std::cerr << "Error: syntax error in 'writeexprlist', with token " << Scanner::getTokenStr() << " on line "
                    << Scanner::getToken().line_number << std::endl;
            exit(1);
        }
    }
}

void factor()
{
    Scanner::Token lookahead = Scanner::getToken();
    if(accept(Scanner::INT_LIT))
        ;
    else if(accept(Scanner::FLOAT_LIT))
        ;
    else if(accept(Scanner::LPAR))
        ;
    //Disambiguate between ID and function-call
    else if(accept(Scanner::ID))
    {
        if(Scanner::getToken().code == Scanner::LPAR)
        {
            //Put back ID for function-call
            Scanner::putToken(lookahead);
            functioncall();
        }
        else
        {
            printVarUse(lookahead, table);
        }
    }
    else
    {
        std::cerr << "Error: syntax error in 'factor', with token " << Scanner::getTokenStr() << " on line "
                << Scanner::getToken().line_number << std::endl;
        exit(1);
    }
}

void boolexpr()
{
    expr();
    boolop();
    expr();
}

void functioncall()
{
    Scanner::Token name = Scanner::getToken();
    expect(Scanner::ID);
    printFuncUse(name, table);

    expect(Scanner::LPAR);
    expr();
    expect(Scanner::RPAR);
}

void term()
{
    //Simply attempt to accept a minus, if you can't then just skip
    accept(Scanner::OP_MINUS);
    factor();
    while(first_mulop(Scanner::getToken().code))
    {
        mulop();
        accept(Scanner::OP_MINUS);
        factor();
    }
}

void expr1()
{
    term();
    while(first_addop(Scanner::getToken().code))
    {
        addop();
        term();
    }
}

void expr()
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
            expr();
        }
        //Assume second case
        else
        {
            Scanner::putToken(lookahead);
            expr1();
        }
    }
    //Definitely expr1 case
    else if(first_expr1(Scanner::getToken().code))
    {
        expr1();
    }
    else
    {
        std::cerr << "Error: syntax error in 'expr', with token " << Scanner::getTokenStr() << " on line "
                << Scanner::getToken().line_number << std::endl;
        exit(1);
    }
}