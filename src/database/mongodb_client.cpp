#include "database/mongodb_client.hpp"
#include <chrono>
#include <mongocxx/v_noabi/mongocxx/instance.hpp>
#include <mongocxx/v_noabi/mongocxx/uri.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>

namespace {

void EnsureMongoInstance() {
    static mongocxx::instance instance{};
    (void)instance; // Suppress unused variable warning
}

} // anonymous namespace

namespace database {

MongoDBClient::MongoDBClient(const std::string& uri, const std::string& db_name, const std::string& collection_name) {
    EnsureMongoInstance(); // Ensure instance is initialized before creating client
    client_ = mongocxx::client{mongocxx::uri{uri}};
    auto db = client_[db_name];
    collection_ = db[collection_name];
}

mongocxx::cursor MongoDBClient::FindAll() {
    return collection_.find({});
}

std::vector<Document> MongoDBClient::FindByIds(const containers::HashSet<std::string>& ids) {
    std::vector<Document> results;
    
    try {
        bsoncxx::builder::basic::array oid_array;
        for (const auto& id : ids) {
            try {
                oid_array.append(bsoncxx::oid{id});
            } catch (const std::exception& e) {
                continue;
            }
        }

        if (oid_array.view().empty()) {
            return results;
        }

        auto query = bsoncxx::builder::basic::make_document(
            bsoncxx::builder::basic::kvp("_id", 
                bsoncxx::builder::basic::make_document(
                    bsoncxx::builder::basic::kvp("$in", oid_array)
                )
            )
        );

        auto cursor = collection_.find(query.view());

        for (auto&& doc : cursor) {
            Document result;

            result.id = doc["_id"].get_oid().value.to_string();
            
            if (doc["pageid"]) {
                result.pageid = doc["pageid"].get_int32().value;
            }
            if (doc["text"]) {
                result.text = std::string(doc["text"].get_string().value);
            }
            if (doc["title"]) {
                result.title = std::string(doc["title"].get_string().value);
            }
            if (doc["url"]) {
                result.url = std::string(doc["url"].get_string().value);
            }
            if (doc["created_at"]) {
                result.created_at = doc["created_at"].get_int32().value;;
            }
            
            results.push_back(std::move(result));
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error in FindByIds: " << e.what() << std::endl;
    }
    
    return results;
}

std::vector<Document> MongoDBClient::GetAllDocuments() {
    std::vector<Document> documents;
    auto cursor = FindAll();
    
    for (auto&& doc : cursor) {
        Document document;
        document.id = doc["_id"].get_oid().value.to_string();

        if (doc["pageid"]) {
            document.pageid = doc["pageid"].get_int32().value;
        }
        if (doc["text"]) {
            document.text = std::string(doc["text"].get_string().value);
        }
        if (doc["title"]) {
            document.title = std::string(doc["title"].get_string().value);
        }
        if (doc["url"]) {
            document.url = std::string(doc["url"].get_string().value);
        }
        if (doc["created_at"]) {
            document.created_at = doc["created_at"].get_int32().value;
        }
        
        documents.push_back(document);
    }
    
    return documents;
}

} // namespace database

