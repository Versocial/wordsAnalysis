
#include <iostream>
#include <unordered_map>
#include <fstream>
#include <string>
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
    string errorInfos;
    int errorNumber;
public:
    enum Type {
        NoError = 0,
        EndOfFile = 1,
        UndefinedSymbol = 2,
        UndefinedNumber = 3,
        UndefinedIdentifier = 4,
        TooLongSymbol = 5,
        UnkownState = 6,
        UnfinishedAnnotation=7,
        UnfinishedToken=8,
        UnfinishedString=9,
        UnfinishedChar=10
    };
    
    Error() { errorNumber = 0; errorInfos = ""; }

    string errorInfo(int type,string detail) {
        string ans = "Error:: error unkown : ";
        switch (type) {
        case UndefinedSymbol:ans = "Error:: Undefined symbol : "+detail; break;
        case UndefinedNumber:ans = "Error:: UndefinedNumber :  "+detail; break;
        case UndefinedIdentifier:ans="Error:: UndefinedIdentifier : "+detail; break;
        case TooLongSymbol:ans = "Error:: TooLongSymbol : " + detail; break;
        case UnkownState:ans = "Error:: DFA state undefined : "+detail; break;
        case UnfinishedAnnotation:ans = "Error:: Unfinished Annotation : " + detail; break;
        case UnfinishedToken:ans = "Error:: Unfinished Token : "+detail; break;
        case UnfinishedString:ans = "Error:: Unifinished String : " + detail; break;
        case UnfinishedChar:ans = "Error:: UnfinishedChar : " + detail; break;
        case NoError:break;
        default:ans = "Error:: Unkown Error : " + detail; break;
        }
        errorInfos = errorInfos + ans + "\n";
        return ans+"\n";
    }

    string getInfo() { return to_string( errorNumber)+" Error: \n"+ errorInfos; }

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
Error error;
public:
    compiler(string path);
    ~compiler();
    void initSymbolCheckTree();
    string symbolCheck();
    string numberCheck();
    string identifierCheck();
    string stringCheck();
    string charCheck();
    void  ignoreAnnotation();
    static ::unordered_map<string, string> symbolMap;


    void wordsAnalyze();
    int incForChar();


};

unordered_map<string, string> compiler::symbolMap = {
            {"+","plus"},{"++","inc"}, {"+=","plusAssign"},
            {"-","sub"},{"--","dec"},{"-=","decAssign"}, {"->","target"},
            {"*","multiply"},{"*=","multiplyAssign" },
            {"/","devide"},{"/=","devideAssign"},
            {"%","reminder"}, {"%=","reminderAssign"},
            {"<","lessThan"},{"<=","lessEql"},{"<<=","leftShift"},{"<<=","leftShiftEql"},
            {">","greaterThan"},{">=","greaterEql"},{">>","rightShift"},{">>=","rightShiftEql"},
            {"=","assign"},{"==","equal"},
            {"!","not"},{"!=","notEql" },
            {"~","bitNot"},{"~=","bitNotAssign" },
            {"^","bitXor"},{"^=","bitXorAssign"},
            {"&","bitAnd"},{"&=","bitAndAssign"},{"&&","and"},
            {"|","bitOr"},{"|=","bitOrAssign"},{"||","or"},
            {"(","("},{")",")"},{"[","["},{"]","]"},{"{","{"},{"}","}"},
            {".","point"},{"?","question"},{":","colon"},{";","semicolon"},{",","comma"}
};

