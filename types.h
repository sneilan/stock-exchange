#ifndef types_h
#define types_h

#define PRICE int

#define SIDE char
#define BUY 'b'
#define SELL 's'

#define SEQUENCE_ID unsigned long long
#define SUBMITTED 0
#define FILLED 1
#define CANCELLED 2
#define PARTIAL_FILL 3

#define ONE_DOLLAR 100
#define ONE_HUNDRED_DOLLARS (ONE_DOLLAR * 100)
#define ONE_CENT 1

struct Order {
    SEQUENCE_ID id;
    int quantity;
    int filled_quantity = 0;

    int unfilled_quantity() {
        return quantity - filled_quantity;
    }

    SIDE side;
    PRICE limitPrice;
    char clientId;
    int status = SUBMITTED;

    // created_at, updated_at etc.
};

#endif