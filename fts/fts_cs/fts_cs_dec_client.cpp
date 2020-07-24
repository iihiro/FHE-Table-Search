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
        STDSC_LOG_INFO("Connecting to Decryptor.");
        client_.connect(host_, port_, retry_interval_usec, timeout_sec);
        STDSC_LOG_INFO("Connected to Decryptor.");
    }

    void disconnect(void)
    {
        client_.close();
    }

    //int32_t new_keys(seal::SecretKey& seckey)
    //{
    //    stdsc::Buffer buffer;
    //    client_.recv_data_blocking(fts_share::kControlCodeDownloadNewKeys, buffer);
    //    STDSC_LOG_INFO("generate new keys");
    //
    //    stdsc::BufferStream buffstream(buffer);
    //    std::iostream stream(&buffstream);
    //
    //    fts_share::PlainData<int32_t> plaindata;
    //    plaindata.load_from_stream(stream);
    //    
    //    seckey.unsafe_load(stream);
    //    
    //    return plaindata.data();
    //}
    //
    //bool delete_keys(const int32_t key_id) const
    //{
    //    STDSC_LOG_INFO("delete keys(%d) [DECClient::delete_keys()]", key_id);
    //    return true;
    //}

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

    void set_midresults(const int32_t key_id, const int32_t query_id,
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
        //fts_share::PlainData<int32_t> rplaindata;
        //rplaindata.load_from_stream(rstream);

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
    pimpl_->connect(retry_interval_usec, timeout_sec);
}

void DecClient::disconnect(void)
{
    pimpl_->disconnect();
}

//int32_t DecClient::new_keys(seal::SecretKey& seckey)
//{
//    return pimpl_->new_keys(seckey);
//}
//
//bool DecClient::delete_keys(const int32_t key_id) const
//{
//    bool res = pimpl_->delete_keys(key_id);
//
//    return res;
//}

void DecClient::get_pubkey(const int32_t key_id, seal::PublicKey& pubkey)
{
    pimpl_->get_key(key_id, fts_share::kControlCodeUpDownloadPubKey, pubkey);
}
    
void DecClient::get_galoiskey(const int32_t key_id, seal::GaloisKeys& galoiskey)
{
    pimpl_->get_key(key_id, fts_share::kControlCodeUpDownloadGaloisKey, galoiskey);
}
    
void DecClient::get_relinkey(const int32_t key_id, seal::RelinKeys& relinkey)
{
    pimpl_->get_key(key_id, fts_share::kControlCodeUpDownloadRelinKey, relinkey);
}

void DecClient::get_param(const int32_t key_id, seal::EncryptionParameters& param)
{
    pimpl_->get_param(key_id, param);
}

void DecClient::set_midresults(const int32_t key_id, const int32_t query_id,
                               const fts_share::EncData& enc_midresult,
                               fts_share::EncData& enc_PIRquery,
                               fts_share::EncData& enc_PIRindex)
                               
{
    pimpl_->set_midresults(key_id, query_id, enc_midresult, enc_PIRquery, enc_PIRindex);
}


} /* namespace fts_cs */
