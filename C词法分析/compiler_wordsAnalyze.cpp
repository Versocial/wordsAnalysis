#include "compiler.h"
using namespace::std;

set<string> compiler::keyWord = {
    "int", "long", "short", "float", "double", "char", 
    "unsigned", "signed", "const", "void", "volatile", "enum", "struct", "union",
    "if", "else", "goto", "switch", "case", "do", "while", "for", "continue", "break", "return", "default", "typedef",
    "auto", "register", "extern", "static",
    "sizeof"
};


unordered_map<string, string> compiler::symbolMap = {
            {"+","plus"},{"++","inc"}, {"+=","plusAssign"},
            {"-","sub"},{"--","dec"},{"-=","decAssign"}, {"->","target"},
            {"*","star"},{"*=","multiplyAssign" },
            {"/","devide"},{"/=","devideAssign"},
            {"%","reminder"}, {"%=","reminderAssign"},
            {"<","lessThan"},{"<=","lessEql"},{"<<","leftShift"},{"<<=","leftShiftAssign"},
            {">","greaterThan"},{">=","greaterEql"},{">>","rightShift"},{">>=","rightShiftAssign"},
            {"=","assign"},{"==","equal"},
            {"!","not"},{"!=","notEql" },
            {"~","bitNot"},{"~=","bitNotAssign" },
            {"^","bitXor"},{"^=","bitXorAssign"},
            {"&","bitAnd"},{"&=","bitAndAssign"},{"&&","and"},
            {"|","bitOr"},{"|=","bitOrAssign"},{"||","or"},
            {"(","-"},{")","-"},{"[","-"},{"]","-"},{"{","-"},{"}","-"},
            {".","point"},{"?","question"},{":","colon"},{";","semicolon"},{",","comma"}
};

void compiler::wordsAnalyze()
{

    input->read(inputStr, LENGTH / 2 - 1);
    inputStr[LENGTH / 2 - 1] = inputStr[LENGTH - 1] = EOF;
    nowChar = forChar = inputStr;
    bufferFlag = false;
    string info="";

    while (*nowChar) {
        info .clear();
        if (isdigit(*nowChar)) {
            info = numberCheck();
        }
        else if (isalpha(*nowChar) || *nowChar == '_') {
            info = identifierCheck();
        }
        else if (*nowChar == '/') {
            if (Error::EndOfFile == incForChar()) { errorInfoAppend(Error::UnfinishedToken, "/"); break; }
            else if (*forChar == '*' || *forChar == '/') { nowChar = forChar; ignoreAnnotation(); }
            else { forChar = nowChar; info = symbolCheck(); }
        }
        else if (isspace(*nowChar)) {
            if (*nowChar == '\n') {
                info += '\n'; lineNumber++;
                lineStartNumber = nowCharNum();
            }
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
        else if (*forChar == '#') {
            info = "\n"+pretreatCheck();
        }
        else {
            while (!isspace(*forChar)) {
                if (Error::EndOfFile == incForChar())break;
                info += *forChar;
            };
            errorInfoAppend(Error::UnkownInput, "\"" + info + "\"");
            info = "<?word,\"" + info + "\>";           

        };
        if (!info.empty()) *output << info << "\t";
        if (Error::EndOfFile == incForChar()) {
            break;
        }
        nowChar = forChar;
    }
    return;

}

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
        if (Error::EndOfFile == incForChar())return "<?symbol,"+symbol+">";
        now = now->seekSon(*forChar);
        if (now == NULL) { forChar = temp; break; }
        symbol += *forChar;
    }
    noticedSymbol++;
    return  "<" + symbol + ",->";//"<"+symbol+","+symbolMap[symbol]+">";
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
                }
                else if (isdigit(*forChar)) {
                    number += 'x';
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
            errorInfoAppend(Error::UnkownState, "numberCheck DFA unkown state  when manage\"" + number + "\"");
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
    case 0:case 12:case 4:number = "<integer," + number+">"; noticedNumber++; break;
    case 1:case 2:case 3:number = "<realNumber," + number+">"; noticedNumber++; break;
    default:number = "<?number," + number+">"; break;
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
    
    if (keyWord.count(identifier)) {
        noticedKeyWord++;
        return "<"+ identifier+",->";
    }
    else {
        noticedIdentifier++;
        return "<identifier," + identifier+">";
    }
}

