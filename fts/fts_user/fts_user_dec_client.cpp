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

namespace fts_user
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

    int32_t new_keys(seal::SecretKey& seckey)
    {
        stdsc::Buffer buffer;
        client_.recv_data_blocking(fts_share::kControlCodeDownloadNewKeys, buffer);
        STDSC_LOG_INFO("create new keys");

        stdsc::BufferStream buffstream(buffer);
        std::iostream stream(&buffstream);

        fts_share::PlainData<int32_t> plaindata;
        plaindata.load_from_stream(stream);
        seckey.unsafe_load(stream);

        return plaindata.data();
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

    void get_pubkey(const int32_t keyID, seal::PublicKey& pubkey)
    {
    }
    
    void get_galoiskey(const int32_t keyID, seal::GaloisKeys& galoiskey)
    {
    }
    
    void get_relinkey(const int32_t keyID, seal::RelinKeys& relinkey)
    {
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

int32_t DecClient::new_keys(seal::SecretKey& seckey)
{
    return pimpl_->new_keys(seckey);
}

bool DecClient::delete_keys(const int32_t key_id) const
{
    bool res = pimpl_->delete_keys(key_id);

    return res;
}

void DecClient::get_pubkey(const int32_t keyID, seal::PublicKey& pubkey)
{
    pimpl_->get_pubkey(keyID, pubkey);
}
    
void DecClient::get_galoiskey(const int32_t keyID, seal::GaloisKeys& galoiskey)
{
    pimpl_->get_galoiskey(keyID, galoiskey);
}
    
void DecClient::get_relinkey(const int32_t keyID, seal::RelinKeys& relinkey)
{
    pimpl_->get_relinkey(keyID, relinkey);
}



} /* namespace fts_user */
