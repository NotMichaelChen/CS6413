//Michael Chen - mzc223 - CS6413 - Spring 2018 - Boris Aronov

//Contains main function that passes the given filename to the parser

#include <iostream>
#include <fstream>

#include "parser/parser.hpp"

int main(int argc, char** argv)
{
    if(argc < 2)
    {
        std::cerr << "Usage:\t" << argv[0] << " filename redirect_to_file\n";
        std::cerr << "Pass in anything for the third argument to redirect output to files" << std::endl;
        exit(1);
    }
    if(argc == 3)
    {
        //Redirect output to files
        std::string filename(argv[1]);

        std::ofstream outstream(filename + "-out.txt");
        std::streambuf* coutbuf = std::cout.rdbuf();
        std::cout.rdbuf(outstream.rdbuf());

        std::ofstream errstream(filename + "-err.txt");
        std::streambuf* cerrbuf = std::cerr.rdbuf();
        std::cerr.rdbuf(errstream.rdbuf());

        parse(argv[1]);

        std::cerr.rdbuf(cerrbuf);
        std::cout.rdbuf(coutbuf);
    }
    else
    {
        parse(argv[1]);
    }
}