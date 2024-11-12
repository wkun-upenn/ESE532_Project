// common.h

#ifndef COMMON_H
#define COMMON_H

#include <cstdlib>
#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <vector>

#include "cdc.h"
#include "sha256.h"
#include "lzw.h"
#include "deduplication.h"

// Constants
#define NUM_ELEMENTS 16384
#define HEADER 2
#define MAX_CHUNK_SIZE 8192
#define MIN_CHUNK_SIZE 8
#define TARGET_CHUNK_SIZE 4096
#define CHUNK_SIZE 4096
#define PIPELINE_BUFFER_LEN 16384
#define MAX_OUTPUT_BUF_SIZE 40960
#define MAX_LZW_CHUNKS 20
#define CODE_LENGTH 12
#define MODULUS_MASK (MODULUS - 1)

// Macro for checking memory allocation
#define CHECK_MALLOC(ptr, msg) \
    if (ptr == NULL) { \
        printf("%s\n", msg); \
        exit(EXIT_FAILURE); \
    }

// Function prototypes

// CDC (Content-Defined Chunking)
void content_defined_chunking(unsigned char* buffer, size_t buffer_size, std::vector<size_t>& chunk_positions);

// Deduplication function prototype
bool dedup(std::unordered_set<std::string>& hash_table, const std::string& hash_value);

// LZW Compression
void lzw_encode(const unsigned char* input_data, size_t input_size, std::vector<uint16_t>& output_codes);


std::string sha256_hash_string(const BYTE data[], size_t length);
#endif // COMMON_H
