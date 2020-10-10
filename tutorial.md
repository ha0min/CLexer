# 开篇

编译，简单地理解，就是把源程序转化为另一种形式的程序,而其中关键的部分就是理解源程序所要表达的意思，才能转化为另一种源程序。

编译器的输入就是这样的一个语言源文件。

词法分析器的作用，就是拿到输入的文件，分离出这个输入文件的每个元素（关键字、变量、符号等），然后根据该种语言的文法，分析这些元素的组合是否合法，以及这些组合所表达的意思。

简而言之，**词法分析器对源码字符串做预处理，以减少语法分析器的复杂程度。**

词法分析器以源码字符串为输入，它的输出是标记流（token stream），即一连串的标记，每个标记通常包括： `(token, token _value)` 即标记本身和标记的值。

# 实现

弄清楚词法分析器的基本原理之后，我们就需要对分析器的运行步骤进行分析。

在搞清基本原理后，我们可以总结出，对于一个词法分析器：

1. 它的输入是一个字符串文本；
2. 它需要过滤掉源程序中的多余字符；
3. 之后对这个文本进行分析；
4. 保存分析结果`(token)`和对应样本值`(token_value)`；
5. 最后输出这些结果。

在本文中，我们会一步步地实现这些功能。

## 数据结构

首先，需要考虑*如何保存我们分析的结果*。这样在进行后续的编程时，我们会有一个统一的路线。

这里，需要的输出包括两部分内容，即词法分析器的样本分析结果`(token)`和对应样本值`(token_value)`。

分析的结果有五类，分别是**标识符**、**保留字**、**常数**、**运算符**、**界符**。用一个枚举可以很好地描述它们：

```c++
//token 的类型
enum tokenKind {
    keyword = 1,
    identifier,
    num,
    operators,
    delimiter
};
```

同时，在本文中，我们还特别地**保存了样本在过滤文本中的所在行。**

因此，保存的结果需要有顺序，这样才能保证输出结果中，会*一行一行*地输出词语。

这种情况下，我采用**链表**进行数据的存储，相关定义如下：

```c++
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
```

我定义了一个全局变量`listOfAllToken`，用来链接、保存所有的 token 。

同时，还定义了两个链表相关的方法，分别执行插入和打印的操作，插入链表的具体实现如下：

```c++
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
```

这里的 `if` 判断是用于辨别首个 token，我定义的 tokenList 在用指针 p 引用时貌似没办法判定对应 token 是否为空。（都说了指针要慎用要慎用！）

打印操作的实现如下：

```c++
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
```

这里的输出由行号和对应分析结果组成，样例为：

```c++
line 1:
(0, )
```

定义了一个常数 `outputNum` 用于判断终端一行的输出个数，虽然执行起来好像差别不是很大，但相信对于代码行数多的文件会更明显一些。

另外借助文件流`ofstream`我们将结果输出到外部文件保存,使其更加易读。

## 输入的过滤

确定好数据结构后，我们就开始逐步构建相关的实现方法了。

首先来对源文本进行过滤。

在本词法分析器中，我们需要删去制表符和注释。因为删除空格会导致无法识别`int i`和`inti`的现象，所以空格的删去在后续的词法分析中实现。

同时需要注意，因为分析器保存和输出行号的特性，这里**我们不对除多行注释外的换行符进行处理**。

过滤方法的具体实现如下：

```c++
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
```

## 分析的实现

拿到过滤后的文本字符串后，也就到了最重要的词法分析环节了。

敲代码之前，让我们先分析一下这个分析词法过程的重点：

- 判断类别的标准：**词首个字符的类别**、与保留字表是否重复
- 词与词如何**分割**：(对某些类型词）前后字符不是同一类型、空格、换行符

### 首个字符类型的判断

通过单词的首个字符，我们可以快速分析输入词的类型：数字开头的一定是数字，字母开头的可能是关键词也可能是标识符，保留特殊符号一定是运算符或界符。

因此我们可以将首个字符的类型分为字母、下划线、数字、运算符和界符，需要分别写函数判断，并返回布尔结果方便后续输出。

```c++
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
```

### 识别单词并生成 token

完成首字符的识别后就要进行词与词分割。

这个过程的判断使用**一个位置参数`pos`对 String 字符串进行定位**来确定的，如果前后**字符**不能构成一个词就回退指针`pos--`，否则一直向后查询。

对于界符 token ，找到插入就好了：

```
int delimiterToken(int pos, int cur_line){
    token delimiter_token;
    string delimiter_token_value;

    delimiter_token_value += filterSource[pos++];

    //生成界符类型token结点并插入
    delimiter_token.setToken(delimiter, delimiter_token_value, cur_line);
    insertIntoList(listOfAllToken, delimiter_token);

    return pos;
}
```

对于数字开头 token 进行的操作也非常简单，只需要查找到不是数字即可：

```
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
```

对于字母开头的 token，需要设置一个判断函数用于判断是否为关键词，当返回值为假时则为保留字 token，否则按关键词 token 插入链表中：

```
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
```

运算符开头的 token 则稍显麻烦，因为需要**超前判断**，当输入为`<`号时，需要判断是`<<`还是`<=` ，这就需要`pos`向前移位的判断：

```
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
```

至此，有关分析判断的函数就都已经写好了，我们只需要提供一个有效的输入，词法分析器就可以运行起来了！

## 最后的拼装

前面说到，*通过单词的首个字符，我们可以快速分析输入词的类型：数字开头的一定是数字，字母开头的可能是关键词也可能是标识符，保留特殊符号一定是运算符或界符。*

但判断条件过多，使用`if..else..`语句在主方法体显得有些累赘，于是我们用`switch`封装成一个函数：

```
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
```

之后，我们就可以**根据首个字符返回值，快速使用不同 token 生成插入函数**了：

```
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
```

至此，所有词法分析器的内容就完结啦～～

# 后记

作为学校编译原理课的实验内容花了近一个星期来完成，因为很久没写 Cpp 代码，再次拾起来花了不少功夫，不过运行结果还是很令人满意的嘿嘿。

你可以在[这里](https://github.com/maxiscute/CLexer)查看和下载源码，一起加油～

