#include "indexing/indexer.hpp"
#include "database/mongodb_client.hpp"
#include "web/server.hpp"
#include <iostream>
#include <string>
#include <csignal>
#include <thread>
#include <chrono>
#include <cstdlib>

namespace {

volatile std::sig_atomic_t g_running = 1;

void SignalHandler(int signal) {
    (void)signal; // Suppress unused parameter warning
    g_running = 0;
}

constexpr const char* kDefaultMongoUri = "mongodb://localhost:27017";
constexpr const char* kDefaultDbName = "wiki_corpus";
constexpr const char* kDefaultCollectionName = "pages";
constexpr int kDefaultServerPort = 8080;

std::string GetEnvOrDefault(const char* env_var, const char* default_value) {
    const char* value = std::getenv(env_var);
    return value ? std::string(value) : std::string(default_value);
}

int GetEnvIntOrDefault(const char* env_var, int default_value) {
    const char* value = std::getenv(env_var);
    if (value) {
        try {
            return std::stoi(value);
        } catch (...) {
            return default_value;
        }
    }
    return default_value;
}

} // anonymous namespace

int main() {
    std::signal(SIGINT, SignalHandler);
    std::signal(SIGTERM, SignalHandler);

    try {
        std::string mongo_uri = GetEnvOrDefault("MONGODB_URI", kDefaultMongoUri);
        std::string db_name = GetEnvOrDefault("DB_NAME", kDefaultDbName);
        std::string collection_name = GetEnvOrDefault("COLLECTION_NAME", kDefaultCollectionName);
        int server_port = GetEnvIntOrDefault("SERVER_PORT", kDefaultServerPort);
        
        std::cout << "Connecting to MongoDB at " << mongo_uri << "..." << std::endl;
        database::MongoDBClient db_client(mongo_uri, db_name, collection_name);
        
        std::cout << "Building index..." << std::endl;
        indexing::Indexer indexer(db_client);
        indexer.BuildIndex();
        
        auto stats = indexer.GetStats();
        std::cout << "Indexing completed:" << std::endl;
        std::cout << "  Documents: " << stats.docs_count << std::endl;
        std::cout << "  Total tokens: " << stats.total_tokens << std::endl;
        std::cout << "  Time: " << stats.elapsed_seconds << " seconds" << std::endl;
        
        std::cout << "Starting web server on port " << server_port << "..." << std::endl;
        web::Server server(indexer, db_client, server_port);
        
        // Start server in a separate thread
        std::thread server_thread([&server]() {
            server.Start();
        });
        
        std::cout << "Server is running. Press Ctrl+C to stop." << std::endl;
        
        while (g_running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        std::cout << "\nShutting down..." << std::endl;
        server.Stop();
        server_thread.join();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}

