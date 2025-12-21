#ifndef DATABASE_MONGODB_CLIENT_HPP
#define DATABASE_MONGODB_CLIENT_HPP

#include "containers/hash_set.hpp"
#include <chrono>
#include <mongocxx/v_noabi/mongocxx/client.hpp>
#include <mongocxx/v_noabi/mongocxx/collection.hpp>
#include <mongocxx/v_noabi/mongocxx/cursor.hpp>
#include <string>
#include <vector>

namespace database {

struct Document {
    std::string id;
    std::string url;
    std::string title;
    std::chrono::system_clock::time_point created_at;
    std::string pageid;
    std::string text;
};

class MongoDBClient {
public:
    MongoDBClient(const std::string& uri, const std::string& db_name, const std::string& collection_name);
    mongocxx::cursor FindAll();
    std::vector<Document> FindByIds(const containers::HashSet<std::string>& ids);
    std::vector<Document> GetAllDocuments();

private:
    mongocxx::client client_;
    mongocxx::collection collection_;
};

} // namespace database

#endif // DATABASE_MONGODB_CLIENT_HPP

