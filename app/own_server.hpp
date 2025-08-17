#pragma once

#include "engine/http_server.hpp"
#include "common_defs.hpp"


class OwnServer : public dbr::Server {
public:
    virtual ~OwnServer() {}
protected:
    virtual dbr::ErrorCode own_configure(httplib::Server& srv) override;
};