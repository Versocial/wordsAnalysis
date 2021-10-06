#include "compiler.h"
using namespace::std;
int main()
{
    string path;
    cin >> path;
   compiler wordsAnalyzer(path,cout); 
   if(!wordsAnalyzer.fileOpened()){
        cout << "Can Not Open File \"" << path << "\" .\n";
        return 1;
    }
    wordsAnalyzer.wordsAnalyze();
    cout << wordsAnalyzer.compileInfos();
    cout << ">>>>>>  Words Analyze Complete !\n";
    system("pause");
    return 0;
}