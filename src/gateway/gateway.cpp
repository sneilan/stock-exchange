#include "gateway.h"

Gateway::Gateway(Producer<NewOrderEvent>* incoming_msg_producer,
                 Consumer<ORDER_MMAP_OFFSET>* outgoing_message_consumer,
                 MMapObjectPool<Order> *order_pool) {
  this->incoming_msg_producer = incoming_msg_producer;
  this->outgoing_message_consumer = outgoing_message_consumer;
  this->order_pool = order_pool;

  // Default to SSL.
 
  ctx = SSL_CTX_new(SSLv23_server_method());

  if (!ctx) {
    SPDLOG_CRITICAL("Could not start SSL_CTX_new");
    exit(1);
  }

  if (SSL_CTX_use_certificate_file(ctx, getenv("PUBLIC_KEY"), SSL_FILETYPE_PEM) <= 0) {
    SPDLOG_CRITICAL("Could not load public key. Check PUBLIC_KEY env variable.");
    exit(1);
  }

  if (SSL_CTX_use_PrivateKey_file(ctx, getenv("PRIVATE_KEY"), SSL_FILETYPE_PEM) <= 0) {
    SPDLOG_CRITICAL("Could not load private key. Check PRIVATE_KEY env variable.");
    exit(1);
  }

  // Initialize ssl connection pool
  SSL* ssl;
  for (int i = 0; i < MAX_CLIENTS; i++) {
    ssl = SSL_new(ctx);
    ssl_pool.push_back(ssl); // take a swim.
  }
}

SSL* Gateway::getNewSSLObj() {
  if (ssl_pool.empty()) {
    // Can also create a new ssl object.
    // return SSL_new(SSL_CTX_new(SSLv23_server_method()));
    return nullptr;
  }

  SSL* ssl = ssl_pool.back();
  ssl_pool.pop_back();
  SSL_clear(ssl);
  return ssl;
}

Gateway::~Gateway() throw() {
  for (SSL* ssl : ssl_pool) {
    SSL_free(ssl);
  }

  SSL_CTX_free(ctx);
}

void Gateway::handleOutgoingMessage() {
  ORDER_MMAP_OFFSET *offset = outgoing_message_consumer->get();
  if (offset != nullptr) {
    Order *order = order_pool->offset_to_pointer(*offset);

    // message type (char)
    // sequence ID (unsigned long long)
    // total quantity (integer)
    // filled quantity (integer)
    // (TODO)
    // last fill price (integer)
    // last quantity filled (integer)

    // hacked out for prototype purposes. Should replace with FIX or protobuf.
    // Maybe even Apache Avro.
    int total_size = sizeof(char) + sizeof(order->id) +
      sizeof(order->quantity) +
      sizeof(order->filled_quantity) + sizeof(order->clientId);
    char buffer[total_size];

    char orderRecieved = 'r';
    char orderUpdated = 'u';
    char orderFilled = 'f';
    // @TODO cancelled later.

    if (order->unfilled_quantity() == order->quantity) {
      buffer[0] = orderRecieved;
    } else if (order->unfilled_quantity() == 0) {
      buffer[0] = orderFilled;
    } else {
      buffer[0] = orderUpdated;
    }

    int offset = 1;
    std::memcpy(buffer + offset, &order->id, sizeof(order->id));
    offset += sizeof(order->id);
    std::memcpy(buffer + offset, &order->quantity, sizeof(order->quantity));
    offset += sizeof(order->quantity);
    std::memcpy(buffer + offset, &order->filled_quantity,
                sizeof(order->filled_quantity));
    offset += sizeof(order->clientId);
    std::memcpy(buffer + offset, &order->clientId, sizeof(order->clientId));

    sendMessage(order->clientId, buffer, total_size);
    SPDLOG_DEBUG("Sent {} message {} about order {}", order->clientId,
                 buffer[0], order->id);
  }
}

void Gateway::readMessage(int client_id, char *message) {
  SPDLOG_INFO("Read message from {}", client_id);

  NewOrderEvent item;

  // Not building an authentication system yet
  // so just sending trades back to clients by socket id.
  item.clientId = client_id;
  item.limitPrice = ((NewOrderEvent *)message)->limitPrice;
  item.side = ((NewOrderEvent *)message)->side;
  item.quantity = ((NewOrderEvent *)message)->quantity;

  incoming_msg_producer->put(item);

  SPDLOG_INFO("Ring buffer Order recieved from client {} for price {} for "
              "side {} quantity {}",
              item.clientId, item.limitPrice, item.side, item.quantity);
}

void Gateway::newClient(int client_id) {
  SPDLOG_INFO("New client {}", client_id);

  SSL* ssl = this->getNewSSLObj();

  SSL_set_fd(ssl, client_socket[client_id]);

  // Perform SSL handshake
  if (SSL_accept(ssl) != 1) {
    SPDLOG_INFO("Client {} did not accept ssl", client_id);
    forceDisconnect(client_id);
    ssl_pool.push_back(ssl);
    return;
  }

  const char *msg = "Welcome new client";

  // this is bad because now I have to munge all my classes together.
  // Fine! SSL will be on the socket level.
  SSL_write(ssl, buffer, strlen(buffer));

        while ((bytes = SSL_read(ssl, buffer, sizeof(buffer))) > 0) {
            buffer[bytes] = '\0';
            printf("Received: %s\n", buffer);

            // Echo the received data back to the client
        }

  if (!sendMessage(client_id, const_cast<char *>(msg), strlen(msg))) {
    // @TODO perhaps sendMessage can handle what happens if a client disconnects
    // Then call our disconnected handler and let us know so we don't have to do
    // an error handling pattern everywhere.
    forceDisconnect(client_id);
  }
}

void Gateway::disconnected(int client_id) {
  SPDLOG_INFO("Client disconnected {}", client_id);
  // Later return the client's ssl to the pool.
}

void Gateway::run() {
  /*
  Socket server should have the following events
   1) New connection
   2) Disconnect
   3) Reading data from client
  Expose the following methods
   1) Sending data to a client
   2) Getting a list of open clients.
   3) Force closing a client connection
  Something else can handle parsing data from clients.
  */

  char * port = getenv("GATEWAY_PORT");
  bindSocket(atoi(port));
  listenToSocket();
}
