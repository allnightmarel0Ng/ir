#include "web/server.hpp"
#include "search/boolean_search.hpp"
#include <httplib.h>
#include <sstream>
#include <iostream>
#include <json/json.h>
#include <optional>

namespace {

constexpr const char* kContentTypeJson = "application/json";
constexpr const char* kContentTypeText = "text/plain; charset=utf-8";

std::string CreateJsonResponse(const std::string& status, const std::string& data) {
    Json::Value root;
    root["status"] = status;
    root["data"] = data;
    
    Json::StreamWriterBuilder builder;
    return Json::writeString(builder, root);
}

std::string CreateErrorResponse(const std::string& message) {
    Json::Value root;
    root["status"] = "error";
    root["message"] = message;
    
    Json::StreamWriterBuilder builder;
    return Json::writeString(builder, root);
}

std::optional<std::string> ParseJsonQuery(const std::string& body) {
    Json::Value root;
    Json::CharReaderBuilder builder;
    std::string errors;
    std::istringstream stream(body);
    
    if (!Json::parseFromStream(builder, stream, &root, &errors)) {
        return std::nullopt;
    }
    
    if (!root.isMember("query") || !root["query"].isString()) {
        return std::nullopt;
    }
    
    return root["query"].asString();
}

} // anonymous namespace

namespace web {

Server::Server(indexing::Indexer& indexer, database::MongoDBClient& db_client, int port)
    : indexer_(indexer), db_client_(db_client), port_(port), server_impl_(nullptr) {
}

void Server::Start() {
    auto* server = new httplib::Server();
    server_impl_ = server;
    
    server->Get("/health", [this](const httplib::Request&, httplib::Response& res) {
        res.set_content(HandleHealth(), kContentTypeText);
    });
    
    server->Get("/stats", [this](const httplib::Request&, httplib::Response& res) {
        res.set_content(HandleStats(), kContentTypeJson);
    });
    
    server->Post("/search", [this](const httplib::Request& req, httplib::Response& res) {
        auto query_opt = ParseJsonQuery(req.body);
        if (!query_opt.has_value()) {
            res.status = 400;
            res.set_content(CreateErrorResponse("Invalid JSON or missing 'query' field"), kContentTypeJson);
            return;
        }
        res.set_content(HandleSearch(query_opt.value()), kContentTypeJson);
    });
    
    std::cout << "Server starting on port " << port_ << std::endl;
    server->listen("0.0.0.0", port_);
}

void Server::Stop() {
    if (server_impl_) {
        auto* server = static_cast<httplib::Server*>(server_impl_);
        server->stop();
        delete server;
        server_impl_ = nullptr;
    }
}

std::string Server::HandleSearch(const std::string& query) {
    try {
        auto& index = indexer_.GetIndex();
        auto result = search::BooleanSearchRu(query, index);
        
        Json::Value root;
        root["status"] = "success";
        root["count"] = static_cast<Json::UInt64>(result.Size());
        
        Json::Value documents(Json::arrayValue);
        auto mongo_documents = db_client_.FindByIds(result);
        for (const auto& mongo_document : mongo_documents) {
            Json::Value doc_obj;
            doc_obj["id"] = mongo_document.id;
            doc_obj["pageid"] = mongo_document.pageid;
            doc_obj["title"] = mongo_document.title;
            doc_obj["text"] = mongo_document.text;
            doc_obj["url"] = mongo_document.url;
            doc_obj["created_at"] = mongo_document.created_at.time_since_epoch().count();
            documents.append(doc_obj);
        }
        root["documents"] = documents;
        
        Json::StreamWriterBuilder builder;
        return Json::writeString(builder, root);
    } catch (const std::exception& e) {
        return CreateErrorResponse(std::string("Search error: ") + e.what());
    }
}

std::string Server::HandleStats() {
    try {
        auto stats = indexer_.GetStats();
        
        Json::Value root;
        root["status"] = "success";
        root["docs_count"] = static_cast<Json::UInt64>(stats.docs_count);
        root["total_bytes"] = static_cast<Json::UInt64>(stats.total_bytes);
        root["total_bytes_kb"] = stats.total_bytes / 1024.0;
        root["total_tokens"] = static_cast<Json::UInt64>(stats.total_tokens);
        root["avg_token_length"] = (stats.total_tokens > 0 ? (double)stats.total_chars / stats.total_tokens : 0.0);
        root["indexing_time_seconds"] = stats.elapsed_seconds;
        root["indexing_speed_kb_per_sec"] = (stats.elapsed_seconds > 0 ? (stats.total_bytes / 1024.0) / stats.elapsed_seconds : 0.0);
        
        Json::Value frequencies(Json::arrayValue);
        for (size_t i = 0; i < stats.top_frequencies.size(); ++i) {
            Json::Value freq_obj;
            freq_obj["rank"] = static_cast<Json::UInt64>(i + 1);
            freq_obj["frequency"] = static_cast<Json::UInt64>(stats.top_frequencies[i]);
            frequencies.append(freq_obj);
        }
        root["top_frequencies"] = frequencies;
        
        Json::StreamWriterBuilder builder;
        return Json::writeString(builder, root);
    } catch (const std::exception& e) {
        return CreateErrorResponse(std::string("Stats error: ") + e.what());
    }
}

std::string Server::HandleHealth() {
    return "OK";
}

} // namespace web

