#ifndef order_book_h_
#define order_book_h_
#include "../eventstore/eventstore.h"
#include "../util/disruptor.h"
#include "../util/linked_list.h"
#include "../util/types.h"
#include "book.h"
#include "price_level.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <list>
#include <spdlog/spdlog.h>
#include <sstream>
#include <stdexcept>
#include <time.h>
#include <unordered_map>

class OrderBook {
  // Main entry point for matching orders
public:
  OrderBook(Producer<L1MarketData> *outbound_mkt_l1);

  std::list<Order *>
  newOrder(Order *order); // give a list of orders matched or none at all.
  void cancelOrder(SEQUENCE_ID id);
  int getVolume();
  PriceLevel *getBid();
  PriceLevel *getAsk();

private:
  void addOrder(Order *);
  std::list<Order *> fillOrder(Order *order);
  bool isOpposingOrderBookBlank(Order *order);
  int opposingOrderVolume(Order *order);
  int bookOrderVolume(Order *order);
  void adjustBidAskIfOrderIsBetterPrice(Order *order);
  bool orderCrossedSpread(Order *order);
  void setBidAskToReflectMarket();
  void printBestBidAsk(const char *prefix);
  void sendMarketData(char type, int val);

  Book *buyBook;
  Book *sellBook;
  PriceLevel *bestBid;
  PriceLevel *bestAsk;
  int totalVolume = 0;
  // Create an unordered map of sequence ids to iterators
  // Then we can later implement a custom allocator to manage these iterators
  // so that they don't blow up the heap.
  // In a v2 consider implementing my own linked list so we don't need pointers
  // to iterators.
  std::unordered_map<SEQUENCE_ID, Node<Order *> *> *orderMap;
  Producer<L1MarketData> *outbound_mkt_l1;
};

#endif
