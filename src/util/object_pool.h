#ifndef object_pool_h
#define object_pool_h

#include <cstddef>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <sys/wait.h>
#include <cstring>
#include <stdlib.h>

// Note to self, template classes must reside entirely in header files
// Compiler needs access to all source to generate code for template
// https://stackoverflow.com/a/1353981/761726
// http://www.parashift.com/c++-faq-lite/templates-defn-vs-decl.html

template <typename T> class ObjectPool {
private:
  T *block;
  T *next_free_space;

  int max_num_obj;
  int cur_num_obj;

  // this is a stack of free spaces
  T **free_spaces;
  // original pointer to stack of free spaces.
  T **free_space_block;
  int num_free_spaces;

public:
  ObjectPool(int max_num_obj_);
  ~ObjectPool();
  T *allocate();
  void del(T *obj);
  int num_obj_stored();
  int num_random_free_spaces();
  void cleanup();
};

template <typename T>
ObjectPool<T>::ObjectPool(int max_num_obj_) {
  max_num_obj = max_num_obj_;

  size_t block_size = sizeof(T) * max_num_obj;
  block = (T*)malloc(block_size);
  memset(block, 0, block_size);

  next_free_space = block;
  num_free_spaces = 0;
  cur_num_obj = 0;

  // Keep track of free spaces in a stack in case we delete an object that's not
  // at end of array.
  size_t free_space_size = sizeof(T*) * max_num_obj;
  free_spaces = (T**)malloc(free_space_size);
  memset(free_spaces, 0, free_space_size);
  free_space_block = free_spaces;
};

template <typename T> void ObjectPool<T>::cleanup() {
  free(block);
  free(free_space_block);
}

template <typename T> ObjectPool<T>::~ObjectPool() {  }

template <typename T> int ObjectPool<T>::num_obj_stored() {
  return cur_num_obj;
}

template <typename T> int ObjectPool<T>::num_random_free_spaces() {
  return num_free_spaces;
}

template <typename T> T *ObjectPool<T>::allocate() {
  if (cur_num_obj + 1 > max_num_obj) {
    throw std::runtime_error("Could not allocate obj");
  }

  cur_num_obj++;

  // if we want to overwrite some existing spaces
  if (num_free_spaces > 0) {

    // subtract from number of existing spaces.
    num_free_spaces--;
    // Pointer in free spaces always points to null so decrementing gives us a
    // pointer to a good memory location.
    --free_spaces;
    return *free_spaces;
  }

  // Return a pointer to the next free space & increment.
  return next_free_space++;
};

template <typename T> void ObjectPool<T>::del(T *obj) {
  // We don't wipe memory here. Up to caller to take care of allocation /
  // setting.

  if (obj > (block + max_num_obj)) {
    throw std::runtime_error(
        "Attempted to free obj outside of stack allocator bounds.");
  }

  cur_num_obj--;

  // if object is deleting something already allocated.
  if (obj < next_free_space) {
    if (num_free_spaces + 1 > max_num_obj) {
      SPDLOG_CRITICAL("Unexpected condition in object pool. {} {}", num_free_spaces, max_num_obj);
      throw std::runtime_error("Unexpected condition in object pool");
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
