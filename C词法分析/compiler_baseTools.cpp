#include "compiler.h"

compiler::compiler(string path,ostream& out) {
    error = Error();
    noticedChar=0;
    noticedString=0;
    noticedIdentifier=0;
    noticedSymbol=0;
    noticedNumber = 0;
    noticedKeyWord = 0;
    lineNumber = 1;
    charNumber = 0;
    lineStartNumber = 0;
    input = new  ifstream(path);
    openFile = input->is_open();
    output = &out;
};

compiler::~compiler()
{
    for (unordered_map<char, node*>::iterator it = symbolCheckTree.begin(); it != symbolCheckTree.end(); it++) {
        delete it->second;
    };
    input->close();
}

bool compiler::isFileOpen()
{
    return openFile;
}

string compiler::compileInfos()
{
    return
        "\n\n\t\t\t\t<<<<<======== [ Information ] ========>>>>>\t\t\t\t\n\t\t\t\t     " +
        to_string(lineNumber) + " lines | " + to_string(charNumber) + " characters | " + to_string(error.errorNum()) + " errors.\n\t\t" +
        "Noticed : "+to_string(noticedIdentifier)+" identifiers | " +to_string(noticedKeyWord)+" keyWords | "+ to_string(noticedSymbol)+" symbols | " + to_string(noticedChar)+" chars | " + to_string(noticedString) +" strings | " + to_string(noticedNumber)+" numbers \n" +
        error.getInfo();
}


int compiler::incForChar()
{
    if (*forChar == '\0')return Error::EndOfFile;
    forChar = &inputStr[(forChar - inputStr + 1) % LENGTH];
    if (*forChar == EOF) {
        if (&inputStr[LENGTH / 2 - 1] == forChar) {
            if (bufferFlag == false) {
                if (nowChar > &inputStr[LENGTH / 2 - 1]) { return Error::TooLongSymbol; }
                input->read(&inputStr[LENGTH / 2], LENGTH / 2 - 1);
                bufferFlag = true;
            }
            forChar = &inputStr[LENGTH / 2];
        }
        else if (&inputStr[LENGTH - 1] == forChar) {
            if (bufferFlag == true) {
                if (nowChar < &inputStr[LENGTH / 2 - 1]) { return Error::TooLongSymbol; }
                input->read(&inputStr[0], LENGTH / 2 - 1);
                charNumber += LENGTH - 2;
                bufferFlag = false;
            }
            forChar = &inputStr[0];
        }
    }
    else if (*forChar == '\0') {
        charNumber = nowCharNum();
        return Error::EndOfFile;
    }

    return Error::NoError;
}


inline long long  compiler::nowCharNum() {
    return charNumber + (forChar - inputStr >= LENGTH / 2) ? forChar - inputStr  : forChar - inputStr +1;
}

void compiler::errorInfoAppend(int type, string detail)
{
    error.addInfo(lineNumber, nowCharNum()-lineStartNumber, type, detail);
}

