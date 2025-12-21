#ifndef WEB_SERVER_HPP
#define WEB_SERVER_HPP

#include "indexing/indexer.hpp"
#include "database/mongodb_client.hpp"
#include <string>
#include <functional>
#include <memory>

namespace web {

class Server {
public:
    Server(indexing::Indexer& indexer, database::MongoDBClient& db_client, int port);
    void Start();
    void Stop();

private:
    indexing::Indexer& indexer_;
    database::MongoDBClient& db_client_;
    int port_;
    void* server_impl_; // Will be httplib::Server*
    
    std::string HandleSearch(const std::string& query);
    std::string HandleStats();
    std::string HandleHealth();
};

} // namespace web

#endif // WEB_SERVER_HPP

