// sha256.h

#ifndef SHA256_H
#define SHA256_H

#include <stdint.h>
#include <stddef.h>
#include <string>
// SHA-256 block size
#define SHA256_BLOCK_SIZE 32 // SHA-256 outputs a 32-byte digest

// Type definitions
typedef unsigned char BYTE; // 8-bit byte
typedef uint32_t WORD;      // 32-bit word

// SHA-256 context structure
typedef struct {
    BYTE data[64];
    WORD datalen;
    uint64_t bitlen;
    WORD state[8];
} SHA256_CTX;

// Function prototypes

void sha256_init(SHA256_CTX *ctx);
void sha256_update(SHA256_CTX *ctx, const BYTE data[], size_t len);
void sha256_final(SHA256_CTX *ctx, BYTE hash[]);

// Helper functions
void sha256_hash(const BYTE data[], size_t length, BYTE hash_output[]);
void sha256_hash_range(const BYTE data[], uint32_t start_idx, uint32_t end_idx, BYTE hash_output[]);
std::string sha256_hash_string(const BYTE data[], size_t length);
std::string bytes_to_hex_string(const BYTE* bytes, size_t length);

#endif // SHA256_H
