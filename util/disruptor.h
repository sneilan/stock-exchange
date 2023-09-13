// Implementation of ring buffer for processes to communicate with each other
// See https://martinfowler.com/articles/lmax.html

#include <spdlog/spdlog.h>
#include "mmap_wrapper.h"

template <typename T>
struct SharedData {
  int producer_position;
  int consumer_position;
  T* entities;
};

template <typename T> class Disruptor {
protected:
  MMap_Info* mmap_info;
  SharedData<T>* shared_mem_region;
  int get_mmap_size();
  int slots;
};

template <typename T> class Producer : public Disruptor<T> {
public:
  Producer(int slots, const char * mmap_name);
  ~Producer() throw();
  void cleanup();
  // This creates a copy on function call for simplicity.
  // Later on use pointers to improve performance.
  void put(T item);
};

template <typename T> class Consumer: public Disruptor<T> {
public: 
  // producer / consumer behavior is totally separate but
  // we have to specify slots twice.
  // For now this is fine because producer and consumer are
  // created in separate processes anyway.
  Consumer(int slots, const char * mmap_name);
  ~Consumer() throw();
  T* get();
  void cleanup();
};

template <typename T>
T* Consumer<T>::get() {
  SPDLOG_DEBUG("{} Producer/Consumer is {}/{}", this->mmap_info->name, this->shared_mem_region->producer_position, this->shared_mem_region->consumer_position);

  // Consumer should never go past producer position. Producer should always be one spot ahead.
  if (this->shared_mem_region->consumer_position > this->shared_mem_region->producer_position) {
    return nullptr;
  }

  T* item = &this->shared_mem_region->entities[this->shared_mem_region->consumer_position];
  this->shared_mem_region->consumer_position++;
  this->shared_mem_region->consumer_position %= this->slots;

  return item;
}

template <typename T>
int Disruptor<T>::get_mmap_size() {
  // Extra two slots is to store consumer / producer positions.
  return sizeof(SharedData<T>) + sizeof(T) * slots;
}

template <typename T>
void Producer<T>::cleanup() { delete_mmap(this->mmap_info); }

template <typename T>
Producer<T>::~Producer() throw() { cleanup(); }

template <typename T>
Producer<T>::Producer(int slots, const char * mmap_name) {
  this->slots = slots;
  this->mmap_info = init_mmap(mmap_name, this->get_mmap_size());
  this->shared_mem_region = (SharedData<T> *)this->mmap_info->location;

  this->shared_mem_region->entities = reinterpret_cast<T*>((char*)this->shared_mem_region + sizeof(SharedData<T>));

  this->shared_mem_region->producer_position = 0;
}

template <typename T>
void Producer<T>::put(T item) {
  SPDLOG_DEBUG("{} Producer/Consumer is {}/{}", this->mmap_info->name, this->shared_mem_region->producer_position, this->shared_mem_region->consumer_position);
  // @TODO we can optionally put a check to make sure producer cannot put more entities into queue here.

  this->shared_mem_region->entities[this->shared_mem_region->producer_position] = item;

  this->shared_mem_region->producer_position++;
  this->shared_mem_region->producer_position %= this->slots;
}

template <typename T>
Consumer<T>::Consumer(int slots, const char * mmap_name) {
  this->slots = slots;
  this->mmap_info = open_mmap(mmap_name, this->get_mmap_size());
  this->shared_mem_region = (SharedData<T> *)this->mmap_info->location;
  this->shared_mem_region->consumer_position = 0;
}

template <typename T>
Consumer<T>::~Consumer() throw() { cleanup(); }

template <typename T>
void Consumer<T>::cleanup() { close_mmap(this->mmap_info); }
