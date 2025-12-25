#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cctype>
#include <map>

using namespace std;

// 关键字映射表
map<string, string> keywords = {
    {"const", "CONSTTK"}, {"int", "INTTK"}, {"char", "CHARTK"},
    {"void", "VOIDTK"}, {"main", "MAINTK"}, {"if", "IFTK"},
    {"else", "ELSETK"}, {"do", "DOTK"}, {"while", "WHILETK"},
    {"for", "FORTK"}, {"scanf", "SCANFTK"}, {"printf", "PRINTFTK"},
    {"return", "RETURNTK"}
};

// 判断是否为单字符符号的辅助函数 (用于快速查表)
string getSingleCharToken(char c) {
    switch (c) {
        case '+': return "PLUS";
        case '-': return "MINU";
        case '*': return "MULT";
        case '/': return "DIV";
        case ';': return "SEMICN";
        case ',': return "COMMA";
        case '(': return "LPARENT";
        case ')': return "RPARENT";
        case '[': return "LBRACK";
        case ']': return "RBRACK";
        case '{': return "LBRACE";
        case '}': return "RBRACE";
        default: return "";
    }
}

// 全局变量用于处理文件流
ifstream inFile;
ofstream outFile;

// 获取下一个非空白字符
char getNextChar() {
    char c;
    if (inFile.get(c)) {
        return c;
    }
    return EOF;
}

// 回退一个字符 (当多读了一个字符用于判断时)
void ungetChar(char c) {
    inFile.unget();
}

void analyze() {
    char ch;
    while ((ch = getNextChar()) != EOF) {
        // 1. 跳过空白字符
        if (isspace(ch)) {
            continue;
        }

        // 2. 识别 标识符 (IDENFR) 或 关键字 (Keyword)
        if (isalpha(ch) || ch == '_') {
            string token = "";
            token += ch;
            // 继续读取直到不是字母、数字或下划线
            while ((ch = getNextChar()) != EOF && (isalnum(ch) || ch == '_')) {
                token += ch;
            }
            if (ch != EOF) ungetChar(ch); // 回退多读的字符

            // 查表判断是关键字还是标识符
            if (keywords.count(token)) {
                outFile << keywords[token] << " " << token << endl;
            } else {
                outFile << "IDENFR" << " " << token << endl;
            }
        }
        // 3. 识别 整型常量 (INTCON)
        else if (isdigit(ch)) {
            string num = "";
            num += ch;
            while ((ch = getNextChar()) != EOF && isdigit(ch)) {
                num += ch;
            }
            if (ch != EOF) ungetChar(ch);
            outFile << "INTCON" << " " << num << endl;
        }
        // 4. 识别 字符串常量 (STRCON)
        else if (ch == '"') {
            string str = "";
            while ((ch = getNextChar()) != EOF && ch != '"') {
                str += ch;
            }
            // 题目要求输出单词值，根据样例，STRCON Hello World 不带引号
            outFile << "STRCON" << " " << str << endl;
        }
        // 5. 识别 字符常量 (CHARCON)
        else if (ch == '\'') {
            string charVal = "";
            while ((ch = getNextChar()) != EOF && ch != '\'') {
                charVal += ch;
            }
            // 题目样例 CHARCON _ 不带引号
            outFile << "CHARCON" << " " << charVal << endl;
        }
        // 6. 识别 操作符 (双字符 或 单字符)
        else {
            if (ch == '<') {
                char nextCh = getNextChar();
                if (nextCh == '=') {
                    outFile << "LEQ" << " <=" << endl;
                } else {
                    ungetChar(nextCh);
                    outFile << "LSS" << " <" << endl;
                }
            } else if (ch == '>') {
                char nextCh = getNextChar();
                if (nextCh == '=') {
                    outFile << "GEQ" << " >=" << endl;
                } else {
                    ungetChar(nextCh);
                    outFile << "GRE" << " >" << endl;
                }
            } else if (ch == '=') {
                char nextCh = getNextChar();
                if (nextCh == '=') {
                    outFile << "EQL" << " ==" << endl;
                } else {
                    ungetChar(nextCh);
                    outFile << "ASSIGN" << " =" << endl;
                }
            } else if (ch == '!') {
                char nextCh = getNextChar();
                if (nextCh == '=') {
                    outFile << "NEQ" << " !=" << endl;
                } else {
                    // 根据文法表，! 单独出现没有定义，这里暂且按语法错误或忽略处理
                    // 但为了程序健壮性，回退并忽略或报错
                    ungetChar(nextCh);
                }
            } else {
                // 处理单字符符号 (+, -, *, /, ;, ,, (, ), [, ], {, })
                string code = getSingleCharToken(ch);
                if (code != "") {
                    outFile << code << " " << ch << endl;
                } else {
                    // 未知符号
                    // cout << "Unknown character: " << ch << endl;
                }
            }
        }
    }
}

int main() {
    // 打开输入文件
    inFile.open("testfile.txt");
    if (!inFile.is_open()) {
        cerr << "Error: Cannot open testfile.txt" << endl;
        return 1;
    }

    // 打开输出文件
    outFile.open("output.txt");
    if (!outFile.is_open()) {
        cerr << "Error: Cannot open output.txt" << endl;
        inFile.close();
        return 1;
    }

    // 执行词法分析
    analyze();

    // 关闭文件
    inFile.close();
    outFile.close();

    return 0;
}