//Michael Chen - mzc223 - CS6413 - Spring 2018 - Boris Aronov

#include "parser_internal.hpp"

bool type_error = false;

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