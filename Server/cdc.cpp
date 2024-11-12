#include "cdc.h"
#include <vector>
#include <string>
static uint64_t last_hash = 0;

int expo_table[]={1, 3, 9, 27, 81, 243, 729, 2187, 6561, 19683, 59049, 177147, 531441, 1594323, 4782969, 14348907, 43046721, 129140163};

static uint64_t hash_func(unsigned char *input, unsigned int pos)
{
	uint64_t hash = 0; 
    if(last_hash == 0){
        for(int i = 0; i < WIN_SIZE; i++){
        	hash += static_cast<uint64_t>(input[pos + WIN_SIZE - 1 - i]) * expo_table[i+1];
        }
    }
    else{
        hash = last_hash * PRIME - static_cast<uint64_t>(input[pos - 1]) * expo_table[WIN_SIZE + 1] + static_cast<uint64_t>(input[pos + WIN_SIZE - 1]) * PRIME;
    }
	// std::cout << hash << std::endl;
    last_hash = hash;

	return hash;
}

void cdc(unsigned char* buffer, std::vector<std::string> &chunks, unsigned int buff_size){
    //do some operation to get the chunks 
    std::cout << "Using cdc to split the input into chunks ..." << std::endl;
    std::string chunk;
    //initialize the first chunk
    for(unsigned int i = 0; i < WIN_SIZE; i++){
        chunk.push_back(buffer[i]);
    }

	for(unsigned int i = WIN_SIZE; i < buff_size - WIN_SIZE; i++){
        chunk.push_back(buffer[i]);
        uint64_t hash = hash_func(buffer, i);
        if((hash % MODULUS) == TARGET){
            chunks.push_back(chunk);
            chunk.clear();
        }
	}

    //push the remaining characters into the last chunk
    //push the last chunk into chunks
    for(unsigned int i = buff_size - WIN_SIZE; i < buff_size; i++){
        chunk.push_back(buffer[i]);
    }
    chunks.push_back(chunk);
    chunk.clear();

    last_hash = 0;
    std::cout << "Finish generating chunks using cdc." << std::endl;
    std::cout << "The number of chunk is " << chunks.size() << std::endl;
}

int main() {
    // Specify the path to the input file
    const std::string file_path = "../LittlePrince.txt";

    // Open the file in binary mode
    std::ifstream file(file_path, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open " << file_path << std::endl;
        return 1;
    }

    // Read the file contents into a buffer
    std::vector<unsigned char> buffer((std::istreambuf_iterator<char>(file)),
                                      std::istreambuf_iterator<char>());
    file.close();

    // Check if buffer is non-empty
    if (buffer.empty()) {
        std::cerr << "File is empty." << std::endl;
        return 1;
    }

    // Prepare a vector to hold the resulting chunks
    std::vector<std::string> chunks;

    // Perform Content-Defined Chunking (CDC)
    cdc(buffer.data(), chunks, static_cast<unsigned int>(buffer.size()));

    // Variables to track chunk positions
    size_t current_position = 0;

    // Process and print each chunk
    for (size_t i = 0; i < chunks.size(); ++i) {
        const std::string& chunk = chunks[i];
        size_t chunk_size = chunk.size();
        size_t start = current_position;
        size_t end = current_position + chunk_size - 1;

        // Print chunk information
        std::cout << "Chunk " << i + 1 << ": "
                  << "Start=" << start << ", "
                  << "End=" << end << ", "
                  << "Size=" << chunk_size << " bytes" 
                  << "Chunk=" << chunk 
                  << std::endl;

        // Update the current position
        current_position += chunk_size;
    }

    std::cout << "Total number of chunks: " << chunks.size() << std::endl;

    return 0;
}