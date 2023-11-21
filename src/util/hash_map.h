// https://craftinginterpreters.com/hash-tables.html
// Robin Hood hashing
// cuckoo hashing
// double hashing

// Linear Probing
// http://www.isthe.com/chongo/tech/comp/fnv/

// UNUSED. Still using stl library but will probably use soon.

#ifndef hashtable_h
#define hashtable_h

#include <cstdint>
#include <stdlib.h>
#include <string.h>

#define TABLE_MAX_LOAD 0.75

typedef struct {
  int key;
  int value;
} Entry;

typedef struct {
  int count;
  int capacity;
  Entry *entries;
} Table;

void tableAddAll(Table *from, Table *to);
bool tableSet(Table *table, int key, int value);
bool tableGet(Table *table, int key, int value);
void initTable(Table *table);
void freeTable(Table *table);

void initTable(Table *table) {
  table->count = 0;
  table->capacity = 0;
  table->entries = 0;
}

void freeTable(Table *table) {
  FREE_ARRAY(Entry, table->entries, table->capacity);
  initTable(table);
}

// FNV-1a
static uint32_t hashInt(int key) {
  uint32_t hash = 2166136261u;

  for (int i = 0; i < sizeof(int); i++) {
    hash ^= (&key)[i];
    hash *= 16777619;
  }

  return hash;
}

static Entry *findEntry(Entry *entries, int capacity, int key) {
  uint32_t index = hashInt(key) % capacity;
  for (;;) {
    Entry *entry = &entries[index];

    if (entry->key == key || entry->key == NULL) {
      return entry;
    }

    index = (index + 1) % capacity;
  }
}

void tableAddAll(Table *from, Table *to) {
  for (int i = 0; i < from->capacity; i++) {
    Entry *entry = &from->entries[i];
    if (entry->key != NULL) {
      tableSet(to, entry->key, entry->value);
    }
  }
}

static void adjustCapacity(Table *table, int capacity) {
  Entry *entries = ALLOCATE(Entry, capacity);
  for (int i = 0; i < capacity; i++) {
    entries[i].key = NULL;
    entries[i].value = NIL_VAL;
  }

  for (int i = 0; i < table->capacity; i++) {
    Entry *entry = &table->entries[i];
    if (entry->key == NULL)
      continue;

    Entry *dest = findEntry(entries, capacity, entry->key);
    dest->key = entry->key;
    dest->value = dest->value;
  }

  FREE_ARRAY(Entry, table->entries, table->capacity);
  table->entries = entries;
  table->capacity = capacity;
}

bool tableSet(Table *table, int key, int value) {
  if (table->count + 1 > table->capacity * TABLE_MAX_LOAD) {
    int capacity = GROW_CAPACITY(table->capacity);
    adjustCapacity(table, capacity);
  }

  Entry *entry = findEntry(table->entries, table->capacity, key);
  bool isNewKey = entry->key == NULL;
  if (isNewKey)
    table->count++;

  entry->key = key;
  entry->value = value;

  return isNewKey;
}

#endif
