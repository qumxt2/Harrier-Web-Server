// assert_app.h

// Copyright 2012
// Graco Inc., Minneapolis, MN
// All Rights Reserved

#ifndef ASSERT_APP_H
#define ASSERT_APP_H

#include "debug.h"

#undef assert

#ifdef NDEBUG

#define assert(ignore)      ((void)0)
#define assert_always()     ((void)0)

#else //!NDEBUG

#ifdef NODEBUG

#define assert(ignore)      ((void)0)
#define assert_always()     ((void)0)

#else //!NODEBUG

#define assert(expression) \
    if (!(expression)) \
        assert_always()

 #if defined(__DEBUG)
 #define DEBUG_HALT() __asm__ volatile (" sdbbp 0")
 #else
 #define DEBUG_HALT() (void)0
 #endif

#define assert_always() \
    do \
    { \
        DEBUG_PRINT_STRING(DBUG_ALWAYS, "\nAssert failed at `"); \
        DEBUG_PRINT_STRING(DBUG_ALWAYS, __FILE__); \
        DEBUG_PRINT_STRING(DBUG_ALWAYS, "':"); \
        DEBUG_PRINT_UNSIGNED_DECIMAL(DBUG_ALWAYS, __LINE__); \
        DEBUG_PRINT_STRING(DBUG_ALWAYS, "\n"); \
        DEBUG_HALT(); \
    } while(FALSE)

#endif //NODEBUG 
 
#endif //NDEBUG

#endif //ASSERT_APP_H

