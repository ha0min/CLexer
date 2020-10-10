//
// Created by Max Cheng on 2020/9/23.
//


#ifndef CLEXER_CLEXER_H
#define CLEXER_CLEXER_H

#include<iostream>
#include<algorithm>
#include<string>
using namespace std;

//token 的类型
enum tokenKind {
    keyword = 1,
    identifier,
    num,
    operators,
    delimiter
};

//存储 token 信息的结构体
struct token{
    enum tokenKind kind;
    string value;
    int line;

    void setToken(tokenKind newKind, string newValue, int curLine){
        this->kind = newKind;
        this->value = newValue;
        this->line = curLine;
    }
};

//用于存放所有 token 的链表结构
struct tokenList {
    token token;
    tokenList *next;
};

string inputFilter(string inputFile);
void insertIntoList(tokenList &list, token newToken);
bool isOperator(char ch);
bool isDelimiter(char ch);
bool isAlpha(char ch);
bool isDigit(char ch);
bool isKeyword(string token);
int alphaToken(int pos, int cur_line);
int numToken(int pos, int cur_line);
int operatorToken(int pos, int cur_line);
int delimiterToken(int pos, int cur_line);
int startCharType(char ch);
void cLexer();
void printList(tokenList list, ofstream &outputFile);

string filterSource;
tokenList listOfAllToken;

#endif //CLEXER_CLEXER_H
