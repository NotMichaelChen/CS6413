//Michael Chen - mzc223 - CS6413 - Spring 2018 - Boris Aronov

#include "symbol.hpp"

#include <iostream>

SymbolTable::SymbolTable() : globalcounter(1), localcounter(999), labelcounter(1)
{}

bool SymbolTable::insertLocal(std::string id, bool isi, int line)
{
    auto iter = localtable.find(id);
    if(iter == localtable.end())
    {
        localtable[id] = {isi, line, localcounter};
        localcounter--;
        if(localcounter <= globalcounter)
        {
            std::cerr << "Error: ran out of space when allocating local variables" << std::endl;
            exit(1);
        }

        return true;
    }
    else
    {
        std::cerr << "Error: inserting local " << id << " when it already exists on line " << iter->second.line_number
            << std::endl;
        return false;
    }
}

bool SymbolTable::insertGlobal(std::string id, bool isf, bool isdec, bool pii, bool isi, int line)
{
    auto iter = globaltable.find(id);
    if(iter == globaltable.end())
    {
        int addr = -1;
        //func
        if(isf)
        {
            addr = labelcounter;
            labelcounter++;
        }
        //variable decl
        else
        {
            addr = globalcounter;
            globalcounter++;
            if(globalcounter >= localcounter)
            {
                std::cerr << "Error, ran out of space when allocating global variables" << std::endl;
                exit(1);
            }
        }

        globaltable[id] = {isf, isdec, pii, isi, line, addr};
        return true;
    }
    //Check if we have a decl and are receiving a def, and check that def and decl are the same
    else if(iter->second.is_function && iter->second.is_decl && isf && !isdec && iter->second.param_is_int == pii &&
            iter->second.is_int == isi)
    {
        //Only change relevant variables
        iter->second.is_decl = false;
        iter->second.line_number = line;
        return true;
    }
    else
    {
        std::cerr << "Error: inserting global " << id << " when it already exists on line " << iter->second.line_number
            << std::endl;
        return false;
    }
}

void SymbolTable::clearLocal()
{
    localtable.clear();
    localcounter = 999;
}

LocalSymbol SymbolTable::getLocal(std::string id)
{
    auto iter = localtable.find(id);
    if(iter != localtable.end())
        return iter->second;
    else
        return {false, -1, -1};
}

GlobalSymbol SymbolTable::getGlobal(std::string id)
{
    auto iter = globaltable.find(id);
    if(iter != globaltable.end())
        return iter->second;
    else
        return {false, false, false, false, -1, -1};
}

GlobalSymbol SymbolTable::getFunction(std::string id)
{
    GlobalSymbol func = getGlobal(id);
    if(func.line_number < 0)
    {
        std::cerr << "Error: getting function " << id << " when it hasn't been declared" << std::endl;
        exit(1);
    }
    else if(!func.is_function)
    {
        std::cerr << "Error: using variable " << id << " as a function (declared in line " << func.line_number << std::endl;
        exit(1);
    }

    return func;
}

int SymbolTable::getLocalCounter()
{
    return localcounter;
}

void SymbolTable::decrementLocalCounter()
{
    localcounter--;
    if(localcounter <= globalcounter)
    {
        std::cerr << "Error: ran out of space when allocating local variables" << std::endl;
        exit(1);
    }
}

bool SymbolTable::isVarInt(std::string id)
{
    auto localiter = localtable.find(id);
    if(localiter != localtable.end())
    {
        return localiter->second.is_int;
    }
    else
    {
        auto globaliter = globaltable.find(id);
        if(globaliter != globaltable.end())
        {
            if(!globaliter->second.is_function)
                return globaliter->second.is_int;
            else
            {
                std::cerr << "Error, trying to use function " << id << " as variable" << std::endl;
                exit(1);
            }
        }
        else
        {
            std::cerr << "Error, trying to get type of undefined identifier '" << id << "'" << std::endl;
            exit(1);
        }
    }
}

bool SymbolTable::isFuncInt(std::string id)
{
    auto globaliter = globaltable.find(id);
    if(globaliter != globaltable.end())
    {
        if(globaliter->second.is_function)
            return globaliter->second.is_int;
        else
        {
            std::cerr << "Error, trying to use variable " << id << " as function" << std::endl;
            exit(1);
        }
    }
    else
    {
        std::cerr << "Error, trying to get type of undefined identifier '" << id << "'" << std::endl;
        exit(1);
    }
}