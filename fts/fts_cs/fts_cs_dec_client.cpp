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
#include <fts_share/fts_packet.hpp>
#include <fts_share/fts_plaindata.hpp>
#include <fts_share/fts_cs2decparam.hpp>
#include <fts_share/fts_encdata.hpp>
#include <fts_cs/fts_cs_dec_client.hpp>

namespace fts_cs
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
        client_.connect(host_, port_, retry_interval_usec, timeout_sec);
    }

    void disconnect(void)
    {
        client_.close();
    }

    template <class T>
    void get_key(const int32_t key_id, const fts_share::ControlCode_t code, T& key)
    {
        stdsc::Buffer sbuffer(sizeof(key_id)), rbuffer;
        *(int32_t*)sbuffer.data() = key_id;
        client_.send_recv_data_blocking(code, sbuffer, rbuffer);

        stdsc::BufferStream buffstream(rbuffer);
        std::iostream stream(&buffstream);
        key.unsafe_load(stream);
    }
    
    void get_param(const int32_t key_id, seal::EncryptionParameters& param)
    {
        stdsc::Buffer sbuffer(sizeof(key_id)), rbuffer;
        *(int32_t*)sbuffer.data() = key_id;
        client_.send_recv_data_blocking(fts_share::kControlCodeUpDownloadParam, sbuffer, rbuffer);
    
        stdsc::BufferStream buffstream(rbuffer);
        std::iostream stream(&buffstream);
        param = seal::EncryptionParameters::Load(stream);
    }

    void get_PIRquery(const int32_t key_id, const int32_t query_id,
                      const fts_share::EncData& enc_midresult,
                      fts_share::EncData& enc_PIRquery,
                      fts_share::EncData& enc_PIRindex)
    {
        fts_share::PlainData<fts_share::Cs2DecParam> splaindata;
        fts_share::Cs2DecParam param = {key_id, query_id};
        splaindata.push(param);

        auto sz = splaindata.stream_size() + enc_midresult.stream_size();
        stdsc::BufferStream sbuffstream(sz);
        std::iostream stream(&sbuffstream);

        splaindata.save_to_stream(stream);
        enc_midresult.save_to_stream(stream);

        stdsc::Buffer* sbuffer = &sbuffstream;
        stdsc::Buffer rbuffer;
        client_.send_recv_data_blocking(fts_share::kControlCodeUpDownloadCsMidResult, *sbuffer, rbuffer);
        STDSC_LOG_INFO("sent mid-results");

        stdsc::BufferStream rbuffstream(rbuffer);
        std::iostream rstream(&rbuffstream);

        enc_PIRquery.load_from_stream(rstream);
        enc_PIRindex.load_from_stream(rstream);
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
    STDSC_LOG_INFO("Connect to decryptor.");
    pimpl_->connect(retry_interval_usec, timeout_sec);
}

void DecClient::disconnect(void)
{
    STDSC_LOG_INFO("Disconnect from decryptor.");
    pimpl_->disconnect();
}

void DecClient::get_pubkey(const int32_t key_id, seal::PublicKey& pubkey)
{
    STDSC_LOG_INFO("Get public key: sending request of #%d to decryptor.", key_id);
    pimpl_->get_key(key_id, fts_share::kControlCodeUpDownloadPubKey, pubkey);
    STDSC_LOG_INFO("Get public key: received key of #%d", key_id);
}
    
void DecClient::get_galoiskey(const int32_t key_id, seal::GaloisKeys& galoiskey)
{
    STDSC_LOG_INFO("Get galois keys: sending request of #%d to decryptor.", key_id);
    pimpl_->get_key(key_id, fts_share::kControlCodeUpDownloadGaloisKey, galoiskey);
    STDSC_LOG_INFO("Get galois keys: received key of #%d", key_id);
}
    
void DecClient::get_relinkey(const int32_t key_id, seal::RelinKeys& relinkey)
{
    STDSC_LOG_INFO("Get relin keys: sending request of #%d to decryptor.", key_id);
    pimpl_->get_key(key_id, fts_share::kControlCodeUpDownloadRelinKey, relinkey);
    STDSC_LOG_INFO("Get relin keys: received key of #%d", key_id);
}

void DecClient::get_param(const int32_t key_id, seal::EncryptionParameters& param)
{
    STDSC_LOG_INFO("Get encryption parameters: sending request of #%d to decryptor.", key_id);
    pimpl_->get_param(key_id, param);
    STDSC_LOG_INFO("Get encryption parameters: received key of #%d", key_id);
}

void DecClient::get_PIRquery(const int32_t key_id, const int32_t query_id,
                             const fts_share::EncData& enc_midresult,
                             fts_share::EncData& enc_PIRquery,
                             fts_share::EncData& enc_PIRindex)
                               
{
    STDSC_LOG_INFO("Get PIR queries: sending request of query #%d to decryptor.", query_id);
    pimpl_->get_PIRquery(key_id, query_id, enc_midresult, enc_PIRquery, enc_PIRindex);
    STDSC_LOG_INFO("Get PIR queries:: received PIR queries of query #%d", query_id);
}


} /* namespace fts_cs */