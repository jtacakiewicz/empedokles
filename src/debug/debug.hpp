#ifndef EMP_DEBUG_H
#define EMP_DEBUG_H
#include "log.hpp"
namespace emp {
#define EMP_DEBUG 1
#if EMP_DEBUG == 1

#define EMP_ASSERT(x)    (assert(x))
#define EMP_DEBUGCALL(x) x

#else

#define EMP_ASSERT(x)    ((void)sizeof(x))
#define EMP_DEBUGCALL(x) ;

#endif

}
#endif  //  EMP_DEBUG_H
