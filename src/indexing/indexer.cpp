#include "indexing/indexer.hpp"
#include "text_processing/tokenizer.hpp"
#include "text_processing/stemmer.hpp"
#include <chrono>
#include <algorithm>

namespace {

constexpr size_t kTopFrequenciesCount = 10;

} // anonymous namespace

namespace indexing {

Indexer::Indexer(database::MongoDBClient& db_client) : db_client_(db_client) {
    stats_ = {};
}

void Indexer::BuildIndex() {
    stats_ = {};
    auto start_time = std::chrono::high_resolution_clock::now();

    auto documents = db_client_.GetAllDocuments();
    
    for (const auto& doc : documents) {
        ProcessDocument(doc);
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;
    stats_.elapsed_seconds = elapsed.count();
    
    CalculateTopFrequencies();
}

void Indexer::ProcessDocument(const database::Document& doc) {
    stats_.total_bytes += doc.text.size();
    stats_.docs_count++;

    auto tokens = text_processing::TokenizeRu(doc.text);
    for (const auto& t : tokens) {
        auto stem = text_processing::StemRu(t);
        index_[stem].Insert(doc.id);
        
        term_frequencies_[stem]++;
        stats_.total_tokens++;
        stats_.total_chars += t.length();
    }
}

void Indexer::CalculateTopFrequencies() {
    std::vector<size_t> freqs;
    for (const auto& node : term_frequencies_) {
        freqs.push_back(node.value);
    }

    std::sort(freqs.rbegin(), freqs.rend());
    
    size_t count = std::min(kTopFrequenciesCount, freqs.size());
    if (count > 0) {
        stats_.top_frequencies.assign(freqs.begin(), freqs.begin() + count);
    }
}

IndexingStats Indexer::GetStats() const {
    return stats_;
}

search::InvertedIndex& Indexer::GetIndex() {
    return index_;
}

containers::HashMap<std::wstring, size_t>& Indexer::GetTermFrequencies() {
    return term_frequencies_;
}

} // namespace indexing

