Michael Chen (mzc223) - Spring 2018

# CS6413 Project - SiCompiler

## Build Instructions

SiCompiler is built with the provided makefile, and requires `g++` and `flex` to compile the flex file and the c++ source files. To build, simply type `make` in the main directory, which will then compile into a binary called `out` in the `exe` folder.

The binary takes at most 2 arguments: a filename and an optional arbitrary string that indicates whether to redirect output to a file. Leaving out the 2nd argument will direct output to the terminal. Output code is stored in a file called `outputcode`. No code is generated if errors are found.

For reference, development was done with `g++:7.2.0` and `flex:2.6.1` on an Ubuntu 17.10 system.

# Project Components

## Lexical Scanner

The scanner used in this project was provided to us; however some extra wrapper code was needed to make using the scanner easier. First, functions to load the scanner, get the current token, and advance the scanner by one token were written to provide a nicer interface. However, to facilitate "look-aheads" in the Parser, "rewinding" is supported through the buffering functions and the rewind function. This is accomplished by using a stack to represent tokens in the scanner. Without buffering, the token stack remains at size=1, and old tokens are popped when new tokens are read. When buffering is enabled, all old tokens are stored inside of a buffer vector instead of beind discarded. Thus, when `rewind` is called, all tokens in the buffer are placed onto the stack, and reading can proceed as normal. This allows a function to "look ahead" in the token stream, then rewind all of the old tokens to prepare the token stream for the next function to be called. This is used in the `program` non-terminal function.

A struct called `Token` is also defined which represents all of the data associated with a token read by the scanner, along with convenient names for the token numbers.

## Syntax Parser

Syntax parsing is accomplished using a recursive-descent method: each non-terminal in the grammar is represented by a function that can call other non-terminal functions and accept tokens. This ensures that the syntactical structure of the program is valid and follows the rules of the grammar.

Unfortunately due to time constraints the syntax parser is unable to recover from errors gracefully and stops when encountering the first syntax error.

## Semantic Checker

Type errors are handled via the `ExprResult` struct defined in `parser_expr.hpp`. This struct contains a memory location for the computation result or variables to store the literal if necessary, but more importantly stores the type of an expression. This allows the recursive parser to get the result from an intermediate non-terminal and compare it with the other non-terminal or terminal symbol to ensure type correctness. If a type error is found, the parser will continue to run but no code will be generated.

## Intermediate Code Generation

Code is generated within the recursive functions of the parser: as soon as enough information is available to generate some code, the appropriate code is generated and added to the output list. Information from the semantic checker is also used to generate the correct assembly commands for the given type.

The memory allocation policy is very naive and takes advantage of SiC's very simple construction. All variables, including global variables, local variables, and intermediate variables, are statically allocated starting from address 0 and count upward for each variable. Technically speaking this policy shouldn't support recursion, however due to the implementation of MASS it seems that recursion ends up working. Perhaps it's because memory locations are zeroed on each function call? I'm not exactly sure.

Label allocation works almost exactly like the memory allocation policy. All labels including function labels and control flow labels are allocated starting from 0 counting upwards.

Return statements are generated at the end of every function, to ensure that functions return properly even if no return statement is used. This sometimes ends up creating unnecessary or duplicate returns, but otherwise does not affect the rest of the program.

Finally, newlines are generated after write statements for clearer formatting

## Symbol Table

The symbol table stores information about global and local symbols; namely functions, global variables, and local variables. The symbol table is also responsible for memory and label allocation, as it stores the next available memory/label address to use. Under the hood, the symbol table is implemented using two hash-tables; one for local symbols and one for global symbols.

For variables, the symbol table stores its type, the line number it's declared on, and the memory location that it's assigned to. For functions, the symbol table stores its return type, its parameter type, the line number that it's declared/defined in (definition takes priority), the label that it's assigned to, and whether it's a declaration or definition. Global variables and functions are stored in the same table, so an additional variable stores whether the symbol is a function or variable.

When receiving a defined function that has already been declared, the SymbolTable class simply overwrites the declaration with a definition, and readjusts the line number. Additional declarations are ignored, except for updating the line number if a definition hasn't been found yet.

At the end of every function, all local variables are cleared to enforce scoping between functions.

# Files Overview

## Main

Contains the main function that handles the command line arguments and redirects cout and cerr if specified. It then calls the parser function with the given filename.

## Parser Module

Contains all of the functions used to recursively parse the given filename. The parser module is split into several different files to modularize code.

### parser_internal

Contains functions and definitions common to all of the parser files. Currently contains the accept/expect functions and the symbol table declaration (which is declared in parser.cpp)

### parser_expr

Contains functions that parse expressions. Also contains the `ExprResult` struct and a function to help format `ExprResult` structs to correct assembly instructions

### parser_ops

Contains functions that parse operation symbols. Returns what symbol was read to aid `parser_expr`

### parser

Contains all the other non-terminal functions of the recursize descent parsers.

`decl` takes in a bool that states whether the declaration is a global or not. This is needed to ensure that the variable declarations are placed in the correct symbol table.

`varlist` takes in three bools: whether the varlist is a declaration or not, whether it is global, and what type the declaration is. This is needed since `varlist` can be called in both a variable-declaring context or another context. If variables are being declared, then they need to be put onto the symbol table, so the function needs to know whether it's creating variables or not.

`body` takes in a bool that states the return type of the function. This is used simply to pass this infor from `functiondef` to `stmt`.

`stmt` takes in a bool that states the return type of the function. This is used to perform type-checking on return statements

## Scan

Wraps the provided scanner with a cleaner interface, and abstracts the work done to advance the scanner and read the data.

## Symbol

Defines the class used to hold symbol data during parsing.

## VarPrint

Contains functions that perform all of the checking, formatting, and printing when attempting to use or declare a variable/function.

## First

Contains functions that return whether the given token is part of the `FIRST` set of the given non-terminal. Used in the parser to determine which non-terminal to call next.

## common.h/tok.l

Provided with the scanning code