compiler::compiler(string path) {
    error = Error();
    input = new  ifstream(path);
    if (!input->is_open())cout << "Error:: Wrong Path.\n";
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
            if (isxdigit(*forChar));
            else Exit = true;
            break;
        default://forChar = temp;
            *output << error.errorInfo(Error::UnkownState,"numberCheck DFA unkown state  when manage\""+string(nowChar,forChar-nowChar)+"\"");
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

string compiler::identifierCheck()
{
    forChar = nowChar;
    int state = 0;
    string identifier = "";
    char* temp = forChar;
    while (1) {
        if (isalpha(*forChar) || *forChar == '_' || (isdigit(*forChar) && identifier.size())) {
            identifier += *forChar;
            temp = forChar;
            if (Error::EndOfFile == incForChar()) { forChar = temp; break; }
        }
        else { forChar = temp; break; }
    }

    return "@"+identifier;
}

string compiler::stringCheck()
{
    forChar = nowChar;
    int state = 0;
    bool Exit = false;
    while (!Exit) {
        switch (state)
        {
        case 0:
            if ('\"' == *forChar)state = 1;
            else { error.errorInfo(Error::UnfinishedString, string(nowChar, forChar - nowChar)); Exit = true; }
            break;
        case 1:
            if ('\\' == *forChar)state = 2;
            else if ('\"' == *forChar)Exit = true;
            else;
            break;
        case 2:
            state = 1;
            break;
        default:
            break;
        }
        if (!Exit&&Error::EndOfFile == incForChar()) { error.errorInfo(Error::UnfinishedString,string(nowChar,forChar-nowChar)); break; }
    }
    if (state == 1) return  string(nowChar, forChar - nowChar+1);
    else return "";
}

string compiler::charCheck() {
    forChar = nowChar;
    int state = 0;
    string ch= "";
    char* temp = forChar;
    bool Exit = false;
    while (!Exit) {
        switch (state) {
        case -1:break; 
        case 0:
            if (*forChar == '\'')state = 1;
            else { state = -1; Exit = true; }
            break;
        case 1:
            if (*forChar == '\\')state = 2;
            else if (isprint(*forChar)&&*forChar!='\n')state = 4; 
            else { state = -1; Exit = true; }
            break;
        case 2:
            if (isalpha(*forChar)||*forChar=='\''||*forChar=='\"'||*forChar=='\\')state = 3;
            else { state = -1; Exit = true; }
            break;
        case 3:
            if (*forChar == '\'');
            else state = -1;
            Exit = true;
            break;
        case 4:
            if (*forChar == '\'');
            else state = -1;
            Exit = true;
            break;
        default:Exit = true; break;
        }
        if (!Exit) {
            ch += *forChar;
            temp = forChar;
            if (Error::EndOfFile == incForChar()) { forChar = temp; Exit = true; }
        }
        else {
            if (state != 4 && state != 3) { forChar = temp;  error.errorInfo(Error::UnfinishedChar, ch); }
            else ch += *forChar;
        }
    }
    if (state == 4 || state == 3)    return ch;
    else return "";
}

void compiler::ignoreAnnotation()
{
    forChar = nowChar;
    int state = 1;
    bool Exit = false;
    char* temp = forChar;
    while (!Exit) {
        switch(state) {        
        case 1:
            if (*forChar == '/')state = 2;
            else if (*forChar == '*')state = 4;
            else return;
        case 2:
            if (*forChar == '\n')Exit = true;
            break;
        case 4:
            if (*forChar == '*')state = 5;
            break;
        case 5:
            if (*forChar == '/')Exit = true;
            else if (*forChar == '*');
            else state = 4;
            break;
        }
        if (!Exit) {
            temp = forChar;
            if (Error::EndOfFile == incForChar()){
                forChar = temp; Exit = true;
            }
        }
        else if(state!=2&&state!=5) forChar = temp; 
    }
    if (state != 2 && state != 5) { error.errorInfo(Error::UnfinishedAnnotation, ""); }
    return;
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
            info = identifierCheck();
        }
        else if (*nowChar == '/') {
            if (Error::EndOfFile == incForChar()) { error.errorInfo(Error::UnfinishedToken, "/"); break; }
            else if (*forChar == '*' || *forChar == '/') { nowChar=forChar; ignoreAnnotation(); }
            else { forChar = nowChar; info = symbolCheck(); }
        }
        else if (isspace(*nowChar)) {
            ;
        }
        else if (*nowChar == '\"') {
            info = stringCheck();
        }
        else if (*nowChar == '\'') {
            info = charCheck();
        }
        else if (symbolCheckTree.count(*nowChar)) {
            info = symbolCheck();
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
