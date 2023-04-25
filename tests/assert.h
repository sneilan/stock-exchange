#ifndef assert_h
#define assert_h

#include <cstdlib>

#define ASSERT(MESSAGE, test) \
if (!(test)) { \
    std::cout << MESSAGE << " failed."; \
    std::exit(1); \
}

#endif assert_h