#ifndef linked_list_h
#define linked_list_h

#include <stdexcept>

// Needed internal implementation of linked list instead of std::list
// because if you have multiple iterators on std::list and you delete nodes
// from one of the iterators, the other iterators get invalidated.
// I needed a way to maintain pointers to orders, delete orders w/o invalidating
// my other pointers to orders.

template <typename T>
struct Node {
    T data;
    Node<T>* prev;
    Node<T>* next;
    Node(T val) : data(val), prev(nullptr), next(nullptr) {}
};

template <typename T>
class DoublyLinkedList {
private:
    Node<T>* head;
    Node<T>* tail;
public:
    DoublyLinkedList();
    Node<T> * push_back(T val);
    Node<T> * get_front();
    Node<T> * get_back();
    void pop_front();
    void remove(Node<T>* node);
};

template <typename T>
DoublyLinkedList<T>::DoublyLinkedList() {
    head = nullptr;
    tail = nullptr;
}

template <typename T>
Node<T> * DoublyLinkedList<T>::get_front() {
    return head;
}

template <typename T>
Node<T> * DoublyLinkedList<T>::get_back() {
    return tail;
}

template <typename T>
Node<T> * DoublyLinkedList<T>::push_back(T val)
{
    Node<T> *newNode = new Node<T>(val);
    if (head == nullptr)
    {
        head = newNode;
        tail = newNode;
    }
    else
    {
        tail->next = newNode;
        newNode->prev = tail;
        tail = newNode;
    }
    return newNode;
}

template <typename T>
void DoublyLinkedList<T>::pop_front()
{
    if (head == nullptr)
    {
        return;
    }
    Node<T> *nodeToRemove = head;

    head = head->next;

    if (head != nullptr)
    {
        head->prev = nullptr;
    }
    else
    {
        tail = nullptr;
    }

    delete nodeToRemove;
}

template <typename T>
void DoublyLinkedList<T>::remove(Node<T> *node)
{
    if (node == nullptr)
    {
        return;
    }
    if (node == head)
    {
        head = node->next;
        if (head != nullptr)
        {
            head->prev = nullptr;
        }
    }
    else if (node == tail)
    {
        tail = node->prev;
        if (tail != nullptr)
        {
            tail->next = nullptr;
        }
    }
    else
    {
        node->prev->next = node->next;
        node->next->prev = node->prev;
    }

    delete node;
}

#endif