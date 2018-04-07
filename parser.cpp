#include "parser.hpp"

#include <iostream>
#include <stack>

#include "scan.hpp"
#include "symbol.hpp"
#include "first.hpp"

void program();
void decl();
bool kind();
void varlist();
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
void mulop();
void expr1();
void addop();
void boolop();
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

//Checks if the current scanner token matches the given scanner token. If it does, advance to the next token and return true.
//Otherwise return false
bool accept(int token)
{
    if(Scanner::getToken().code == token)
    {
        Scanner::nextToken();
        return true;
    }
    return false;
}

//Stronger form of accept, will print an error if no match
bool expect(int token)
{
    if(accept(token))
    {
        return true;
    }
    Scanner::Token curtoken = Scanner::getToken();
    std::cerr << "Error: unexpected token " << curtoken.code << " on line " << curtoken.line_number << std::endl;
    return false;
}

//To disambiguate function-def, decl, function-decl, must look ahead
void program()
{
    std::cout << "In program" << std::endl;
    std::cin.ignore();
    //Create a stack so we can "rewind" read tokens
    std::stack<Scanner::Token> rewind_stack;
    //First disambiguation after reading "kind" and "ID"

    rewind_stack.push(Scanner::getToken());
    //Didn't read a 'kind', didn't get new token, don't save this token
    if(!kind())
        rewind_stack.pop();

    rewind_stack.push(Scanner::getToken());
    if(!expect(Scanner::ID))
        rewind_stack.pop();

    rewind_stack.push(Scanner::getToken());
    //Either function-decl or function-def
    if(accept(Scanner::LPAR))
    {
        //Must read another "kind"
        rewind_stack.push(Scanner::getToken());
        if(!kind())
            rewind_stack.pop();

        //Must be function-decl
        rewind_stack.push(Scanner::getToken());
        if(accept(Scanner::RPAR))
        {
            Scanner::rewind(rewind_stack);
            functiondecl();
        }
        //Must be function-def
        else if(accept(Scanner::ID))
        {
            Scanner::rewind(rewind_stack);
            functiondef();
        }
        else
            std::cerr << "Error: syntax error in 'program'" << std::endl;
    }
    //Must be decl
    else
    {
        rewind_stack.pop();
        Scanner::rewind(rewind_stack);
        decl();
    }
}

//Skips kind if called from program
void decl()
{
    std::cout << "In decl" << std::endl;
    std::cin.ignore();

    kind();
    varlist();

    expect(Scanner::SEMICOLON);
}

bool kind()
{
    std::cout << "In kind" << std::endl;
    std::cin.ignore();
    if(accept(Scanner::KW_INT))
        return true;
    else if(accept(Scanner::KW_FLOAT))
        return true;
    else
    {
        //std::cerr << "Error: syntax error in 'kind'" << std::endl;
        return false;
    }
}

void varlist()
{
    std::cout << "In varlist" << std::endl;
    std::cin.ignore();

    expect(Scanner::ID);

    while(accept(Scanner::COMMA))
    {
        expect(Scanner::ID);
    }
}

void functiondecl()
{
    std::cout << "In functiondecl" << std::endl;
    std::cin.ignore();

    kind();
    expect(Scanner::ID);
    expect(Scanner::LPAR);
    kind();
    expect(Scanner::RPAR);

    expect(Scanner::SEMICOLON);
}

void functiondef()
{
    std::cout << "In functiondef" << std::endl;
    std::cin.ignore();

    kind();
    expect(Scanner::ID);
    expect(Scanner::LPAR);
    kind();
    expect(Scanner::ID);
    expect(Scanner::RPAR);
    body();

}

//Assume that all variable declarations comes before all statements
void body()
{
    std::cout << "In body" << std::endl;
    std::cin.ignore();
    expect(Scanner::LBRACE);

    while(kind())
    {
        varlist();
        expect(Scanner::SEMICOLON);
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
    std::cout << "In stmt" << std::endl;
    std::cin.ignore();
    if(accept(Scanner::KW_IF))
    {
        expect(Scanner::LPAR);
        expr();
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
        varlist();
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
    std::cout << "In writeexprlist" << std::endl;
    std::cin.ignore();
    if(accept(Scanner::STRING_LIT))
        ;
    else if(first_expr(Scanner::getToken().code))
        expr();
    else
        std::cerr << "Error: syntax error in 'writeexprlist'" << std::endl;

    while(accept(Scanner::COMMA))
        expr();
}

void factor()
{
    std::cout << "In factor" << std::endl;
    std::cin.ignore();
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
    }
    else
        std::cerr << "Error: syntax error in 'factor'" << std::endl;
}

void boolexpr()
{
    std::cout << "In boolexpr" << std::endl;
    std::cin.ignore();
    expr();
    boolop();
    expr();
}

void functioncall()
{
    std::cout << "In functioncall" << std::endl;
    std::cin.ignore();

    expect(Scanner::ID);
    expect(Scanner::LPAR);
    expr();
    expect(Scanner::RPAR);
}

void term()
{
    std::cout << "In term" << std::endl;
    std::cin.ignore();

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

void mulop()
{
    std::cout << "In mulop" << std::endl;
    std::cin.ignore();
    if(accept(Scanner::OP_MULT))
        ;
    else if(accept(Scanner::OP_DIV))
        ;
    else
    {
        std::cerr << "Error: syntax error in 'mulop'" << std::endl;
    }
}

void expr1()
{
    std::cout << "In expr1" << std::endl;
    std::cin.ignore();

    term();
    while(first_addop(Scanner::getToken().code))
    {
        addop();
        term();
    }
}

void addop()
{
    std::cout << "In addop" << std::endl;
    std::cin.ignore();
    if(accept(Scanner::OP_PLUS))
        ;
    else if(accept(Scanner::OP_MINUS))
        ;
    else
    {
        std::cerr << "Error: syntax error in 'addop'" << std::endl;
    }
}

void boolop()
{
    std::cout << "In boolop" << std::endl;
    std::cin.ignore();
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
        std::cerr << "Error: syntax error in 'boolop'" << std::endl;
    }
}

void expr()
{
    std::cout << "In expr" << std::endl;
    std::cin.ignore();
    Scanner::Token lookahead = Scanner::getToken();
    //Could still be either case
    if(accept(Scanner::ID))
    {
        //Definitely first case
        if(accept(Scanner::OP_ASSIGN))
        {
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
        std::cerr << "Error: syntax error in 'expr'" << std::endl;
}