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
#include <fts_share/fts_packet.hpp>
#include <fts_share/fts_plaindata.hpp>
#include <fts_share/fts_csparam.hpp>
#include <fts_share/fts_seal_utility.hpp>
#include <fts_user/fts_user_cs_client.hpp>

namespace fts_user
{

struct CSClient::Impl
{
    struct ResultCallback
    {
        std::shared_ptr<ResultThread<>> thread;
        ResultThreadParam param;
    };
    
    Impl(const char* host, const char* port,
         const seal::EncryptionParameters& enc_params)
        : host_(host),
          port_(port),
          enc_params_(enc_params),
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

    int32_t send_query(const int32_t key_id, const int32_t func_no,
                       const fts_share::EncData& enc_inputs)
    {
        fts_share::PlainData<fts_share::CSParam> splaindata;
        fts_share::CSParam csparam {key_id, static_cast<fts_share::FuncNo_t>(func_no)};
        splaindata.push(csparam);

        auto sz = (splaindata.stream_size()
                   + fts_share::seal_utility::stream_size(enc_params_)
                   + enc_inputs.stream_size());
        stdsc::BufferStream sbuffstream(sz);
        std::iostream stream(&sbuffstream);
        
        splaindata.save_to_stream(stream);
        seal::EncryptionParameters::Save(enc_params_, stream);
        enc_inputs.save_to_stream(stream);

        stdsc::Buffer* sbuffer = &sbuffstream;
        stdsc::Buffer rbuffer;
        client_.send_recv_data_blocking(fts_share::kControlCodeUpDownloadQuery, *sbuffer, rbuffer);
        STDSC_LOG_INFO("sent query");

        stdsc::BufferStream rbuffstream(rbuffer);
        std::iostream rstream(&rbuffstream);
        fts_share::PlainData<int32_t> rplaindata;
        rplaindata.load_from_stream(rstream);

        return rplaindata.data();
    }

    void recv_results(const int32_t query_id, fts_share::EncData& enc_result)
    {
        fts_share::PlainData<int32_t> splaindata;
        splaindata.push(query_id);

        auto sz = (splaindata.stream_size()
                   + fts_share::seal_utility::stream_size(enc_params_));
        stdsc::BufferStream sbuffstream(sz);
        std::iostream stream(&sbuffstream);

        splaindata.save_to_stream(stream);
        seal::EncryptionParameters::Save(enc_params_, stream);

        stdsc::Buffer* sbuffer = &sbuffstream;
        stdsc::Buffer rbuffer;
        client_.send_recv_data_blocking(fts_share::kControlCodeUpDownloadResult, *sbuffer, rbuffer);
        STDSC_LOG_INFO("request result for query#%d", query_id);

        stdsc::BufferStream rbuffstream(rbuffer);
        std::iostream rstream(&rbuffstream);
        enc_result.load_from_stream(rstream);
        fts_share::seal_utility::write_to_file("result.txt", enc_result.data());
    }

    void set_callback(const int32_t query_id, nbc_client::cbfunc_t func, void* args)
    {
        ResultCallback rcb;
        rcb.thread = std::make_shared<ResultThread<>>(client_, func, args);
        rcb.param  = {query_id};
        cbmap_[query_id] = rcb;
        cbmap_[query_id].thread->start(cbmap_[query_id].param);
    }

    void wait(const int32_t query_id) const
    {
        STDSC_LOG_TRACE("waiting result for query#%d", query_id);
        if (cbmap_.count(query_id)) {
            auto& rcb = cbmap_.at(query_id);
            rcb.thread->wait();
        }
    }

private:
    const char* host_;
    const char* port_;
    const seal::EncryptionParameters enc_params_;
    stdsc::Client client_;
    std::unordered_map<int32_t, ResultCallback> cbmap_;
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

int32_t CSClient::send_query(const int32_t key_id, const int32_t func_no,
                             const fts_share::EncData& enc_inputs) const
{
    return pimpl_->send_query(key_id, func_no, enc_inputs);
}

int32_t CSClient::send_query(const int32_t key_id, const int32_t func_no,
                             const fts_share::EncData& enc_inputs,
                             nbc_client::cbfunc_t cbfunc,
                             void* cbfunc_args) const
{
    int32_t query_id = pimpl_->send_query(key_id, func_no, enc_inputs);
    set_callback(query_id, cbfunc, cbfunc_args);
    return query_id;
}

void CSClient::recv_results(const int32_t query_id, fts_share::EncData& enc_result) const
{
    pimpl_->recv_results(query_id, enc_result);
}

void CSClient::set_callback(const int32_t query_id, nbc_client::cbfunc_t func, void* args) const
{
    pimpl_->set_callback(func, args);
}

void CSClient::wait(const int32_t query_id) const
{
    pimpl_->wait(query_id);
}

} /* namespace fts_user */
