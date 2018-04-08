//Michael Chen - mzc223 - CS6413 - Spring 2018 - Boris Aronov

//Contains main function that passes the given filename to the parser

#include <iostream>

#include "parser.hpp"

int main(int argc, char** argv)
{
    if(argc != 2)
    {
        std::cerr << "Usage:\t" << argv[0] << " filename\n";
        exit(1);
    }

    parse(argv[1]);
}