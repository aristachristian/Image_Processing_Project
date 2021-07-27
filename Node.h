#ifndef NODE_H
#define NODE_H

template <typename TYPE> class Stack;

template <typename TYPE> class Node
{
    friend class Stack <TYPE>;

public:
    explicit Node(TYPE d)
        :data(d), nextPtr(nullptr)
    {

    }

private:
    TYPE data;
    Node *nextPtr;
};

#endif
