#include "linked_list.h"

template <typename T>
Node<T> * get_front() {
    return head;
}

template <typename T>
Node<T> * get_back() {
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
void remove(Node<T> *node)
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