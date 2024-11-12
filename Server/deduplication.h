#ifndef DEDUPLICATION_H
#define DEDUPLICATION_H

#include <string>
#include <cstdint>
#include <unordered_set>

bool dedup(std::unordered_set<std::string>& hash_table, const std::string& hash_value);

#endif