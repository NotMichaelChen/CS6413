#include <iostream>

#include "parser.hpp"

int main(int argc, char** argv)
{
    if(argc != 2)
    {
        std::cerr << "\n\t\tUsage:\t" << argv[0] << " filename\n";
        exit(1);
    }

    parse(argv[1]);
}