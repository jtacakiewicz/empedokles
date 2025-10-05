
#ifndef EMP_STACKED_LINKED_LIST_H
#define EMP_STACKED_LINKED_LIST_H
namespace emp {

template <class T> class StackLinkedList {
public:
    struct Node {
        T data;
        Node *next;
    };

    Node *head;

public:
    StackLinkedList() = default;
    StackLinkedList(StackLinkedList &stackLinkedList) = delete;
    void push(Node *newNode)
    {
        newNode->next = head;
        head = newNode;
    }

    Node *pop()
    {
        Node *top = head;
        head = head->next;
        return top;
    }
};

}  //  namespace emp
#endif
