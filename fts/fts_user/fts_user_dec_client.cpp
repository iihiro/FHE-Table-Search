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
#include <fts_share/fts_pubkey.hpp>
#include <fts_share/fts_seckey.hpp>
#include <fts_share/fts_packet.hpp>
#include <fts_user/fts_user_dec_client.hpp>

namespace fts_client
{

struct DecClient::Impl
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
        STDSC_LOG_INFO("Connecting to Decryptor.");
        client_.connect(host_, port_, retry_interval_usec, timeout_sec);
        STDSC_LOG_INFO("Connected to Decryptor.");
    }

    void disconnect(void)
    {
        client_.close();
    }

    int32_t new_keys(fts_share::PubKey& pubkey, fts_share::SecKey& seckey)
    {
        // stdsc::Buffer bufferA(sz);
        // client.send_data_blocking(nantoka);
        client_.send_request_blocking(fts_share::kControlCodeRequestNewKeys);

        stdsc::Buffer result;
        client_.recv_data_blocking(fts_share::kControlCodeDownloadResult, result);
        STDSC_LOG_INFO("create new keys [DECClient::new_keys()]");

        return 0;
    }

    bool delete_keys(const int32_t key_id) const
    {
        // stdsc::Buffer bufferA(sz);
        // client.send_data_blocking(nantoka);
        // client_.send_request_blocking(fts_share::kControlCodeRequestDeleteKeys);

        // stdsc::Buffer result;
        // client_.recv_data_blocking(fts_share::kControlCodeDownloadResult, result);
        STDSC_LOG_INFO("delete keys(%d) [DECClient::delete_keys()]", key_id);

        return true;
    }


private:
    const char* host_;
    const char* port_;
    stdsc::Client client_;
};

DecClient::DecClient(const char* host, const char* port)
    : pimpl_(new Impl(host, port))
{
}

void DecClient::connect(const uint32_t retry_interval_usec,
                       const uint32_t timeout_sec)
{
    pimpl_->connect(retry_interval_usec, timeout_sec);
}

void DecClient::disconnect(void)
{
    pimpl_->disconnect();
}

int32_t DecClient::new_keys(fts_share::PubKey& pubkey, fts_share::SecKey& seckey)
{
    int32_t res = pimpl_->new_keys(pubkey, seckey);

    return res;
}

bool DecClient::delete_keys(const int32_t key_id) const
{
    bool res = pimpl_->delete_keys(key_id);

    return res;
}


} /* namespace fts_client */