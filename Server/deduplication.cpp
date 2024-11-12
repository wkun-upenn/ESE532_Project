// deduplication.cpp

#include "common.h"
#include <cstdlib>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <string>

using namespace std;

// Return -1 if the chunk is new; otherwise, return the existing chunk ID.
bool dedup(std::unordered_set<std::string>& hash_table, const std::string& hash_value) {

    auto result = hash_table.insert(hash_value);
    
    return !result.second;
}