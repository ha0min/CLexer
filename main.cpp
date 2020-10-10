/**
 * Copyright 2020 MaxCheng. All Right Reserved.
 * Project: CLexer
 * Author: MaxCheng
 * Date: 2020.9.23
 */

#include "CLexer.h"
#include "fstream"
using namespace std;

int main() {
    ifstream in;
    ofstream out("./out.txt");

    string originSource;//原源文档

    in.open("./test.txt");
    if(!in.is_open()){
        cout << "error opening file!"<<endl;
        exit(0);
    }

    while(!in.eof()){
        originSource += in.get();
    }
    originSource += '\0';
    in.close();

    cout << "源程序为: " << endl << originSource << endl;
    cout << "---------------" << endl;

    filterSource = inputFilter(originSource);
    cout << "过滤后程序为: " << endl << filterSource << endl;
    cout << "---------------" << endl;

    cout << "开始词法分析... " << endl;
    cLexer();
    cout << "词法分析完成！！" << endl;
    cout << "---------------" << endl;

    cout << "开始打印... " << endl;
    printList(listOfAllToken, out);
    cout << "打印完成！！" << endl;
    out.close();
    cout << "---------------" << endl;

    return 0;
}