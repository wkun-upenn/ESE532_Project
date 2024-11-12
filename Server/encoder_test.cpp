// encoder_test.cpp

#include "common.h" // Ensure this header is available and compatible
#include <fstream>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <string>
#include <unistd.h>    // Re-included for getopt
#include "stopwatch.h" // Ensure this header is available and compatible

// Constants
#define NUM_PACKETS 8
#define pipe_depth 4
#define DONE_BIT_L (1 << 7)
#define DONE_BIT_H (1 << 15)

#define NUM_ELEMENTS 16384
#define BLOCKSIZE_ENC 8192 //2048
#define HEADER 2

// Global Variables
int offset = 0;
unsigned char* file_buffer;

// Utility functions remain unchanged
void append_uint32_le(std::vector<uint8_t>& buffer, uint32_t value) {
    buffer.push_back(static_cast<uint8_t>(value & 0xFF));
    buffer.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
    buffer.push_back(static_cast<uint8_t>((value >> 16) & 0xFF));
    buffer.push_back(static_cast<uint8_t>((value >> 24) & 0xFF));
}

void append_uint16_le(std::vector<uint8_t>& buffer, uint16_t value) {
    buffer.push_back(static_cast<uint8_t>(value & 0xFF));
    buffer.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
}

void save_to_text_file(const unsigned char* data, size_t data_size, const std::string& output_filename) {
    std::ofstream output_file(output_filename, std::ios::binary);
    if (!output_file) {
        std::cerr << "Error: Could not open file " << output_filename << " for writing." << std::endl;
        return;
    }

    output_file.write(reinterpret_cast<const char*>(data), data_size);
    output_file.close();

    if (output_file) {
        std::cout << "Successfully wrote " << data_size << " bytes to " << output_filename << std::endl;
    } else {
        std::cerr << "Error: Failed to write data to " << output_filename << std::endl;
    }
}

void handle_input(int argc, char* argv[], int* blocksize) {
    int option;
    while ((option = getopt(argc, argv, ":b:")) != -1) {
        switch (option) {
            case 'b':
                *blocksize = atoi(optarg);
                printf("Blocksize is set to %d\n", *blocksize);
                break;
            case ':':
                fprintf(stderr, "Option -%c requires a parameter\n", optopt);
                break;
            default:
                fprintf(stderr, "Usage: %s [-b blocksize]\n", argv[0]);
        }
    }
}

bool process_packet(unsigned char* file, int& offset, 
                    std::unordered_map<std::string, std::vector<unsigned char> > & lzw_cache,
                    std::unordered_set<std::string>& hash_table) {
    // Initial size before processing
    size_t original_size = offset;

    // Step 1: Perform Content-Defined Chunking
    std::vector<std::string> chunks;
    cdc(file, chunks, static_cast<unsigned int>(offset)); // Corrected call
    size_t chunk_count = chunks.size();
    std::cout << "CDC function called: chunk_count = " << chunk_count << std::endl;

    // Step 2: SHA-256 -> Deduplication or LZW for each chunk
    uint64_t dedup_size = 0;
    uint64_t lzw_compressed_size = 0;
    uint64_t lzw_before = 0;
    uint64_t dedup_before = 0;
    for (size_t i = 0; i < chunk_count; i++) {
        const std::string& chunk = chunks[i];
        uint32_t chunk_size = chunk.size();
        const unsigned char* chunk_data = reinterpret_cast<const unsigned char*>(chunk.c_str());

        // Compute SHA-256 hash of the chunk
        std::string hash_value = sha256_hash_string(chunk_data, chunk_size); // Corrected call

        if (dedup(hash_table, hash_value)) { // Corrected call
            dedup_before += chunk_size;
            // Hash found in table, reuse cached LZW data
            const std::vector<unsigned char>& cached_data = lzw_cache[hash_value];
            uint32_t cached_size = cached_data.size();
            dedup_size += cached_size;  // Add size of reused chunk to dedup size

            // Ensure buffer does not overflow
            if (offset + cached_size < 70000000) {
                memcpy(&file[offset], cached_data.data(), cached_size);
                offset += cached_size;
            } else {
                std::cerr << "File buffer overflow!" << std::endl;
                return false;
            }
        } else {
            // New chunk, perform LZW compression
            std::vector<uint16_t> output_codes;
            lzw_encode(chunk_data, chunk_size, output_codes); // Corrected call

            // Serialize the uint16_t codes to uint8_t bytes
            std::vector<unsigned char> compressed_data;
            compressed_data.reserve(output_codes.size() * 2); // Reserve space to optimize

            for (const uint16_t& code : output_codes) {
                compressed_data.push_back((code >> 8) & 0xFF); // High byte
                compressed_data.push_back(code & 0xFF);        // Low byte
            }

            uint32_t compressed_size = compressed_data.size();  
            std::cout << "Compressed chunk " << i + 1 << " with size = " << compressed_size << " bytes \n";
            lzw_compressed_size += compressed_size;  // Track the LZW compressed size   
            lzw_cache[hash_value] = compressed_data;
        }
    }

    std::cout << "Compression ratio (compared to original file size): " 
              << (lzw_before / static_cast<float>(original_size)) << std::endl;

    return true;
}



int main(int argc, char* argv[]) {
    const char* output_filename = "encoder_test"; // Fixed output file name

    // Handle command-line input for blocksize
    int blocksize = BLOCKSIZE_ENC;
    handle_input(argc, argv, &blocksize);

    // Read LittlePrince.txt into file_buffer
    std::ifstream input_file("../LittlePrince.txt", std::ios::binary | std::ios::ate);
    if (!input_file) {
        std::cerr << "Error: Could not open LittlePrince.txt for reading." << std::endl;
        return 1;
    }

    std::streamsize file_size = input_file.tellg();
    input_file.seekg(0, std::ios::beg);

    // Allocate memory for the file buffer
    file_buffer = (unsigned char*) malloc(sizeof(unsigned char) * file_size);
    if (file_buffer == NULL) {
        std::cerr << "Failed to allocate memory for file buffer.\n";
        return 1;
    }

    if (!input_file.read(reinterpret_cast<char*>(file_buffer), file_size)) {
        std::cerr << "Error reading file LittlePrince.txt." << std::endl;
        free(file_buffer);
        return 1;
    }

    input_file.close();
    offset = file_size;

    // Optionally save the raw input data to a text file
    save_to_text_file(file_buffer, offset, "received_output.txt");

    // Initialize caches for deduplication and compression
    std::unordered_map<std::string, std::vector<unsigned char> > lzw_cache;
    std::unordered_set<std::string> hash_table;

    // Process the file buffer
    if (!process_packet(file_buffer, offset, lzw_cache, hash_table)) {
        std::cerr << "Error processing packet!" << std::endl;
    }

    // Save the processed data to a binary file
    FILE *outfd = fopen(output_filename, "wb");
    if (outfd == NULL) {
        std::cerr << "Error: Could not open " << output_filename << " for writing." << std::endl;
        free(file_buffer);
        return 1;
    }

    size_t bytes_written = fwrite(file_buffer, 1, offset, outfd);
    if (bytes_written != offset) {
        std::cerr << "Warning: Not all data was written to the file. Expected " 
                  << offset << " bytes, but wrote " << bytes_written << " bytes." << std::endl;
    } else {
        std::cout << "All data written successfully." << std::endl;
    }
    printf("Written file with %zu bytes\n", bytes_written);
    fclose(outfd);

    // Free allocated memory
    free(file_buffer);

    std::cout << "Processing completed successfully." << std::endl;

    return 0;
}
