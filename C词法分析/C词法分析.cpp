
#include <iostream>
#include <unordered_map>
#include <fstream>
using namespace::std;

class node;

class Error;

class compiler;

class node
{
private:
    char ch;
    node* peer;
    node* son;
public:
    node(char ch) {
        this->ch = ch;
        peer = NULL;
        son = NULL;
    }
    ~node() {
        delete peer;
        delete son;
    }
    node* newSon(char ch) {
        node* temp = new node(ch);
        temp->peer = this->son;
        son = temp;
        return temp;
    }
    node* newPeer(char ch) {
        node* temp= new node(ch);
        temp->peer = this->peer;
        this->peer = temp;
        return temp;
    }
    bool hasSon() { return son != NULL; }
    bool hasPeer() { return peer != NULL; }
    node* Son() { return son; }
    node* Peer() { return peer; }
    node* seekPeer(char charac){
        if (this->ch == charac)return this;
        for (node* now = peer; now != NULL; now = now->peer) {
            if (now->ch == charac)return now;
        }return NULL;
    }
    node* seekSon(char charac) {
        return this->hasSon() ? this->Son()->seekPeer(charac) : NULL;
    }
};

class Error 
{
private:

public:
    enum Type {
        NoError = 0,
        EndOfFile = 1,
        UndefinedSymbol = 2,
        UndefinedNumber = 3,
        UndefinedIdentif = 4,
        TooLongSymbol = 5,
        UnkownState = 6
    };
   static string errorInfo(int type) {
        string ans = "Error:: error unkown";
        switch (type) {
        case UndefinedSymbol:ans = ""; break;
        case UndefinedNumber:break;
        case UndefinedIdentif:break;
        case TooLongSymbol:break;
        case UnkownState:ans = "Error:: DFA state undefined."; break;
        }
        return ans;
    }
};


class compiler
{
private:
unordered_map<char, node*> symbolCheckTree = {};
ifstream* input;
ostream* output;
#define LENGTH 2048
char inputStr[LENGTH];
char* nowChar, * forChar;
bool bufferFlag;//true:12;false:21
public:
    compiler(string path);
    ~compiler();
    void initSymbolCheckTree();
    string symbolCheck();
    string numberCheck();
    static ::unordered_map<string, string> symbolMap;


    void wordsAnalyze();
    int incForChar();


};

unordered_map<string, string> compiler::symbolMap = {
            {"+","plus"},{"++","inc"}, {"+=","plusAssign"},
            {"-","sub"},{"--","dec"},{"-=","decAssign"}, {"->","target"},
            {">","greaterThan"},{">=","greaterEql"},{">>","rightShift"},{">>=","rightShiftEql"},
            {"=","assign"},{"==","equal"}
};

compiler::compiler(string path) {
    input = new  ifstream(path);
    if (!input->is_open())cout << "Error:: Wrong Path\n";
    output = &cout;
};

compiler::~compiler()
{
    for (unordered_map<char, node*>::iterator it = symbolCheckTree.begin(); it != symbolCheckTree.end(); it++) {
        delete it->second;
    };
    input->close();
    // output->close();
};

void compiler::initSymbolCheckTree() {
    for (unordered_map<string, string>::iterator it = symbolMap.begin(); it != symbolMap.end(); it++) {
        node* now = NULL;
        for (char ch : it->first) {
            if (now == NULL) {
                if (symbolCheckTree[ch] == NULL)symbolCheckTree[ch] = new node(ch);
                now = symbolCheckTree[ch];
            }
            else {
                node* temp = now->seekSon(ch);
                if (temp != NULL) {
                    now = temp;
                }
                else {
                    now = now->newSon(ch);
                }
            }

        }

    }
}

string compiler::symbolCheck()
{
    forChar = nowChar;
    node* now = symbolCheckTree[*nowChar];
    string symbol = ""; symbol += *nowChar;
    while (now->hasSon()) {
        char* temp = forChar;
        if (Error::EndOfFile == incForChar())return "";
        now = now->seekSon(*forChar);
        if (now == NULL) { forChar = temp; break; }
        symbol += *forChar;
    }
    return symbolMap[symbol];
}

