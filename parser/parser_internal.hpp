//Michael Chen - mzc223 - CS6413 - Spring 2018 - Boris Aronov

#ifndef PARSER_INTERNAL_HPP
#define PARSER_INTERNAL_HPP

#include <iostream>

#include "scan.hpp"
#include "symbol.hpp"

/**
 * Internal header file for all parser functions that define accept/expect functions and exposes the global symbol table.
 */

extern SymbolTable table;

//Returns whether the current token in the scanner matches the given token. If it does, advance the scanner to the next token
bool accept(int token);
//Stronger form of accept, will print an error if the token doesn't match
bool expect(int token);

#endif