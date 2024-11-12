#ifndef LZW_H
#define LZW_H

#include <cstdint>  // For uint32_t, uint8_t, etc.


#define CAPACITY 8192          // Size of the hash table
#define ASSOCIATE_MEM_STORE 64 // Size of the associative memory
#define SEED 524057            // Seed for the custom hash function

// Function prototype for LZW compression
void lzw_encode(const unsigned char* input_data, size_t input_size, std::vector<uint16_t>& output_codes);

#endif