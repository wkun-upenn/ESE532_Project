// utilities.h

#ifndef UTILITIES_H
#define UTILITIES_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <errno.h>
#include <sys/mman.h>

// Constants
#define NUM_PACKETS 8
#define PIPE_DEPTH 4
#define DONE_BIT_L (1 << 7)
#define DONE_BIT_H (1 << 15)

#define MAX_CHUNK_SIZE 8192 // 8 KB
#define MIN_CHUNK_SIZE 1024 // 1 KB
#define ROLLING_WINDOW 48

// Assuming BLOCKSIZE, NUM_ELEMENTS, and HEADER are defined elsewhere
#ifndef BLOCKSIZE
#define BLOCKSIZE 2048
#endif

#ifndef NUM_ELEMENTS
#define NUM_ELEMENTS 2048
#endif

#ifndef HEADER
#define HEADER 2
#endif

#endif // UTILITIES_H
