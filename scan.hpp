#ifndef SCAN_HPP
#define SCAN_HPP

#include <string>
#include <stack>


namespace Scanner
{

struct Token {
    int code;
    int line_number;
    union
    {
        int value;
        float valuef;
        char *ptr;
    };
};

//Call only once
void loadFile(std::string filename);

void nextToken();
//Call only after nextToken
//returning a zero means end of file
Token getToken();
std::string getTokenStr();
std::string decode(int code);

void putToken(Token token);
void rewind(std::stack<Token>& rewind_by);

/* Tokens */
const int KW_INT = 1;
const int KW_FLOAT = 2;
const int KW_IF = 3;
const int KW_ELSE = 4;
const int KW_WHILE = 5;
const int KW_RETURN = 6;
const int KW_READ = 7;
const int KW_WRITE = 8;
const int ID = 10;
const int OP_PLUS = 11;
const int OP_MINUS = 12;
const int OP_MULT = 14;
const int OP_DIV = 15;
const int OP_ASSIGN = 16;
const int OP_EQ = 30;
const int OP_LT = 31;
const int OP_LE = 32;
const int OP_GT = 33;
const int OP_GE = 34;
const int LPAR = 17;
const int RPAR = 18;
const int LBRACE = 19;
const int RBRACE = 20;
const int SEMICOLON = 21;
const int COMMA = 22;
const int INT_LIT = 23;
const int FLOAT_LIT = 24;
const int STRING_LIT = 25;

/* not really tokens */
const int NL_TOKEN = 100;
const int WS_TOKEN = 99;

}

#endif