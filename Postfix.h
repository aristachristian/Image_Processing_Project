#include <iostream>
#include <string>
#include <cmath>
#include "Stack.h"

using namespace std;

string convertToPostFix(const string);
int evaluatePostfixExpression(const string);
bool isOperator(const char);
bool precedence(const char, const char);
int precedenceValue(const char);
int calculate(const int, const int, const char);

string convertToPostFix(const string infix)
{
    Stack <char> c;
    string postfix = "";
    c.push(0);

    for (size_t i = 0; infix[i] != '='; ++i)
    {
        if (isdigit(infix[i]))
        {
            for (; isdigit(infix[i]); ++i)
            {
                postfix.push_back(infix[i]);
            }
            --i;
            postfix.push_back(' ');
        }else if (isOperator(infix[i]))
        {
            while (precedence(c.stackTop(), infix[i]))
            {
                postfix.push_back(c.stackTop());
                postfix.push_back(' ');
                c.pop();
            }
            c.push(infix[i]);
        }
    }

    while (c.stackTop() != 0)
    {
        postfix.push_back(c.stackTop());
        postfix.push_back(' ');
        c.pop();
    }
    c.pop();

    return postfix;
}

int evaluatePostfixExpression(const string postfix)
{
    int result = 0;
    Stack <int> num;

    size_t length = postfix.length();
    int a, b;
    for (size_t i = 0, temp = 0; i < length; ++i)
    {
        if (isdigit(postfix[i]))
        {
            temp = 0;
            for (; isdigit(postfix[i]); ++i)
            {
                temp = 10 * temp + (postfix[i] - '0');
            }
            num.push(temp);
        }else if (isOperator(postfix[i]))
        {
            a = num.stackTop();
            num.pop();
            b = num.stackTop();
            num.pop();

            result = calculate(a, b, postfix[i]);

            num.push(result);
        }
    }
    
    result = num.stackTop();
    num.pop();

    return result;
}

bool isOperator(const char c)
{
    return (c == '+' || c == '-' || c == '*' ||
        c == '/') ? true : false;
}

bool precedence(const char op1, const char op2)
{
    return (precedenceValue(op1) >= precedenceValue(op2)) ?
        true : false;
}

int precedenceValue(const char op)
{
    if (op == '*' || op == '/')
        return 2;
    else if (op == '+' || op == '-')
        return 1;

    return 0;
}

int calculate(const int a, const int b, const char op)
{
    if (op == '+')
        return b + a;
    else if (op == '-')
        return b - a;
    else if (op == '*')
        return b * a;
    else if (op == '/')
        return b / a;

    return 0;
}
