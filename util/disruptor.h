// Implementation of ring buffer for processes to communicate with each other
// See https://martinfowler.com/articles/lmax.html
//
// Basic concept is producer should never produce more than consumer can consume.
// and Consumer should never consume more than consumer can produce.
// Otherwise it's not a ring buffer anymore it's a snake eating it's own tail.
//
// The consumer can consume up until and including the producer position
// But the producer position can only produce up until the consumer position.

#include "mmap_wrapper.h"
#include <spdlog/spdlog.h>

template <typename T> struct SharedData {
  int producer_position;
  int consumer_position;
  T *entities;
};

template <typename T> class Disruptor {
protected:
  MMap_Info *mmap_info;
  SharedData<T> *shared_mem_region;
  int get_mmap_size();
  int slots;
};

template <typename T> class Producer : public Disruptor<T> {
public:
  Producer(int slots, const char *mmap_name);
  ~Producer() throw();
  void cleanup();
  // This creates a copy on function call for simplicity.
  // Later on use pointers to improve performance.
  bool put(T item);
};

template <typename T> class Consumer : public Disruptor<T> {
public:
  // producer / consumer behavior is totally separate but
  // we have to specify slots twice.
  // For now this is fine because producer and consumer are
  // created in separate processes anyway.
  Consumer(int slots, const char *mmap_name);
  ~Consumer() throw();
  T *get();
  void cleanup();
};

template <typename T> T *Consumer<T>::get() {
  // SPDLOG_DEBUG("{} Producer/Consumer is {}/{}", this->mmap_info->name,
  //              this->shared_mem_region->producer_position,
  //              this->shared_mem_region->consumer_position);

  // Consumer can consume up until the producer position. Producer is not allowed to produce > consumer position - 1
  // So producers next position it will write to is it's current position and the last position it wrote to is
  // position - 1. Then consumer can consume only up to producer position - 1. It's a real mind bender but it works.
  if (this->shared_mem_region->consumer_position == this->shared_mem_region->producer_position) {
    return nullptr;
  }

  T *item = &this->shared_mem_region
                 ->entities[this->shared_mem_region->consumer_position];
  this->shared_mem_region->consumer_position++;
  this->shared_mem_region->consumer_position %= this->slots;

  return item;
}

template <typename T> int Disruptor<T>::get_mmap_size() {
  return sizeof(SharedData<T>) + sizeof(T) * slots;
}

template <typename T> void Producer<T>::cleanup() {
  delete_mmap(this->mmap_info);
}

template <typename T> Producer<T>::~Producer() throw() { cleanup(); }

template <typename T> Producer<T>::Producer(int slots, const char *mmap_name) {
  this->slots = slots;
  this->mmap_info = init_mmap(mmap_name, this->get_mmap_size());
  this->shared_mem_region = (SharedData<T> *)this->mmap_info->location;

  this->shared_mem_region->entities = reinterpret_cast<T *>(
      (char *)this->shared_mem_region + sizeof(SharedData<T>));

  // producer always starts ahead of consumer
  this->shared_mem_region->producer_position = 0;
}

template <typename T> bool Producer<T>::put(T item) {
  // SPDLOG_DEBUG("{} Producer/Consumer is {}/{}", this->mmap_info->name,
  //              this->shared_mem_region->producer_position,
  //              this->shared_mem_region->consumer_position);
  int next_producer_position = (this->shared_mem_region->producer_position+1) % this->slots;

  // Producer cannot produce spots that the consumer cannot read without looping around.
  // producer spot must be < consumer spot.
  if (next_producer_position == this->shared_mem_region->consumer_position) {
    SPDLOG_ERROR("{} Producer overflow!", this->mmap_info->name);
    return false;
  }

  this->shared_mem_region
      ->entities[this->shared_mem_region->producer_position] = item;

  this->shared_mem_region->producer_position++;
  this->shared_mem_region->producer_position %= this->slots;

  return true;
}

template <typename T> Consumer<T>::Consumer(int slots, const char *mmap_name) {
  this->slots = slots;
  this->mmap_info = open_mmap(mmap_name, this->get_mmap_size());
  this->shared_mem_region = (SharedData<T> *)this->mmap_info->location;
  this->shared_mem_region->consumer_position = 0;
}

template <typename T> Consumer<T>::~Consumer() throw() { cleanup(); }

template <typename T> void Consumer<T>::cleanup() {
  close_mmap(this->mmap_info);
}
