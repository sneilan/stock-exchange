#ifndef assert_h
#define assert_h

void assert(const char * message, bool val) {
    if (!val) {
        throw(message);
    }
}

#endif assert_h