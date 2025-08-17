#pragma once

#include <httplib.h>
#include <memory>

#include "common_defs.hpp"

namespace dbr {

using std::make_unique;

class Server {
public:
    ErrorCode start();
    ErrorCode stop();
    ErrorCode configure();
    bool is_valid() const { return srv_->is_valid(); }
    bool is_running() const { return thread_ && thread_->joinable() && srv_->is_running(); }
    void wait_until_ready() {
        return srv_->wait_until_ready();
    }
    Server() : srv_(make_unique<httplib::Server>()) { }
    virtual ~Server();


protected:
    virtual ErrorCode own_configure(httplib::Server& srv);
    std::unique_ptr<std::thread> thread_;
    std::unique_ptr<httplib::Server> srv_;
    bool is_configured_ = false;
};


}