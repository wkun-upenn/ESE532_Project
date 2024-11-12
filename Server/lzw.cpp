#include <cstdint>
#include <vector>
#include <cstring>

// Constants
#define CAPACITY 8192          // Size of the hash table
#define ASSOCIATE_MEM_STORE 64 // Size of the associative memory
#define SEED 524057            // Seed for the custom hash function

// Function prototype for LZW compression
void lzw_encode(const unsigned char* input_data, size_t input_size, std::vector<uint16_t>& output_codes);

// Hash function using a variant of MurmurHash3
static inline uint32_t murmur_32_scramble(uint32_t k) {
    k *= 0xcc9e2d51;
    k = (k << 15) | (k >> 17);
    k *= 0x1b873593;
    return k;
}

// Custom hash function
unsigned int my_hash(uint32_t key) {
    uint32_t h = SEED;
    h ^= murmur_32_scramble(key);
    h = (h << 13) | (h >> 19);
    h = h * 5 + 0xe6546b64;
    h ^= murmur_32_scramble(key);
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    return h % CAPACITY;
}

// Hash table entry structure
struct HashEntry {
    uint32_t key;   // 20-bit key
    uint16_t value; // 12-bit value
    bool valid;     // Validity flag
};

// Associative memory structure
struct AssocMem {
    uint32_t keys[ASSOCIATE_MEM_STORE];
    uint16_t values[ASSOCIATE_MEM_STORE];
    uint32_t fill; // Number of entries filled
};

// Hash table lookup
void hash_lookup(HashEntry* hash_table, uint32_t key, bool& hit, uint16_t& result) {
    uint32_t hash_val = my_hash(key);
    HashEntry& entry = hash_table[hash_val];
    if (entry.valid && entry.key == key) {
        hit = true;
        result = entry.value;
    } else {
        hit = false;
    }
}

// Hash table insert
void hash_insert(HashEntry* hash_table, uint32_t key, uint16_t value, bool& collision) {
    uint32_t hash_val = my_hash(key);
    HashEntry& entry = hash_table[hash_val];
    if (entry.valid) {
        collision = true;
    } else {
        entry.key = key;
        entry.value = value;
        entry.valid = true;
        collision = false;
    }
}

// Associative memory lookup
void assoc_lookup(AssocMem& mem, uint32_t key, bool& hit, uint16_t& result) {
    for (uint32_t i = 0; i < mem.fill; ++i) {
        if (mem.keys[i] == key) {
            hit = true;
            result = mem.values[i];
            return;
        }
    }
    hit = false;
}

// Associative memory insert
void assoc_insert(AssocMem& mem, uint32_t key, uint16_t value, bool& collision) {
    if (mem.fill < ASSOCIATE_MEM_STORE) {
        mem.keys[mem.fill] = key;
        mem.values[mem.fill] = value;
        mem.fill++;
        collision = false;
    } else {
        collision = true;
    }
}

// Insert function combining hash table and associative memory
void insert(HashEntry* hash_table, AssocMem& mem, uint32_t key, uint16_t value, bool& collision) {
    hash_insert(hash_table, key, value, collision);
    if (collision) {
        assoc_insert(mem, key, value, collision);
    }
}

// Lookup function combining hash table and associative memory
void lookup(HashEntry* hash_table, AssocMem& mem, uint32_t key, bool& hit, uint16_t& result) {
    hash_lookup(hash_table, key, hit, result);
    if (!hit) {
        assoc_lookup(mem, key, hit, result);
    }
}

// LZW compression function
void lzw_encode(const unsigned char* input_data, size_t input_size, std::vector<uint16_t>& output_codes) {
    if (input_size == 0) {
        // Handle empty input
        return;
    }

    // Initialize hash table and associative memory
    HashEntry hash_table[CAPACITY];
    memset(hash_table, 0, sizeof(hash_table));
    AssocMem assoc_mem;
    assoc_mem.fill = 0;

    // Initialize the dictionary with single-character strings
    uint16_t next_code = 256;
    for (uint16_t i = 0; i < 256; ++i) {
        uint32_t key = (i << 8) | 0; // Prefix is 0
        bool collision = false;
        insert(hash_table, assoc_mem, key, i, collision);
    }

    uint16_t prefix_code = input_data[0];
    size_t i = 0;

    while (i < input_size - 1) {
        uint8_t next_char = input_data[i + 1];
        uint32_t key = (prefix_code << 8) | next_char; // Combine prefix and next character
        bool hit = false;
        uint16_t code = 0;

        lookup(hash_table, assoc_mem, key, hit, code);

        if (hit) {
            prefix_code = code;
        } else {
            // Output the code for the prefix
            output_codes.push_back(prefix_code);

            // Add new sequence to the dictionary
            bool collision = false;
            insert(hash_table, assoc_mem, key, next_code, collision);
            if (!collision) {
                next_code++;
            }
            prefix_code = next_char;
        }
        i++;
    }
    // Output the code for the last prefix
    output_codes.push_back(prefix_code);
}
