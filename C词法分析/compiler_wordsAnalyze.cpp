#include "compiler.h"


set<string> compiler::keyWord = {
    "int", "long", "short", "float", "double", "char", 
    "unsigned", "signed", "const", "void", "volatile", "enum", "struct", "union",
    "if", "else", "goto", "switch", "case", "do", "while", "for", "continue", "break", "return", "default", "typedef",
    "auto", "register", "extern", "static",
    "sizeof"
};

void compiler::wordsAnalyze()
{

    input->read(inputStr, LENGTH / 2 - 1);
    inputStr[LENGTH / 2 - 1] = inputStr[LENGTH - 1] = EOF;
    nowChar = forChar = inputStr;
    bufferFlag = false;
    string info;

    while (*nowChar) {
        info = "";
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
            };
            errorInfoAppend(Error::UnkownInput,"\""+ string(nowChar, forChar-nowChar) + "\"" );

        };
        if (!info.empty()) *output << info << " ";
        if (Error::EndOfFile == incForChar()) {
            break;
        }
        nowChar = forChar;
    }
    return;

}

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
    noticedSymbol++;
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
            errorInfoAppend(Error::UnkownState, "numberCheck DFA unkown state  when manage\"" + string(nowChar, forChar - nowChar) + "\"");
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
    case 0:case 12:case 4:number = "$I" + number; noticedNumber++; break;
    case 1:case 2:case 3:number = "$F" + number; noticedNumber++; break;
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
    
    if (keyWord.count(identifier)) {
        noticedKeyWord++;
        return  identifier;
    }
    else {
        noticedIdentifier++;
        return "@" + identifier;
    }
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
            else { errorInfoAppend(Error::UnfinishedString, string(nowChar, forChar - nowChar)); Exit = true; }
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
        if (!Exit && Error::EndOfFile == incForChar()) { errorInfoAppend(Error::UnfinishedString, string(nowChar, forChar - nowChar)); break; }
    }
    if (state == 1) { noticedString++; return  string(nowChar, forChar - nowChar + 1); }
    else return "";
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
        if (!Exit) {
            temp = forChar;
            if (Error::EndOfFile == incForChar()) { errorInfoAppend(Error::UnfinishedChar, string(nowChar, forChar - nowChar + 1)); forChar = temp; Exit = true; }
        }
        else {
            if (*forChar != '\'') 
                forChar = temp;
            if (state != 4 && state != 3) {
                if (state == -2)errorInfoAppend(Error::UnsupportedChar, string(nowChar, forChar - nowChar + 1));
                else errorInfoAppend(Error::UnfinishedChar, string(nowChar, forChar - nowChar + 1));
            }            
        }
    }
    if (state == 4 || state == 3) {
        noticedChar++;    return string(nowChar,forChar-nowChar+1);
    }
    else return "";
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
    while ('\n' != (*forChar)) {
        if (Error::EndOfFile == incForChar())break;
    };
    string info=string(nowChar,forChar-nowChar);
    nowChar = forChar;
    return info;
}