#ifndef CONTRACTS_H_
#define CONTRACTS_H_

#include <cassert>

#include "settings.hpp"

#ifdef DEBUG

#define REQUIRES(x) assert(x)
#define ENSURES(x) assert(x)
#define ASSERT(x) assert(x)

#else

#define REQUIRES(x)
#define ENSURES(x)
#define ASSERT(x)

#endif

#endif
