#ifndef VARPRINT_HPP
#define VARPRINT_HPP

#include "symbol.hpp"
#include "scan.hpp"

#include <vector>

void printVarUse(Scanner::Token tok, SymbolTable& table);
void printVarDeclare(std::vector<Scanner::Token>& toks, SymbolTable& table, bool isint, bool isglobal);
void printFuncUse(Scanner::Token tok, SymbolTable& table);

#endif