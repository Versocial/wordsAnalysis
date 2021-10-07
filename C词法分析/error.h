#pragma once
#include<string>
class Error
{
private:

    std::string errorInfos;
    int errorNumber;

public:

    Error() {
        errorNumber = 0;
        errorInfos = std::string("");
    }

    enum Type {
        NoError = 0,
        EndOfFile = 1,
        TooLongToken = 5,
        UnkownState = 6,
        UnfinishedAnnotation = 7,
        UnfinishedToken = 8,
        UnfinishedString = 9,
        UnfinishedChar = 10,
        UnkownInput = 11,
        UnsupportedChar=12
    };

    std::string addInfo(int lineNumber, int rowNumber, int type, std::string detail) {
        std::string ans = "Error:: error unkown : ";
        switch (type) {
        case TooLongToken:ans = "Error:: TooLongSymbol : " + detail; break;
        case UnkownState:ans = "Error:: DFA state undefined : " + detail; break;
        case UnfinishedAnnotation:ans = "Error:: Unfinished Annotation : " + detail; break;
        case UnfinishedToken:ans = "Error:: Unfinished Token : " + detail; break;
        case UnfinishedString:ans = "Error:: Unifinished String : " + detail; break;
        case UnfinishedChar:ans = "Error:: UnfinishedChar : " + detail; break;
        case UnkownInput:ans = "Error:: Unkown Input code : " + detail; break;
        case UnsupportedChar:ans = "Error:: UnSupported char : " + detail; break;
        case NoError:return ""; break;
        default:ans = "Error:: Unkown Error : " + detail; break;
        }
        errorInfos = errorInfos + "Position ( " + std::to_string(lineNumber) + " , " + std::to_string(rowNumber) + " ) >> " + ans + "\n";
        errorNumber++;
        return ans + "\n";
    }


    std::string getInfo() { return std::to_string(errorNumber) + " Error: \n" + errorInfos; }

    int errorNum() { return errorNumber; }
};
