#include "../util/linked_list.h"
#include "assert.h"
#include <iostream>

int main() {
    DoublyLinkedList<int> ints;
    ints.push_back(5);
    // assert("asdf", ints.get_front()->data == 5);
    std::cout << "Im proud of you chelsea\n";
};