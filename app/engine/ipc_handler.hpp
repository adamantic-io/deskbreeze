#pragma once

#include <string>
#include <functional>
#include <map>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

namespace dbr {
namespace ipc {

using json = nlohmann::json;

class IPCMessage {
public:
    std::string method;
    json params;
    std::string id;
    
    IPCMessage() = default;
    IPCMessage(const std::string& method, const json& params, const std::string& id = "")
        : method(method), params(params), id(id) {}
    
    json to_json() const {
        json msg;
        msg["method"] = method;
        msg["params"] = params;
        if (!id.empty()) {
            msg["id"] = id;
        }
        return msg;
    }
    
    static IPCMessage from_json(const json& j) {
        IPCMessage msg;
        if (j.contains("method")) {
            msg.method = j["method"];
        }
        if (j.contains("params")) {
            msg.params = j["params"];
        }
        if (j.contains("id")) {
            msg.id = j["id"];
        }
        return msg;
    }
};

class IPCResponse {
public:
    json result;
    std::string error;
    std::string id;
    
    IPCResponse() = default;
    IPCResponse(const json& result, const std::string& id = "")
        : result(result), id(id) {}
    
    IPCResponse(const std::string& error, const std::string& id = "")
        : error(error), id(id) {}
    
    json to_json() const {
        json resp;
        if (!error.empty()) {
            resp["error"] = error;
        } else {
            resp["result"] = result;
        }
        if (!id.empty()) {
            resp["id"] = id;
        }
        return resp;
    }
};

using IPCHandler = std::function<IPCResponse(const IPCMessage&)>;

class IPCHandlerRegistry {
private:
    std::map<std::string, IPCHandler> handlers_;
    
public:
    void register_handler(const std::string& method, IPCHandler handler) {
        handlers_[method] = handler;
    }
    
    IPCResponse handle_message(const IPCMessage& message) {
        auto it = handlers_.find(message.method);
        if (it != handlers_.end()) {
            try {
                return it->second(message);
            } catch (const std::exception& e) {
                return IPCResponse("Handler error: " + std::string(e.what()), message.id);
            }
        }
        return IPCResponse("Unknown method: " + message.method, message.id);
    }
    
    IPCResponse handle_json_message(const std::string& json_str) {
        try {
            spdlog::debug("Handling IPC message: {}", json_str);
            json j = json::parse(json_str);
            spdlog::debug("Parsed IPC message: {}", j.dump());
            IPCMessage message = IPCMessage::from_json(j);
            spdlog::debug("IPC message method: {}, params: {}, id: {}", 
                          message.method, message.params.dump(), message.id);
            return handle_message(message);
        } catch (const std::exception& e) {
            return IPCResponse("JSON parse error: " + std::string(e.what()));
        }
    }
};

} // namespace ipc
} // namespace dbr