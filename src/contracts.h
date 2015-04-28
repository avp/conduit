#ifndef CONTRACTS_H_

#include <cassert>

#ifndef NO_DEBUG

#define REQUIRES(x) assert(x)
#define ENSURES(x) assert(x)
#define ASSERT(x) assert(x)

#else

#define REQUIRES(x)
#define ENSURES(x)
#define ASSERT(x)

#endif

#endif
