#ifndef types_h
#define types_h

#define PRICE int

#define SIDE char
#define BUY 'b'
#define SELL 's'

#define SEQUENCE_ID unsigned long long

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
    // status, created_at, updated_at etc.
};

#endif