string compiler::stringCheck()
{
    forChar = nowChar;
    int state = 0;
    bool Exit = false;
    string ans = "";
    while (!Exit) {
        switch (state)
        {
        case 0:
            if ('\"' == *forChar) {
                ans += *forChar;
                state = 1;
            }
            else {
                errorInfoAppend(Error::UnfinishedString, ans); 
                Exit = true; 
            }
            break;
        case 1:
            if ('\\' == *forChar) { ans += *forChar; state = 2; }
            else if ('\"' == *forChar) { ans += *forChar; Exit = true; }
            else if ('\n' == *forChar||!isprint(*forChar)) { 
                Exit = true; 
                errorInfoAppend(Error::UnfinishedString, ans);
               if('\n'==*forChar) lineNumber++;
            }
            else ans += *forChar;
            break;
        case 2:
            if (*forChar == '\n') { ans.pop_back(); lineNumber++; }
            else if (!isprint(*forChar)) {
                Exit = true;
                errorInfoAppend(Error::UnfinishedString, ans);
            }
            else ans += *forChar;
            state = 1;
            break;
        default:
            break;
        }               
        if (!Exit && Error::EndOfFile == incForChar()) { errorInfoAppend(Error::UnfinishedString,ans); break; }
    }
    if (state == 1) { noticedString++; return "<string,"+ ans+">"; }
    else return "<?string,"+ans+">";
}

string compiler::charCheck() {
    forChar = nowChar;
    int state = 0;
    string ch = "";
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
            else if (isprint(*forChar) && *forChar != '\n')state = 4;
            else { state = -1; Exit = true; }
            break;
        case 2:
            if (isalpha(*forChar) || *forChar == '\'' || *forChar == '\"' || *forChar == '\\')state = 3;
            else { state = -2; Exit = true; }
            break;
        case 3:
            if (*forChar == '\'')
                Exit = true;
            else
                state = -2;
            break;
        case 4:
            if (*forChar == '\'')
                Exit = true;
            else if (*forChar == '\\')
                state = -3;
            else 
                state = -2;
            break;
        case -2:
            if (*forChar == '\'')
                Exit = true;
            else if (*forChar == '\\')
                state = -3;
            else;
            break;
        case -3:
            state = -2;
            break;
        default:Exit = true; break;
        }
        ch += *forChar;
        if (!Exit) {
            temp = forChar;
            if (Error::EndOfFile == incForChar()) { errorInfoAppend(Error::UnfinishedChar, ch); forChar = temp; Exit = true; }
        }
        else {
            if (*forChar != '\''){
                forChar = temp;
                ch.pop_back();
            }
            if (state != 4 && state != 3) {
                if (state == -2)errorInfoAppend(Error::UnsupportedChar, ch);
                else errorInfoAppend(Error::UnfinishedChar,ch);
            }            
        }
    }
    if (state == 4 || state == 3) {
        noticedChar++;    return "<char," + ch + ">";
    }
    else return "<?char,"+ ch +">";
}

void compiler::ignoreAnnotation()
{
    forChar = nowChar;
    int state = 1;
    bool Exit = false;
    char* temp = forChar;
    while (!Exit) {
        switch (state) {
        case 1:
            if (*forChar == '/')state = 2;
            else if (*forChar == '*')state = 4;
            else return;
        case 2:
            if (*forChar == '\n')Exit = true;
            break;
        case 4:
            if (*forChar == '*')state = 5;
            else if (*forChar == '\n')lineNumber++;
            break;
        case 5:
            if (*forChar == '/')Exit = true;
            else if (*forChar == '*');
            else state = 4;
            break;
        }
        if (!Exit) {
            temp = forChar;
            if (Error::EndOfFile == incForChar()) {
                forChar = temp; Exit = true;
            }
        }
        else if (state != 5) forChar = temp;
    }
    if (state != 2 && state != 5) { errorInfoAppend(Error::UnfinishedAnnotation, ""); }
    return;
}

string compiler::pretreatCheck()
{
    char* temp=forChar;
    while ('\n' != (*forChar)) {
        temp = forChar;
        if (Error::EndOfFile == incForChar())break;
    };
    string info=string(nowChar,forChar-nowChar);
    nowChar = forChar=temp;
    return "<#pretreat,"+info+">";
}