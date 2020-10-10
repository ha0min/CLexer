//
// Created by Max Cheng on 2020/9/23.
//

#include "CLexer.h"

void insertIntoList(tokenList &list, token newToken) {
    //新建结点
    tokenList *newTokenList = new tokenList;
    newTokenList->token = newToken;
    newTokenList->next = nullptr;

    tokenList *p = &list;

    if (p->token.kind == 0 && p->token.value == "" && p->token.line == 0){
        p->token = newToken;
    }else{
        //将token插入链表末尾
        while (p->next){
            p = p->next;
        }
        p->next = newTokenList;
    }
}

void printList(tokenList list, ofstream &outputFile){
    tokenList *p = &list;//指向 list 的指针
    int outputNum = 0;//行输出计数器

    //开始输出
    cout << "line 1: " << endl;
    outputFile << "line 1: " << endl;

    while (p && p->next){

        //输出token，以(kind, value)的形式
        cout << "(" << p->token.kind << ", " << p->token.value << ")";
        outputFile << "(" << p->token.kind << ", " << p->token.value << ")";

        //控制每行输出数量
        outputNum++;
        if (outputNum >= 9){
            cout << endl;
            outputFile << endl;

            outputNum = 0;
        } else{
            cout << "   ";
            outputFile << "   ";
        }

        //输出行号
        if (p->token.line != p->next->token.line){
            cout << endl;
            cout << "line " << p->next->token.line << ": " << endl;//输出行号

            outputFile << '\n';
            outputFile << "line " << p->next->token.line << ": " << '\n';

            outputNum = 0;//归零输出计数器
        }

        p = p->next;
    }

    cout << "(" << p->token.kind << ", " << p->token.value << ")" << endl;
    cout << "end of list" << endl;

    outputFile << "(" << p->token.kind << ", " << p->token.value << ")" << '\n';
}

string inputFilter(string inputFile) {
    string tmp;
    int pos = 0;

    for (pos; pos < inputFile.length(); pos++) {
        //单行注释过滤
        if (inputFile[pos] == '/' && inputFile[pos + 1] == '/') {
            while (inputFile[pos] != '\n') {
                pos++;
            }
        }

        //多行注释过滤
        if (inputFile[pos] == '/' && inputFile[pos + 1] == '*') {
            while (!(inputFile[pos + 1] == '/' && inputFile[pos] == '*')) {
                //判断注释符是否合拢
                if (inputFile[pos] == '\0') {
                    cout << "annotation error!" << endl;
                    exit(0);
                }
                ++pos;
            }
            pos += 2;
        }

        if (inputFile[pos] != '\v' && inputFile[pos] != '\t') {
            tmp += inputFile[pos];
        }
    }

    return tmp;
}

bool isDigit(char ch) {
    if (ch >= '0' && ch <= '9') {
        return true;
    }
    return false;
}

bool isAlpha(char ch) {
    if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_'){
        return true;
    }
    return false;
}

bool isOperator(char ch){

    char operators[13]={
            '+', '-', '*', '/', '%', '<', '>', '=', '&', '|', '!','\'','&'
    };

    for (char i : operators) {
        if (ch == i){
            return true;
        }
    }

    return false;
}

bool isDelimiter(char ch) {
    char delimiter[]={
            '(',   ')',   ',',   ';',  '{',  '}', '\"', '#', '\'', '.', ':'
    };

    for (char i : delimiter) {
        if (ch == i){
            return true;
        }
    }

    return false;
}

bool isKeyword(string token) {

    string KeyWord[32] = {
            "auto", "break", "case", "char", "const", "continue",
            "default", "do", "double", "else", "enum", "extern",
            "float", "for", "goto", "if", "int", "long",
            "register", "return", "short", "signed", "sizeof", "static",
            "struct", "switch", "typedef", "union", "unsigned", "void",
            "volatile", "while"
    };

    for (string a : KeyWord){
        if (token.compare(a) == 0){
            return true;
        }
    }

    return false;
}

