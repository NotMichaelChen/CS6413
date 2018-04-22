//Michael Chen - mzc223 - CS6413 - Spring 2018 - Boris Aronov

#ifndef VARPRINT_HPP
#define VARPRINT_HPP

#include "symbol.hpp"
#include "scan.hpp"

#include <vector>

/**
 * Contains functions that print out a formatted message when a variable or function is declared/defined and called/used
 */

void printVarUse(Scanner::Token tok, SymbolTable& table);
void printVarDeclare(std::vector<Scanner::Token>& toks, SymbolTable& table, bool isint, bool isglobal);
void printFuncUse(Scanner::Token tok, SymbolTable& table);

#endif