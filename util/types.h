#ifndef types_h
#define types_h

#include <cstdint>
#define PRICE int

#define SIDE char
#define BUY 'b'
#define SELL 's'

#define SEQUENCE_ID unsigned long long
#define ORDER_MMAP_OFFSET int
#define SUBMITTED 0
#define FILLED 1
#define CANCELLED 2
#define PARTIAL_FILL 3

#define ONE_DOLLAR 100
#define ONE_HUNDRED_DOLLARS (ONE_DOLLAR * 100)
#define ONE_CENT 1

struct Order {
    char clientId;
    int quantity;
    SIDE side;
    PRICE limitPrice;

    SEQUENCE_ID id;
    int filled_quantity = 0;

    int unfilled_quantity() {
        return quantity - filled_quantity;
    }

    int status = SUBMITTED;

    int64_t created_at;
    // int64_t updated_at;
};

#endif
