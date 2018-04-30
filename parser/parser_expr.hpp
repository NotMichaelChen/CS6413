//Michael Chen - mzc223 - CS6413 - Spring 2018 - Boris Aronov

#ifndef PARSER_EXPR_HPP
#define PARSER_EXPR_HPP

struct ExprResult
{
    bool isint;
    //Represents which memory location the result of the computation is in
    int resultloc;
};

ExprResult expr();
ExprResult expr1();
ExprResult term();
ExprResult factor();
ExprResult functioncall();

#endif