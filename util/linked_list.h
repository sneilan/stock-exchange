#ifndef linked_list_h
#define linked_list_h

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
    DoublyLinkedList() : head(nullptr), tail(nullptr) {}

    Node<T> * push_back(T val);
    Node<T> * get_front();
    Node<T> * get_back();
    void pop_front();
    void remove(Node<T>* node);
};

#endif