#ifndef DEBUG_H
#define DEBUG_H

#if defined(_DEBUG)
#define DBGLOG(FMT, ...) { printf("Line %d: ", __LINE__); printf(FMT "\n", ##__VA_ARGS__ ); }
#else
#define DBGLOG(FMT, ...)
#endif

#endif //DEBUG_H