void cLexer() {
    int pos = 0;
    int cur_line = 1;

    for (pos; pos < filterSource.length(); pos++) {
        while (filterSource[pos] != ' ' && pos < filterSource.length()) {

            switch (startCharType(filterSource[pos])) {
                case 1:
                    pos = numToken(pos, cur_line);
                    break;
                case 2:
                    pos = alphaToken(pos, cur_line);
                    break;
                case 3:
                    pos = delimiterToken(pos, cur_line);
                    break;
                case 4:
                    pos = operatorToken(pos,cur_line);
                    break;
                case 5:
                    cout << "Unknown Character in line " << cur_line << " as [ "<< filterSource[pos] <<" ]" << endl;
                    cout << "..." << endl;
                    pos++;
                case 6:
                    //换行符
                    cur_line++;
                    pos++;
                    break;
                default:
                    pos++;
                    break;
            }
        }
    }
}

int startCharType(char ch)
{
    int type = 0;
    if (isDigit(ch)){
        type = 1;
    }else{
        if (isAlpha(ch)){
            type = 2;
        }else{
            if (isDelimiter(ch)){
                type = 3;
            }else{
                if (isOperator(ch)){
                    type = 4;
                }else {
                    if (ch == '\n'){
                        type = 6;
                    }else {
                        type = 5;
                    }
                }
            }
        }
    }
    return type;
}

int numToken(int pos, int cur_line){
    token num_token;
    string num_token_value;

    num_token_value += filterSource[pos++];

    //数字类型
    while(isDigit(filterSource[pos]) || filterSource[pos] == '.'){
        num_token_value += filterSource[pos++];
    }

    //生成数字类型token结点并插入
    num_token.setToken(num, num_token_value, cur_line);
    insertIntoList(listOfAllToken, num_token);

    //返回分析进度最新位置
    return pos;
}

int alphaToken(int pos, int cur_line){
    token alpha_token;
    string alpha_token_value;
    alpha_token_value += filterSource[pos++];

    //后面字符是字母或数字
    while(isAlpha(filterSource[pos]) || isDigit(filterSource[pos])){
        alpha_token_value += filterSource[pos++];
    }

    //查表,若不是保留字则是标识符
    if(isKeyword(alpha_token_value)){
        alpha_token.setToken(keyword, alpha_token_value, cur_line);
    }else{
        alpha_token.setToken(identifier,alpha_token_value,cur_line);
    }

    insertIntoList(listOfAllToken, alpha_token);

    return pos;
}

int operatorToken(int pos, int cur_line){
    token operator_token;
    string operator_token_value;

    if (filterSource[pos] == '=' && filterSource[pos + 1] == '=' ){
        operator_token_value = "==";
        pos++;
    }else if(filterSource[pos] == '>' && filterSource[pos + 1] == '='){
        operator_token_value = ">=";
        pos++;
    }else if(filterSource[pos] == '>' && filterSource[pos + 1] == '>'){
        operator_token_value = ">>";
        pos++;
    }else if(filterSource[pos] == '<' && filterSource[pos + 1] == '='){
        operator_token_value = "<=";
        pos++;
    }else if(filterSource[pos] == '<' && filterSource[pos + 1] == '<'){
        operator_token_value = "<<";
        pos++;
    }else if(filterSource[pos] == '!' && filterSource[pos + 1] == '='){
        operator_token_value = "!=";
        pos++;
    }else if(filterSource[pos] == '&' && filterSource[pos + 1] == '&'){
        operator_token_value = "&&";
        pos++;
    }else if(filterSource[pos] == '|' && filterSource[pos + 1] == '|'){
        operator_token_value = "||";
        pos++;
    }else{
        operator_token_value = filterSource[pos++];
    }

    //生成操作符类型token结点并插入
    operator_token.setToken(operators, operator_token_value, cur_line);
    insertIntoList(listOfAllToken, operator_token);

    //返回分析进度最新位置
    return pos;
}

int delimiterToken(int pos, int cur_line){
    token delimiter_token;
    string delimiter_token_value;

    delimiter_token_value += filterSource[pos++];

    //生成界符类型token结点并插入
    delimiter_token.setToken(delimiter, delimiter_token_value, cur_line);
    insertIntoList(listOfAllToken, delimiter_token);

    return pos;
}