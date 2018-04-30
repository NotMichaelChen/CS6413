//Michael Chen - mzc223 - CS6413 - Spring 2018 - Boris Aronov

#ifndef PARSER_EXPR_HPP
#define PARSER_EXPR_HPP

struct ExprResult
{
    bool isint;
    //Represents which memory location the result of the computation is in. Negative if literal
    int resultloc;

    //Only access the one indicated by isint
    int intliteral;
    float floatliteral;
};

ExprResult expr();
ExprResult expr1();
ExprResult term();
ExprResult factor();
ExprResult functioncall();

#endif