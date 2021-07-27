#ifndef STACK_H
#define STACK_H

#include <iostream>
#include "Node.h"
using namespace std;

template <typename TYPE> class Stack
{
public:
    Stack()
        :topOfStack(nullptr), bottomOfStack(nullptr)
    {

    }

    void push(const TYPE d)
    {
        Node <TYPE> *newStack = createNewStackNode(d), *tempPtr;

        if (isEmpty())
        {
            bottomOfStack = topOfStack = newStack;
        }else
        {
            tempPtr = topOfStack;
            topOfStack = newStack;
            tempPtr->nextPtr = topOfStack;
        }
    }

    TYPE stackTop() const
    {
        return topOfStack->data; 
    }

    void pop()
    {
        if (isEmpty())
        {
            cout << "List is already empty!\n";
        }

        if (topOfStack == bottomOfStack)
        {
            topOfStack = bottomOfStack = nullptr;
        }else
        {
            Node <TYPE> *currentPtr = bottomOfStack;

            while (currentPtr->nextPtr != topOfStack)
            {
                currentPtr = currentPtr->nextPtr;
            }
            
            currentPtr->nextPtr = nullptr;
            delete topOfStack;
            topOfStack = currentPtr;
        }
    }

    bool isEmpty() const
    {
        if (bottomOfStack == nullptr)
            return true;
        else
            return false;
    }

    void printStack() const
    {
        Node <TYPE> *iteratorPtr = bottomOfStack;

        cout << "Stack (Bottom to Top) : ";
        while (iteratorPtr != nullptr)
        {
            cout << iteratorPtr->data << ' ';
            iteratorPtr = iteratorPtr->nextPtr;
        }
        
    }

    ~Stack()
    {

        while (!isEmpty())
        {
            pop();
        }
    }

private:
    Node <TYPE> *topOfStack;
    Node <TYPE> *bottomOfStack;

    Node <TYPE> *createNewStackNode(const TYPE d) const
    {
        return new Node <TYPE> (d);
    }
};

#endif