string compiler::numberCheck()
{
    forChar = nowChar;
    int state = 0;
    string number = "";
    char* temp = forChar;
    bool Exit = false;
    
    while (!Exit) {
        switch (state)
        {
        case 0:
            if (isdigit(*forChar));
            else if (*forChar == '.')state = 1;
            else if (*forChar == 'f') state = 3;
            else if (*forChar == 'E' || *forChar == 'e')state = 12;
            else if (*forChar == 'x') {
                if (number != "0")Exit = true;
                if (Error::EndOfFile == incForChar()) {
                    Exit = true;
                }else if(isdigit(*forChar)){
                    number+='x';
                    state = 4;
                }
                else {
                    Exit = true;
                }
            }
            else Exit = true;
            break;
        case 1:
            if (isdigit(*forChar));
            else if (*forChar == 'E' || *forChar == 'e')state = 2;
            else if (*forChar == 'f')  state = 3; 
            else Exit = true;
            break;
        case 2://float
        case 12://integer
            if (isdigit(*forChar));
            else if ((*forChar == '+' || *forChar == '-') && (*temp == 'E' || *temp == 'e'));
            else if (*forChar == 'f') state = 3;
            else Exit = true;
                break;
        case 3:
            Exit = true;
            break;
        case 4:
            if (isdigit(*forChar));
            else Exit = true;
            break;
        default://forChar = temp;
            *output << Error::errorInfo(Error::UnkownState);
            Exit = true;
            break;
        }
        if (!Exit) {
            number += *forChar;
            temp = forChar;
            if (Error::EndOfFile == incForChar()) {
                forChar = temp; Exit = true;
            }
        }
        else forChar = temp;
    }

    switch (state) {
    case 0:case 12:case 4:number = "$I" + number; break;
    case 1:case 2:case 3:number = "$F" + number; break;
    default:number = "$?" + number; break;
    }
    return number;
}

void compiler::wordsAnalyze()
{

    input->read(inputStr, LENGTH/2-1);
    inputStr[LENGTH / 2-1] = inputStr[LENGTH - 1] = EOF;
    nowChar = forChar = inputStr;
    bufferFlag = false;
    string info ;
    while (*nowChar) {
        info = "";
        if (isdigit(*nowChar)) {
            info=  numberCheck();
        }
        else if (isalpha(*nowChar) || *nowChar == '_') {

        }
        else if (isspace(*nowChar)) {
            ;
        }
        else if (symbolCheckTree.count(*nowChar)) {
            info = symbolCheck();
        }
        else if (*nowChar == '\"'){

        }
        else if (*nowChar == '\''){

        }
        else {

        };
       if(info.size()) *output << info<<" ";
        if (Error::EndOfFile == incForChar())break;
        nowChar = forChar;
    }
    return;

}

int compiler::incForChar()
{
    if (*forChar == EOF) {
        if (&inputStr[LENGTH / 2 - 1] == forChar&&bufferFlag==false) {
            if (nowChar > &inputStr[LENGTH / 2 - 1]) { return Error::TooLongSymbol; }
            input->read(&inputStr[LENGTH / 2], LENGTH / 2 - 1);
            forChar = &inputStr[LENGTH / 2];
        }
        else if (&inputStr[LENGTH - 1] == forChar&&bufferFlag==true) {
            if (nowChar < &inputStr[LENGTH / 2 - 1]) { return Error::TooLongSymbol; }
            input->read(&inputStr[0], LENGTH / 2 - 1);
            forChar = &inputStr[0];
        }
        else {
            return Error::EndOfFile;//FILE END SO NEVER INC ANYMORE AND THIS FUNCTION WILL ALWAYS RETURN Erorr::EndOfFile
        }
    }
    else forChar++;
    return Error::NoError;
}


int main()
{
    compiler Compiler = compiler("test.txt");
    Compiler.initSymbolCheckTree();
    Compiler.wordsAnalyze();
    std::cout << "Hello World!\n";
    system("pause");
}
