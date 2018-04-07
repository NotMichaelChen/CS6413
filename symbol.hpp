#ifndef SYMBOL_HPP
#define SYMBOL_HPP

#include <unordered_map>

//issue: decl symbols vs def symbols (just overwrite symbol)
//issue: multiple local symbols with same name, different scopes (nonissue: local symbols are cleared at end of definition)
//question: are functions and variables allowed to have the same name? (assume for now that they don't)

//No functions may be local
struct LocalSymbol
{
    //Only two types, either int or float
    bool is_int;
    int line_number;
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
};

class SymbolTable
{
public:
    bool insertLocal(std::string id, bool isi, int line);
    bool insertGlobal(std::string id, bool isf, bool isdec, bool pii, bool isi, int line);

    void clearLocal();

    //Invalid id's will return a symbol with a negative line number
    LocalSymbol getLocal(std::string id);
    GlobalSymbol getGlobal(std::string id);

private:
    //Associates a name/identifier with a symbol
    std::unordered_map<std::string, LocalSymbol> localtable;
    std::unordered_map<std::string, GlobalSymbol> globaltable;
};;

#endif