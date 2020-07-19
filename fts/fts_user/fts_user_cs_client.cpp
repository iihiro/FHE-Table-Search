#include <memory>
#include <fstream>
#include <vector>
#include <cstring>
#include <stdsc/stdsc_client.hpp>
#include <stdsc/stdsc_buffer.hpp>
#include <stdsc/stdsc_packet.hpp>
#include <stdsc/stdsc_log.hpp>
#include <stdsc/stdsc_exception.hpp>
#include <fts_share/fts_utility.hpp>
#include <fts_share/fts_encdata.hpp>
//#include <fts_share/fts_seckey.hpp>
#include <fts_share/fts_packet.hpp>
#include <fts_user/fts_user_cs_client.hpp>

namespace fts_client
{

struct CSClient::Impl
{
public:
    Impl(const char* host, const char* port)
        : host_(host),
          port_(port),
          client_()
    {
    }

    ~Impl(void)
    {
        disconnect();
    }

    void connect(const uint32_t retry_interval_usec,
                 const uint32_t timeout_sec)
    {
        client_.connect(host_, port_, retry_interval_usec, timeout_sec);
        STDSC_LOG_INFO("Connected to Computation Server.");
    }

    void disconnect(void)
    {
        client_.close();
    }

    int32_t send_query(const int32_t key_id, const int32_t func_no, const std::vector<fts_share::EncData>& enc_input)
    {
        //client_.send_request_blocking(fts_share::kControlCodeRequestQuery);
        //
        //stdsc::Buffer result;
        //client_.recv_data_blocking(fts_share::kControlCodeDownloadQueryID, result);
        //STDSC_LOG_INFO("sent the query");
        //
        //return *static_cast<const uint32_t*>(result.data());
        return 0;
    }

    bool recv_result(const int32_t query_id, fts_share::EncData& enc_result)
    {
        //client_.send_request_blocking(fts_share::kControlCodeRequestResults);
        //
        //stdsc::Buffer result;
        //client_.recv_data_blocking(fts_share::kControlCodeDownloadResult, result);
        //STDSC_LOG_INFO("recieved results");

        return true;
    }


private:
    const char* host_;
    const char* port_;
    stdsc::Client client_;
};

CSClient::CSClient(const char* host, const char* port)
    : pimpl_(new Impl(host, port))
{
}

void CSClient::connect(const uint32_t retry_interval_usec,
                       const uint32_t timeout_sec)
{
    pimpl_->connect(retry_interval_usec, timeout_sec);
}

void CSClient::disconnect(void)
{
    pimpl_->disconnect();
}

int32_t CSClient::send_query(const int32_t key_id, const int32_t func_no, const std::vector<fts_share::EncData>& enc_input) const
{
    int32_t res = pimpl_->send_query(key_id, func_no, enc_input);

    return res;
}

bool CSClient::recv_result(const int32_t query_id, fts_share::EncData& enc_result) const
{
    bool res = pimpl_->recv_result(query_id, enc_result);

    return res;
}


} /* namespace fts_client */
