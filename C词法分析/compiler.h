#pragma once
#include <iostream>
#include <unordered_map>
#include <fstream>
#include <string>
#include <set>
#include "error.h"
#include "node.h"
using namespace::std;

class compiler
{
private:
#define LENGTH 2048
    int lineNumber;
    long long  charNumber;
    long long lineStartNumber;

    static set<string>keyWord;
    static unordered_map<string, string> symbolMap;
    unordered_map<char, node*> symbolCheckTree = {};

    ifstream* input;
    ostream* output;
    char inputStr[LENGTH];
    char* nowChar, * forChar;
    bool bufferFlag;//true:12;false:21
    bool openFile;
    Error error;

    int noticedChar;
    int noticedString;
    int noticedIdentifier;
    int noticedSymbol;
    int noticedNumber;
    int noticedKeyWord;

    string symbolCheck();
    string numberCheck();
    string identifierCheck();
    string stringCheck();
    string charCheck();
    string pretreatCheck();
    void  ignoreAnnotation();

    inline long long nowCharNum();
    int incForChar();

public:

    compiler(string path,ostream& out);
    ~compiler();
    bool isFileOpen();
    void initSymbolCheckTree();
    void wordsAnalyze();
    void errorInfoAppend(int type, string detail);
    string compileInfos();

};
