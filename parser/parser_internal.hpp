#ifndef PARSER_INTERNAL_HPP
#define PARSER_INTERNAL_HPP

#include <iostream>

#include "scan.hpp"

bool accept(int token);
bool expect(int token);

#endif