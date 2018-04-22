//Michael Chen - mzc223 - CS6413 - Spring 2018 - Boris Aronov

#ifndef SCAN_HPP
#define SCAN_HPP

#include <string>
#include <stack>

/**
 * Wraps the provided scanner with a nicer interface, and implements token rewinding for look-aheads in the parser
 */

namespace Scanner
{

//Represents a scanned token
//TODO: Free ptr when done?
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

//Loads the given file
//Call only once
void loadFile(std::string filename);

//Advances the scanner by one token
//Unless buffering is enabled, tokens are discarded
void nextToken();

//Gets the current token
//Call only after nextToken
//returning a zero means end of file
Token getToken();

//Puts a token back into the scanner stream
//This is similar to ungetc()
void putToken(Token token);

//Enables buffering of scanner tokens
//nextToken will not discard tokens, and will instead remember old tokens
void enableBuffering();

//Discards the scanner tokens if a rewind is not necessary
void discardBuffer();

//Rewinds the stream by putting back all of the old tokens that were collected while buffering was enables
void rewind();

//Gets the current token as a string
std::string getTokenStr();

//Given a token code, return a string representing it. Useful for printing
std::string decode(int code);

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