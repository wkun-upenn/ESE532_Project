#include "encoder.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "server.h"
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <errno.h>
#include <sys/mman.h>
#include "stopwatch.h"
#include <map>
#include <vector>

// Constants and placeholders
#define NUM_PACKETS 8
#define PIPE_DEPTH 4
#define DONE_BIT_L (1 << 7)
#define DONE_BIT_H (1 << 15)

#define MAX_CHUNK_SIZE 1024 // Placeholder max chunk size
#define MIN_CHUNK_SIZE 512  // Placeholder min chunk size

int offset = 0;
unsigned char* file;

// Placeholder hash function: addition modulo 2^32
uint32_t placeholder_hash(unsigned char* data, size_t length) {
    uint32_t hash = 0;
    for (size_t i = 0; i < length; i++) {
        hash += data[i];
    }
    return hash;
}

// Placeholder CDC function
size_t content_defined_chunking(unsigned char* data, size_t data_length, size_t chunk_start, size_t& chunk_length) {
    // Placeholder: Fixed-size chunks for simplicity
    chunk_length = std::min((size_t)MAX_CHUNK_SIZE, data_length - chunk_start);
    return chunk_length;
}

// Placeholder LZW encoding (no actual compression)
void lzw_encode(unsigned char* input_data, size_t input_size, unsigned char*& output_data, size_t& output_size) {
    // Placeholder: No compression, just pass data through
    output_data = input_data;
    output_size = input_size;
}

void handle_input(int argc, char* argv[], int* blocksize) {
    int x;
    extern char *optarg;

    while ((x = getopt(argc, argv, ":b:")) != -1) {
        switch (x) {
        case 'b':
            *blocksize = atoi(optarg);
            printf("blocksize is set to %d optarg\n", *blocksize);
            break;
        case ':':
            printf("-%c without parameter\n", optopt);
            break;
        }
    }
}

int main(int argc, char* argv[]) {
    stopwatch ethernet_timer;
    unsigned char* input[NUM_PACKETS];
    int writer = 0;
    int done = 0;
    int length = 0;
    int count = 0;
    ESE532_Server server;

    // default is 2k
    int blocksize = BLOCKSIZE;

    // set blocksize if declared through command line
    handle_input(argc, argv, &blocksize);

    file = (unsigned char*) malloc(sizeof(unsigned char) * 70000000);
    if (file == NULL) {
        printf("Memory allocation failed for file buffer\n");
        return 1;
    }

    for (int i = 0; i < NUM_PACKETS; i++) {
        input[i] = (unsigned char*) malloc(sizeof(unsigned char) * (NUM_ELEMENTS + HEADER));
        if (input[i] == NULL) {
            std::cout << "Memory allocation failed for input buffer" << std::endl;
            return 1;
        }
    }

    server.setup_server(blocksize);

    writer = PIPE_DEPTH;
    server.get_packet(input[writer]);
    count++;

    // get packet
    unsigned char* buffer = input[writer];

    // decode
    done = buffer[1] & DONE_BIT_L;
    length = buffer[0] | (buffer[1] << 8);
    length &= ~DONE_BIT_H;
    // printing takes time so be wary of transfer rate
    // printf("length: %d offset %d\n", length, offset);

    // Placeholder processing
    size_t chunk_start = 0;
    size_t chunk_length = 0;

    // Process the received buffer
    while (chunk_start < length) {
        // Content-Defined Chunking
        content_defined_chunking(&buffer[HEADER], length, chunk_start, chunk_length);

        unsigned char* chunk_data = &buffer[HEADER + chunk_start];

        // Compute placeholder hash
        uint32_t chunk_hash = placeholder_hash(chunk_data, chunk_length);

        // Placeholder deduplication (simple hash map)
        // For simplicity, we're not implementing an actual deduplication mechanism here
        // In practice, you would check if the chunk_hash exists and handle duplicates

        // Placeholder LZW Encoding
        unsigned char* compressed_data = nullptr;
        size_t compressed_size = 0;
        lzw_encode(chunk_data, chunk_length, compressed_data, compressed_size);

        // Store the processed chunk into the file buffer
        memcpy(&file[offset], compressed_data, compressed_size);
        offset += compressed_size;

        chunk_start += chunk_length;
    }

    writer++;

    // Process remaining packets
    while (!done) {
        // reset ring buffer
        if (writer == NUM_PACKETS) {
            writer = 0;
        }

        ethernet_timer.start();
        server.get_packet(input[writer]);
        ethernet_timer.stop();

        count++;

        // get packet
        unsigned char* buffer = input[writer];

        // decode
        done = buffer[1] & DONE_BIT_L;
        length = buffer[0] | (buffer[1] << 8);
        length &= ~DONE_BIT_H;
        // printf("length: %d offset %d\n", length, offset);

        // Placeholder processing
        chunk_start = 0;
        chunk_length = 0;

        // Process the received buffer
        while (chunk_start < length) {
            // Content-Defined Chunking
            content_defined_chunking(&buffer[HEADER], length, chunk_start, chunk_length);

            unsigned char* chunk_data = &buffer[HEADER + chunk_start];

            // Compute placeholder hash
            uint32_t chunk_hash = placeholder_hash(chunk_data, chunk_length);

            // Placeholder deduplication (simple hash map)
            // For simplicity, we're not implementing an actual deduplication mechanism here

            // Placeholder LZW Encoding
            unsigned char* compressed_data = nullptr;
            size_t compressed_size = 0;
            lzw_encode(chunk_data, chunk_length, compressed_data, compressed_size);

            // Store the processed chunk into the file buffer
            memcpy(&file[offset], compressed_data, compressed_size);
            offset += compressed_size;

            chunk_start += chunk_length;
        }

        writer++;
    }

    // write file to root and you can use diff tool on board
    FILE *outfd = fopen("output_cpu.bin", "wb");
    if (outfd == NULL) {
        perror("Error opening output file");
        return 1;
    }
    int bytes_written = fwrite(&file[0], 1, offset, outfd);
    printf("write file with %d bytes\n", bytes_written);
    fclose(outfd);

    for (int i = 0; i < NUM_PACKETS; i++) {
        free(input[i]);
    }

    free(file);
    std::cout << "--------------- Key Throughputs ---------------" << std::endl;
    float ethernet_latency = ethernet_timer.latency() / 1000.0;
    float input_throughput = (bytes_written * 8 / 1000000.0) / ethernet_latency; // Mb/s
    std::cout << "Input Throughput to Encoder: " << input_throughput << " Mb/s."
              << " (Latency: " << ethernet_latency << "s)." << std::endl;

    return 0;
}
