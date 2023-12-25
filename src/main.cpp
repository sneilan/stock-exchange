#include <fcntl.h>
#include <sodium.h>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <spdlog/version.h>
#include <sys/mman.h>
#include <unistd.h>

#include "eventstore/eventstore.h"
#include "gateway/gateway.h"
#include "gateway/market_data.h"
#include "gateway/socket.h"
#include "order_book/order_book.h"
#include "util/types.h"

int main() {
  spdlog::set_level(spdlog::level::debug);
  // https://github.com/gabime/spdlog/wiki/3.-Custom-formatting
  spdlog::set_pattern("%-5l %E %-16s%-4#%-21! %v");

  if (sodium_init() == -1) {
    // Initialization failed
    SPDLOG_CRITICAL("Could not initialize libsodium for user auth! ❌");
    return -1;
  }
  SPDLOG_INFO("libsodium initialized.");

  const char *outgoing_message_buf = "/ss_outgoing_messages";

  Producer<ORDER_MMAP_OFFSET> outboundMessage(MAX_OUTGOING_MESSAGES,
                                              outgoing_message_buf);

  SPDLOG_INFO("Allocating EventStore mmap pool..");
  const char *eventstore_buf = "/eventstore_buf";
  MMapObjectPool<Order> *order_pool =
      new MMapObjectPool<Order>(MAX_OPEN_ORDERS, eventstore_buf, IS_CONTROLLER);
  SPDLOG_INFO("Allocated EventStore mmap pool!");

  const char *incoming_msg_buf = "/gateway_ring_buf";
  Producer<NewOrderEvent> *producer =
      new Producer<NewOrderEvent>(GATEWAY_BUFLEN, incoming_msg_buf);

  Consumer<ORDER_MMAP_OFFSET> *outgoing_message_consumer =
      new Consumer<ORDER_MMAP_OFFSET>(MAX_OUTGOING_MESSAGES,
                                      outgoing_message_buf,
                                      OUTGOING_MESSAGE_CONSUMER);

  order_pool =
      new MMapObjectPool<Order>(MAX_OPEN_ORDERS, eventstore_buf, IS_CLIENT);

  const char *outbound_market_data_buf = "/l1_market_data";
  Producer<L1MarketData> *producer_l1_market_data = new Producer<L1MarketData>(
      MAX_MARKET_DATA_UPDATES, outbound_market_data_buf);

  Consumer<L1MarketData> *consumer_l1_market_data = new Consumer<L1MarketData>(
      MAX_MARKET_DATA_UPDATES, outbound_market_data_buf,
      OUTGOING_MESSAGE_CONSUMER);

  Gateway *gateway =
      new Gateway(producer, outgoing_message_consumer, order_pool);
  MarketData *market_data = new MarketData(consumer_l1_market_data);

  SPDLOG_INFO("Exchange starting");

  pid_t c_pid = fork();

  if (c_pid == -1) {
    SPDLOG_CRITICAL("fork");
    exit(EXIT_FAILURE);
  }

  if (c_pid > 0) {
    // Parent
    // Listens to new orders from clients and puts them into the mmap ring
    // buffer maintained by gateway.
    SPDLOG_INFO("Gateway starting");
    gateway->run();
  } else {
    pid_t c_pid2 = fork();

    if (c_pid2 > 0) {
      SPDLOG_INFO("MarketData starting");
      market_data->run();
    } else {
      // Child
      SPDLOG_INFO("Order engine starting");
      EventStore *eventStore = new EventStore(order_pool);
      SPDLOG_INFO("Created EventStore");

      OrderBook *orderBook = new OrderBook(producer_l1_market_data);
      SPDLOG_INFO("Created OrderBook");

      Consumer<NewOrderEvent> *incoming_order_consumer =
          new Consumer<NewOrderEvent>(GATEWAY_BUFLEN, incoming_msg_buf,
                                      GATEWAY_CONSUMER);
      SPDLOG_INFO("Created consumer for incoming orders.");

      while (1) {
        // Constantly checking for new orders in the gateway ring buffer.
        NewOrderEvent *item = incoming_order_consumer->get();

        if (item == nullptr) {
          continue;
        }

        SPDLOG_DEBUG("Order get for client {} for price {} for "
                     "side {} quantity {}",
                     item->clientId, item->limitPrice, item->side,
                     item->quantity);

        // Store the event in the event store
        SEQUENCE_ID id = eventStore->newEvent(item->side, item->limitPrice,
                                              item->clientId, item->quantity);
        SPDLOG_INFO("Sequence ID is now {} & size is now {}", id,
                    eventStore->size());

        // Get response here & spool information to new ring buffer
        Order *order = eventStore->get(id);
        ORDER_MMAP_OFFSET offset = eventStore->getOffset(id);
        SPDLOG_INFO("Grabbed order {}", order->id);
        std::list<Order *> updated_orders = orderBook->newOrder(order);

        outboundMessage.put(offset);
        // State of order is based on how many fills.
        SPDLOG_DEBUG("Order {} recieved message sent", order->id);

        SPDLOG_INFO("Order book volume is now {}", orderBook->getVolume());
        SPDLOG_INFO("Orders updated are size {}", updated_orders.size());

        for (Order *order : updated_orders) {
          // @TODO Stop using socket ids as client ids. Set up a map
          // between client ids and sockets. Also create a buffer to try
          // to send orders to clients that have disconnected.
          outboundMessage.put(order_pool->pointer_to_offset(order));
          SPDLOG_DEBUG("Order {} updated message sent", order->id);
        }
      }
    }
  }

  // @TODO create signal handler to clean up

  return 0;
}
