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
#include <unordered_map>
#include <vector>

#define NUM_PACKETS 8
#define pipe_depth 4
#define DONE_BIT_L (1 << 7)
#define DONE_BIT_H (1 << 15)
#define CHUNK_SIZE 1024
#define HASH_TABLE_SIZE 1000

int offset = 0;
unsigned char* file;

struct Chunk {
    unsigned char* data;
    size_t size;
};

// Placeholder function for Content-Defined Chunking (CDC)
void content_defined_chunking(unsigned char* data, size_t length, std::vector<Chunk>& chunks) {
    size_t num_chunks = (length + CHUNK_SIZE - 1) / CHUNK_SIZE;
    for (size_t i = 0; i < num_chunks; i++) {
        Chunk chunk;
        chunk.size = (i == num_chunks - 1) ? (length % CHUNK_SIZE) : CHUNK_SIZE;
        chunk.data = (unsigned char*)malloc(chunk.size);
        memcpy(chunk.data, &data[i * CHUNK_SIZE], chunk.size);
        chunks.push_back(chunk);
    }
}

// Placeholder for Deduplication using a simple hash table
bool is_unique_chunk(const unsigned char* chunk_data, size_t size, std::unordered_map<size_t, bool>& dedup_table) {
    size_t hash = 0;
    for (size_t i = 0; i < size; i++) {
        hash = (hash * 31 + chunk_data[i]) % 1000003;
    }
    if (dedup_table.find(hash) != dedup_table.end()) {
        return false; // Duplicate chunk
    } else {
        dedup_table[hash] = true;
        return true; // Unique chunk
    }
}

// Placeholder function for LZW Encoding
void lzw_encoding(const unsigned char* data, size_t length, std::vector<uint16_t>& encoded_data) {
    for (size_t i = 0; i < length; i++) {
        encoded_data.push_back(static_cast<uint16_t>(data[i]));
    }
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

    int blocksize = BLOCKSIZE;

    handle_input(argc, argv, &blocksize);

    file = (unsigned char*)malloc(sizeof(unsigned char) * 70000000);
    if (file == NULL) {
        printf("Memory allocation failed\n");
        return 1;
    }

    for (int i = 0; i < NUM_PACKETS; i++) {
        input[i] = (unsigned char*)malloc(sizeof(unsigned char) * (NUM_ELEMENTS + HEADER));
        if (input[i] == NULL) {
            std::cout << "Memory allocation failed" << std::endl;
            return 1;
        }
    }

    server.setup_server(blocksize);

    writer = pipe_depth;
    server.get_packet(input[writer]);
    count++;

    std::unordered_map<size_t, bool> dedup_table;

    unsigned char* buffer = input[writer];
    done = buffer[1] & DONE_BIT_L;
    length = buffer[0] | (buffer[1] << 8);
    length &= ~DONE_BIT_H;

    std::vector<Chunk> chunks;
    content_defined_chunking(&buffer[HEADER], length, chunks);

    for (auto& chunk : chunks) {
        if (is_unique_chunk(chunk.data, chunk.size, dedup_table)) {
            std::vector<uint16_t> encoded_data;
            lzw_encoding(chunk.data, chunk.size, encoded_data);

            for (auto& code : encoded_data) {
                memcpy(&file[offset], &code, sizeof(uint16_t));
                offset += sizeof(uint16_t);
            }
        }
        free(chunk.data);
    }
    writer++;

    while (!done) {
        if (writer == NUM_PACKETS) {
            writer = 0;
        }

        ethernet_timer.start();
        server.get_packet(input[writer]);
        ethernet_timer.stop();

        count++;

        unsigned char* buffer = input[writer];
        done = buffer[1] & DONE_BIT_L;
        length = buffer[0] | (buffer[1] << 8);
        length &= ~DONE_BIT_H;

        content_defined_chunking(&buffer[HEADER], length, chunks);

        for (auto& chunk : chunks) {
            if (is_unique_chunk(chunk.data, chunk.size, dedup_table)) {
                std::vector<uint16_t> encoded_data;
                lzw_encoding(chunk.data, chunk.size, encoded_data);

                for (auto& code : encoded_data) {
                    memcpy(&file[offset], &code, sizeof(uint16_t));
                    offset += sizeof(uint16_t);
                }
            }
            free(chunk.data);
        }
        writer++;
    }

    FILE *outfd = fopen("output_cpu.bin", "wb");
    int bytes_written = fwrite(&file[0], 1, offset, outfd);
    printf("File written with %d bytes\n", bytes_written);
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
