#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <errno.h>
#include <stdio.h>
#include <string.h>

#ifndef NDEBUG
#define debug(M, ...)                                                          \
    fprintf(stderr, "DEBUG %s:%d: " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define debug(M, ...)
#endif

#define clean_errno() (errno == 0 ? "None" : strerror(errno))

#ifndef NDEBUG
#define log_error(M, ...)                                                      \
    fprintf(stderr, "[ERROR] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__,  \
            clean_errno(), ##__VA_ARGS__)
#else
#define log_error(M, ...)
#endif

#ifndef NDEBUG
#define log_info(M, ...)                                                       \
    fprintf(stderr, "[INFO] (%s:%d) " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define log_info(M, ...)
#endif

#define check(A, M, ...)                                                       \
    if (!(A)) {                                                                \
        log_error(M, ##__VA_ARGS__);                                           \
        errno = 0;                                                             \
        goto error;                                                            \
    }

#define sentinel(M, ...)                                                       \
    {                                                                          \
        log_error(M, ##__VA_ARGS__);                                           \
        errno = 0;                                                             \
        goto error;                                                            \
    }

#define check_mem(A) check((A), "Out of memory.")

#define check_debug(A, M, ...)                                                 \
    if (!(A)) {                                                                \
        debug(M, ##__VA_ARGS__);                                               \
        errno = 0;                                                             \
        goto error;                                                            \
    }

#endif