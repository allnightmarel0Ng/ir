#ifndef INDEXING_INDEXER_HPP
#define INDEXING_INDEXER_HPP

#include "search/boolean_search.hpp"
#include "database/mongodb_client.hpp"
#include "containers/hash_map.hpp"
#include <string>
#include <vector>

namespace indexing {

struct IndexingStats {
    size_t docs_count;
    size_t total_bytes;
    size_t total_tokens;
    size_t total_chars;
    double elapsed_seconds;
    std::vector<size_t> top_frequencies;
};

class Indexer {
public:
    Indexer(database::MongoDBClient& db_client);
    void BuildIndex();
    IndexingStats GetStats() const;
    search::InvertedIndex& GetIndex();
    containers::HashMap<std::wstring, size_t>& GetTermFrequencies();

private:
    database::MongoDBClient& db_client_;
    search::InvertedIndex index_;
    containers::HashMap<std::wstring, size_t> term_frequencies_;
    IndexingStats stats_;
    
    void ProcessDocument(const database::Document& doc);
    void CalculateTopFrequencies();
};

} // namespace indexing

#endif // INDEXING_INDEXER_HPP

