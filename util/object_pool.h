#ifndef stack_allocator_h
#define stack_allocator_h

#include "mmap_wrapper.h"
#include <stdexcept>
#include <spdlog/spdlog.h>

// Note to self, template classes must reside entirely in header files
// Compiler needs access to all source to generate code for template https://stackoverflow.com/a/1353981/761726
// http://www.parashift.com/c++-faq-lite/templates-defn-vs-decl.html

template <typename T>
class MMapObjectPool {
private:
  T* block;
  T* next_free_space;

  // Need to track number of objects allocated so we don't overflow capacity.
  int max_num_obj;
  int cur_num_obj;

  T** free_spaces;
  int num_free_spaces;
  MMap_Info * mmap_info;
  bool is_controller;
public:
  MMapObjectPool(int max_num_obj, const char * pool_name, bool _is_controller);
  // allocate should return an offset
  // or there should be a function that given the pointer from allocate, returns the offset
  // that can be passed into a ring buffer for use in other processes.
  T* allocate();
  void free(T* obj);
  int num_obj_stored();
  int num_random_free_spaces();
  void cleanup();
};

template <typename T>
void MMapObjectPool<T>::cleanup() {
  if (is_controller) {
    delete_mmap(mmap_info);
  } else {
    close_mmap(mmap_info);
  }
}

template <typename T>
int MMapObjectPool<T>::num_obj_stored() {
  return cur_num_obj;
}

template <typename T>
int MMapObjectPool<T>::num_random_free_spaces() {
  return num_free_spaces;
}

template <typename T>
MMapObjectPool<T>::MMapObjectPool(int max_num_obj_, const char * pool_name, bool _is_controller) {
  is_controller = _is_controller;

  max_num_obj = max_num_obj_;

  if (is_controller) {
    mmap_info = init_mmap(pool_name, sizeof(T) * max_num_obj);
  } else {
    mmap_info = open_mmap(pool_name, sizeof(T) * max_num_obj);
  }

  block = (T*)mmap_info->location;

  next_free_space = block;
  num_free_spaces = 0;
  cur_num_obj = 0;

  // Keep track of free spaces in a stack in case we delete an object that's not at end of array.
  free_spaces = new T*[max_num_obj];
};

template <typename T>
T* MMapObjectPool<T>::allocate() {
  if (cur_num_obj + 1 > max_num_obj) {
    throw std::runtime_error("Could not allocate obj");
  }

  cur_num_obj++;

  if (num_free_spaces > 0) {

    num_free_spaces--;
    // Pointer in free spaces always points to null so decrementing gives us a pointer to a good memory location.
    --free_spaces;
    return *free_spaces;
  }

  // Return a pointer to the next free space & increment.
  return next_free_space++;
};

template <typename T>
void MMapObjectPool<T>::free(T* obj) {
  // We don't wipe memory here. Up to caller to take care of allocation / setting.

  if (obj > (block + max_num_obj)) {
    throw std::runtime_error("Attempted to free obj outside of stack allocator bounds.");
  }

  cur_num_obj--;

  if (obj < next_free_space) {
    if (num_free_spaces + 1 > max_num_obj) {
      throw std::runtime_error("Unexpected condition.");
    }

    // push location of obj we are freeing onto stack of memory spaces
    // We can give to other objects.
    *free_spaces = obj;
    // Point stack at null so we can put another object reference there.
    free_spaces++;
    num_free_spaces++;
  } else {
    // Only use new memory if we run out of randomly deleted spaces.
    next_free_space--;
  }
}

#endif
