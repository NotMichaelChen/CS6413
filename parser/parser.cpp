//Michael Chen - mzc223 - CS6413 - Spring 2018 - Boris Aronov

#include "parser.hpp"

#include <iostream>
#include <stack>
#include <vector>
#include <fstream>
#include <string.h>

#include "scan.hpp"
#include "symbol.hpp"
#include "first.hpp"
#include "varprint.hpp"

#include "parser_internal.hpp"
#include "parser_ops.hpp"
#include "parser_expr.hpp"

/*
 * Recursive descent parser implementation. Each non-terminal is represented by one function
 */

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

SymbolTable table;
std::vector<std::string> output;

int main_label = -1;

//TODO: Do not generate code if there were type errors
void writefile()
{
    std::ofstream ofile("outputcode");
    for(size_t i = 0; i < output.size(); i++)
    {
        ofile << output[i] << '\n';
    }
}

void parse(std::string filename)
{
    Scanner::loadFile(filename);
    std::cout << "File loaded, time to scan" << std::endl;

    //Implicitly does the "zero or more of" required of program
    Scanner::nextToken();
    while(Scanner::getToken().code)
        program();
    
    if(main_label == -1)
        throw std::runtime_error("Error: no main function");
    
    output.push_back("START " + std::to_string(main_label));

    writefile();
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
    //'read' case
    else
    {
        std::string command = "READ";

        //Read the first ID
        Scanner::Token idtok = Scanner::getToken();
        expect(Scanner::ID);
        LocalSymbol var = printVarUse(idtok, table);

        command += var.is_int ? " " : "F ";

        std::string line = command + std::to_string(var.memloc);
        output.push_back(line);
        
        while(accept(Scanner::COMMA))
        {
            line.clear();

            idtok = Scanner::getToken();
            expect(Scanner::ID);
            var = printVarUse(idtok, table);

            line = command + std::to_string(var.memloc);
            output.push_back(line);
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

    //Generate label to indicate function
    GlobalSymbol funcsymbol = table.getFunction(name.ptr);
    output.push_back("LABEL " + std::to_string(funcsymbol.memloc));

    //Insert local param variable
    table.insertLocal(argname.ptr, (argtype.code == Scanner::KW_INT), argname.line_number);
    std::cout << "Local " << Scanner::decode(argtype.code) << " variable " << argname.ptr << " declared in line "
        << argname.line_number << std::endl;
    
    //Generate code to retrieve param variable from stack
    //Do not pop for main function
    //TODO: Figure out how the parameter to main works
    if(strcmp("main", name.ptr) == 0)
    {
        main_label = funcsymbol.memloc;
    }
    else
    {
        LocalSymbol param = table.getLocal(argname.ptr);
        output.push_back("POP " + std::to_string(param.memloc));
    }

    body();

    //main function should have a "stop" at the end
    if(strcmp("main", name.ptr) == 0)
        output.push_back("STOP");
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

void stmt()
{
    if(accept(Scanner::KW_IF))
    {
        //Reserve a label to jump to from first if
        int iflabel = table.getControlCounter();
        table.decrementControlCounter();

        //Generate jump
        expect(Scanner::LPAR);
        boolexpr(iflabel);
        expect(Scanner::RPAR);

        stmt();

        if(accept(Scanner::KW_ELSE))
        {
            //Reserve an ending label
            int endinglabel = table.getControlCounter();
            table.decrementControlCounter();

            //Jump before entering else
            output.push_back("JUMP " + std::to_string(endinglabel));

            //Place if-label
            output.push_back("LABEL " + std::to_string(iflabel));

            stmt();

            //Place ending label
            output.push_back("LABEL " + std::to_string(endinglabel));
        }
        else
        {
            //Just place if-label
            output.push_back("LABEL " + std::to_string(iflabel));
        }
    }
    else if(accept(Scanner::KW_WHILE))
    {
        //Generate start and exit label
        int startlabel = table.getControlCounter();
        table.decrementControlCounter();
        int exitlabel = table.getControlCounter();
        table.decrementControlCounter();

        //Place start label
        output.push_back("LABEL " + std::to_string(startlabel));

        //Generate conditional jump
        expect(Scanner::LPAR);
        boolexpr(exitlabel);
        expect(Scanner::RPAR);

        stmt();

        //Generate loop-back jump
        output.push_back("JUMP " + std::to_string(startlabel));

        //Place exit label
        output.push_back("LABEL " + std::to_string(exitlabel));
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

//TODO: Write expressions too, figure out how to deal with newlines
void writeexprlist()
{
    Scanner::Token lookahead = Scanner::getToken();
    if(accept(Scanner::STRING_LIT))
    {
        std::string command = "WRITES \"";
        command += lookahead.ptr;
        output.push_back(command + "\"");
    }
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
        {
            std::string command = "WRITES \"";
            command += lookahead.ptr;
            output.push_back(command + "\"");
        }
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