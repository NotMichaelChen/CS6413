Michael Chen (mzc223) - Spring 2018

# CS6413 Project Part 3: Semantic Checking

## Build Instructions

SiCParser is built with the provided makefile, and requires `g++` and `flex` to compile the flex file and the c++ source files. To build, simply type `make` in the main directory, which will then compile into a binary called `out` in the `exe` folder.

The binary takes at most 2 arguments: a filename and an optional arbitrary string that indicates whether to redirect output to a file. Leaving out the 2nd argument will direct output to the terminal.

For reference, development was done with `g++:7.2.0` and `flex:2.6.1` on an Ubuntu 17.10 system.

## Files Overview

### Main

Contains the main function that handles the command line arguments and redirects cout and cerr if specified. It then calls the parser function with the given filename.

### Parser Module

Contains all of the functions used to recursively parse the given filename. Each non-terminal is represented by a function. The parser module is split into several different files to modularize code.

#### parser_internal

Contains functions and definitions common to all of the parser files. Currently contains the accept/expect functions and the symbol table declaration (which is declared in parser.cpp)

#### parser_expr

Contains functions that parse expressions. All functions return a bool that indicate whether the result is an int or bool. This is used to do type checking inside of the expression.

#### parser_ops

Contains functions that parse operation symbols. Separated to clear clutter from the parser file.

#### parser

Contains all the other non-terminal functions of the recursize descent parsers.

Only two functions take parameters: `decl` and `varlist`.

`decl` takes in a bool that states whether the declaration is a global or not. This is needed to ensure that the variable declarations are placed in the correct symbol table.

`varlist` takes in three bools: whether the varlist is a declaration or not, whether it is global, and what type the declaration is. This is needed since `varlist` can be called in both a variable-declaring context or another context. If variables are being declared, then they need to be put onto the symbol table, so the function needs to know whether it's creating variables or not.

### Scan

Wraps the provided scanner with a cleaner interface, and abstracts the work done to advance the scanner and read the data.

The header file defines the interface to the Scanner, which includes functions to load the scanner, get the current token, and advance the scanner by one token. However, to facilitate "look-aheads" in the Parser, "rewinding" is supported through the buffering functions and the rewind function. This is accomplished by using a stack to represent tokens in the scanner. Without buffering, the token stack remains at size=1, and old tokens are popped when new tokens are read. When buffering is enabled, all old tokens are stored inside of a buffer vector instead of beind discarded. Thus, when `rewind` is called, all tokens in the buffer are placed onto the stack, and reading can proceed as normal. This allows a function to "look ahead" in the token stream, then rewind all of the old tokens to prepare the token stream for the next function to be called. This is used in the `program` non-terminal function.

The header file also defines a struct called `Token` which represents all of the data associated with a token read by the scanner, along with convenient names for the token numbers.

### Symbol

Defines the class used to hold symbol data during parsing.

The SymbolTable class maintains both the local and global symbol table. Local symbols can only be variables, so the LocalSymbol struct only keeps data required for variables, which is their type (represented as a bool) and their line number. Global symbols can also be functions, so global symbols also store whether the symbol is a function, whether it's a declaration or a definition, and what its parameter type is. None of these variables are used if the symbol is not a function.

When receiving a defined function that has already been declared, the SymbolTable class simply overwrites the declaration with a definition, and readjusts the line number.

Symbols returned from the SymbolTable class that don't exist return a falsified struct with a negative line number. The negative line number is used to determine whether the result exists in the symbol table or not.

### VarPrint

Contains functions that perform all of the checking, formatting, and printing when attempting to use or declare a variable/function.

### First

Contains functions that return whether the given token is part of the `FIRST` set of the given non-terminal. Used in the parser to determine which non-terminal to call next.

### common.h/tok.l

Provided with the scanning code