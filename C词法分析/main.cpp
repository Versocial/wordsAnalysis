#include "compiler.h"
using namespace::std;
int main()
{
    string path;
    cin >> path;
    compiler wordsAnalyzer = compiler(path,cout);
    if (!wordsAnalyzer.isFileOpen()) {
        cout << "Can Not Open File \"" << path << "\" .\n";
        return 1;
    }
    wordsAnalyzer.initSymbolCheckTree();
    wordsAnalyzer.wordsAnalyze();
    cout << wordsAnalyzer.compileInfos();
    cout << ">>>>>>  Words Analyze Complete !\n";
    system("pause");
    return 0;
}