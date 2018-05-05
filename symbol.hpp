//Michael Chen - mzc223 - CS6413 - Spring 2018 - Boris Aronov

#ifndef SYMBOL_HPP
#define SYMBOL_HPP

#include <unordered_map>

/*
 * Defines the symbol table used during parsing to verify variable usage
 */

//No functions may be local
struct LocalSymbol
{
    //Only two types, either int or float
    bool is_int;
    int line_number;

    int memloc;
};

struct GlobalSymbol
{
    bool is_function;
    //Invalid to use if not a function
    bool is_decl;
    //All functions have a parameter
    bool param_is_int;
    //No need to store param id, is stored as local symbol

    bool is_int;
    int line_number;
    //Acts as a label for functions
    int memloc;
};

class SymbolTable
{
public:
    SymbolTable();

    //Methods to insert symbols into the table
    bool insertLocal(std::string id, bool isi, int line);
    bool insertGlobal(std::string id, bool isf, bool isdec, bool pii, bool isi, int line);

    //Clear locals at end of function
    void clearLocal();

    //Get symbols. Invalid id's will return a symbol with a negative line number
    LocalSymbol getLocal(std::string id);
    GlobalSymbol getGlobal(std::string id);

    //Get specific symbols. Will exit if incorrect symbol type
    GlobalSymbol getFunction(std::string id);

    //Manage the memloc counter for local variables
    int getLocalCounter();
    void decrementLocalCounter();

    //Manage the label counter for control statements
    int getLabelCounter();
    void decrementLabelCounter();

    //Halts if not a variable/function or does not exist
    bool isVarInt(std::string id);
    bool isFuncInt(std::string id);

private:
    //Associates a name/identifier with a symbol
    std::unordered_map<std::string, LocalSymbol> localtable;
    std::unordered_map<std::string, GlobalSymbol> globaltable;

    int variablecounter;
    int functionlabelcounter;
};

#endif