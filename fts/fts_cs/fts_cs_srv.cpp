#include <sstream>
#include <stdsc/stdsc_server.hpp>
#include <stdsc/stdsc_log.hpp>
#include <stdsc/stdsc_exception.hpp>
#include <fts_share/fts_utility.hpp>
#include <fts_cs/fts_cs_srv.hpp>

namespace fts_cs
{

struct CSServer::Impl
{
public:
    Impl(const char *port,
         const std::string &LUT_dirpath,
         stdsc::CallbackFunctionContainer &callback,
         stdsc::StateContext &state)
        : server_(new stdsc::Server<>(port, state, callback)),
          state_(state)
    {
        STDSC_LOG_INFO("Launched CS server [port]: %s", port);
    }

    ~Impl(void) = default;


    void start(void)
    {
        STDSC_LOG_INFO("[starting fts_cs server]");

        bool enable_async_mode = true;
        server_->start(enable_async_mode);

        STDSC_LOG_INFO("* done * [starting fts_cs server]");
    }

    void stop(void)
    {
        server_->stop();
    }

    void wait(void)
    {
        server_->wait();
    }


private:
    std::shared_ptr<stdsc::Server<>> server_;
    stdsc::StateContext& state_;
};

CSServer::CSServer(const char *port,
                   const std::string &LUT_dirpath,
                   stdsc::CallbackFunctionContainer &callback,
                   stdsc::StateContext &state,
                   const uint32_t max_concurrent_queries,
                   const uint32_t max_results,
                   const uint32_t result_lifetime_sec)
    : pimpl_(new Impl(port, LUT_dirpath, callback, state))
{
}

void CSServer::start(void)
{
    pimpl_->start();
}

void CSServer::stop(void)
{
    pimpl_->stop();
}

void CSServer::wait(void)
{
    pimpl_->wait();
}


} /* namespace fts_cs */
