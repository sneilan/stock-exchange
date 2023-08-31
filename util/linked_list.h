#ifndef linked_list_h
#define linked_list_h

#include <stdexcept>
#include <iostream>

// #define LL_DEBUG(x) std::cout << x << "\n";
#define LL_DEBUG(x)

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
  int total;
public:
  DoublyLinkedList();
  Node<T> * push_back(T val);
  Node<T> * get_front();
  Node<T> * get_back();
  void pop_front();
  void remove(Node<T>* node);
  int get_total();
};

template <typename T>
DoublyLinkedList<T>::DoublyLinkedList() {
  head = nullptr;
  tail = nullptr;
  total = 0;
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
int DoublyLinkedList<T>::get_total() {
  return total;
}

template <typename T>
Node<T> * DoublyLinkedList<T>::push_back(T val)
{
  Node<T> *newNode = new Node<T>(val);
  if (head == nullptr) {
    head = newNode;
    tail = newNode;
  }
  else {
    tail->next = newNode;
    newNode->prev = tail;
    tail = newNode;
  }
  total++;
  return newNode;
}

template <typename T>
void DoublyLinkedList<T>::pop_front()
{
  if (head == nullptr) {
    return;
  }
  Node<T> *nodeToRemove = head;

  head = head->next;

  if (head != nullptr) {
    head->prev = nullptr;
  }
  else {
    tail = nullptr;
  }

  total--;
  delete nodeToRemove;
}

template <typename T>
void DoublyLinkedList<T>::remove(Node<T> *node) {
  if (node == nullptr) {
    return;
  }
  if (node == head) {
      LL_DEBUG(1);
      head = node->next;
      LL_DEBUG(2);
      if (head != nullptr) {
        LL_DEBUG(3);
        head->prev = nullptr;
        LL_DEBUG(4);
      }
    }
  else if (node == tail) {
    tail = node->prev;
    LL_DEBUG(5);
    if (tail != nullptr) {
      tail->next = nullptr;
      LL_DEBUG(6);
    }
  }
  else {
    LL_DEBUG(7);
    node->prev->next = node->next;
    LL_DEBUG(8);
    node->next->prev = node->prev;
    LL_DEBUG(9);
  }
  total--;
  LL_DEBUG(10);
  delete node;
  LL_DEBUG(11);
}

#endif
