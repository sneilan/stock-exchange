#include "../util/linked_list.h"
#include "assert.h"
#include <iostream>

int main() {
    DoublyLinkedList<int> ints;
    ints.push_back(5);
    ints.push_back(6);
    ints.push_back(7);
    ints.push_back(8);

    ASSERT("5", ints.get_front()->data == 5)
    ints.pop_front();
    ASSERT("6", ints.get_front()->data == 6)
};