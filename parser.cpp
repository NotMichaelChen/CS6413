#include "parser.hpp"

#include <iostream>
#include <stack>
#include <vector>

#include "scan.hpp"
#include "symbol.hpp"
#include "first.hpp"

void program();
void decl(bool global);
bool kind();
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
    std::cerr << "Error: unexpected token " << Scanner::getTokenStr() << " on line " << curtoken.line_number << std::endl;
    exit(1);
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
        {
            std::cerr << "Error: syntax error in 'program', with token " << Scanner::getTokenStr() << " on line "
                << Scanner::getToken().line_number << std::endl;
            exit(1);
        }
    }
    //Must be decl
    else
    {
        rewind_stack.pop();
        Scanner::rewind(rewind_stack);
        decl(true);
    }
}

//Skips kind if called from program
void decl(bool global)
{
    std::cout << "In decl" << std::endl;
    std::cin.ignore();

    Scanner::Token type = Scanner::getToken();
    kind();
    varlist(true, global, type.code == Scanner::KW_INT);

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
        std::cerr << "Error: syntax error in 'kind', with token " << Scanner::getTokenStr() << " on line "
                << Scanner::getToken().line_number << std::endl;
        exit(1);
        return false;
    }
}

void varlist(bool dec, bool global, bool isint)
{
    std::cout << "In varlist" << std::endl;
    std::cin.ignore();

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

        std::string typestr = isint ? "int" : "float";
        for(size_t i = 0; i < vars.size(); i++)
        {
            if(global)
            {
                table.insertGlobal(vars[i].ptr, false, false, false, isint, vars[i].line_number);
                std::cout << "Global " << typestr << " variable " << vars[i].ptr << " declared in line " << vars[i].line_number << std::endl;
            }
            else
            {
                table.insertLocal(vars[i].ptr, isint, vars[i].line_number);
                std::cout << "Local " << typestr << " variable " << vars[i].ptr << " declared in line " << vars[i].line_number << std::endl;
            }
        }
    }
    else
    {
        expect(Scanner::ID);

        while(accept(Scanner::COMMA))
        {
            expect(Scanner::ID);
        }
    }
}

void functiondecl()
{
    std::cout << "In functiondecl" << std::endl;
    std::cin.ignore();

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
    std::cout << "Function " << Scanner::decode(type.code) << ' ' << name.ptr << '(' << Scanner::decode(arg.code) << ") declared in line " << name.line_number << std::endl;
}

void functiondef()
{
    std::cout << "In functiondef" << std::endl;
    std::cin.ignore();

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
    std::cout << "Function " << Scanner::decode(type.code) << ' ' << name.ptr << '(' << Scanner::decode(argtype.code) << ") defined in line " << name.line_number << std::endl;
    //Clear local table before entering body
    table.clearLocal();
    //Insert local param variable
    table.insertLocal(argname.ptr, (argtype.code == Scanner::KW_INT), argname.line_number);
    std::cout << "Local " << Scanner::decode(argtype.code) << " variable " << argname.ptr << " declared in line " << argname.line_number << std::endl;

    body();
}

//Assume that all variable declarations comes before all statements
void body()
{
    std::cout << "In body" << std::endl;
    std::cin.ignore();
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
    std::cout << "In stmt" << std::endl;
    std::cin.ignore();
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
    {
        std::cerr << "Error: syntax error in 'writeexprlist', with token " << Scanner::getTokenStr() << " on line "
                << Scanner::getToken().line_number << std::endl;
        exit(1);
    }

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
        else
        {
            LocalSymbol localvar = table.getLocal(lookahead.ptr);
            if(localvar.line_number >= 0)
            {
                std::cout << "Local variable " << lookahead.ptr << " declared in line " << localvar.line_number << " used in line " << lookahead.line_number << std::endl;
            }
            else
            {
                GlobalSymbol globalvar = table.getGlobal(lookahead.ptr);

                if(globalvar.line_number >= 0)
                {
                    if(!globalvar.is_function)
                    {
                        std::cout << "Global variable " << lookahead.ptr << " declared in line " << globalvar.line_number << " used in line " << lookahead.line_number << std::endl;
                    }
                    else
                    {
                        std::string isdec = globalvar.is_decl ? "declared " : "defined ";
                        std::cerr << "Error: using function " << lookahead.ptr << " on line " << lookahead.line_number << " as a variable (" << isdec << "in line " << globalvar.line_number << ')' << std::endl;
                        exit(1);
                    }
                }
                else
                {
                    std::cerr << "Error: identifier " << lookahead.ptr << " on line " << lookahead.line_number << " not declared" << std::endl;
                    exit(1);
                }
            }
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

    Scanner::Token name = Scanner::getToken();
    expect(Scanner::ID);
    GlobalSymbol func = table.getGlobal(name.ptr);
    if(func.line_number < 0)
    {
        std::cerr << "Error: calling function " << name.ptr << " on line " << name.line_number << " when it hasn't been declared" << std::endl;
        exit(1);
    }
    else if(!func.is_function)
    {
        std::cerr << "Error: using variable " << name.ptr << " on line " << name.line_number << " as a function (declared in line " << func.line_number << std::endl;
        exit(1);
    }
    std::string isdec = func.is_decl ? " declared " : " defined ";
    std::cout << "Function " << name.ptr << isdec << "in line " << func.line_number << " used in line " << name.line_number << std::endl;

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
        std::cerr << "Error: syntax error in 'mulop', with token " << Scanner::getTokenStr() << " on line "
                << Scanner::getToken().line_number << std::endl;
        exit(1);
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
        std::cerr << "Error: syntax error in 'addop', with token " << Scanner::getTokenStr() << " on line "
                << Scanner::getToken().line_number << std::endl;
        exit(1);
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
        std::cerr << "Error: syntax error in 'boolop', with token " << Scanner::getTokenStr() << " on line "
                << Scanner::getToken().line_number << std::endl;
        exit(1);
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
            LocalSymbol localvar = table.getLocal(lookahead.ptr);
            if(localvar.line_number >= 0)
            {
                std::cout << "Local variable " << lookahead.ptr << " declared in line " << localvar.line_number << " used in line " << lookahead.line_number << std::endl;
            }
            else
            {
                GlobalSymbol globalvar = table.getGlobal(lookahead.ptr);

                if(globalvar.line_number >= 0)
                {
                    if(!globalvar.is_function)
                    {
                        std::cout << "Global variable " << lookahead.ptr << " declared in line " << globalvar.line_number << " used in line " << lookahead.line_number << std::endl;
                    }
                    else
                    {
                        std::string isdec = globalvar.is_decl ? "declared " : "defined ";
                        std::cerr << "Error: using function " << lookahead.ptr << " on line " << lookahead.line_number << " as a variable (" << isdec << "in line " << globalvar.line_number << ')' << std::endl;
                        exit(1);
                    }
                }
                else
                {
                    std::cerr << "Error: identifier " << lookahead.ptr << " on line " << lookahead.line_number << " not declared" << std::endl;
                    exit(1);
                }
            }
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