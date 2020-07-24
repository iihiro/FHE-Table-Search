#include <unistd.h>
#include <stdsc/stdsc_log.hpp>
#include <fts_share/fts_seal_utility.hpp>
#include <fts_cs/fts_cs_query.hpp>
#include <fts_cs/fts_cs_result.hpp>
#include <fts_cs/fts_cs_calcthread.hpp>
#include <fts_cs/fts_cs_dec_client.hpp>
#include <seal/seal.h>

namespace fts_cs
{
    
struct CalcThread::Impl
{
    Impl(QueryQueue& in_queue, ResultQueue& out_queue,
         const std::string& dec_host, const std::string& dec_port)
        : dec_host_(dec_host), dec_port_(dec_port),
          in_queue_(in_queue), out_queue_(out_queue)
    {
    }

    void exec(CalcThreadParam& args, std::shared_ptr<stdsc::ThreadException> te)
    {
        STDSC_LOG_INFO("Launched calc thread.");

        while (!args.force_finish) {

            int32_t query_id;
            Query query;
            while (!in_queue_.pop(query_id, query)) {
                usleep(args.retry_interval_msec * 1000);
            }

            // queryを使って処理をする
            // resultが生成される
            Result result;
            compute(query, result);
            
            out_queue_.push(query_id, result);
        }
    }

    void compute(const Query& query, Result& result)
    {
        DecClient dec_client(dec_host_.c_str(), dec_port_.c_str());
        dec_client.connect();

        seal::PublicKey pubkey;
        seal::GaloisKeys galoiskey;
        seal::RelinKeys relinkey;
        seal::EncryptionParameters params(seal::scheme_type::BFV);
        
        dec_client.get_pubkey(query.key_id_, pubkey);
        fts_share::seal_utility::write_to_file("pubkey.txt", pubkey);

        dec_client.get_galoiskey(query.key_id_, galoiskey);
        fts_share::seal_utility::write_to_file("galoiskey.txt", galoiskey);

        dec_client.get_relinkey(query.key_id_, relinkey);
        fts_share::seal_utility::write_to_file("relinkey.txt", relinkey);

        dec_client.get_param(query.key_id_, params);
        fts_share::seal_utility::write_to_file("param.txt", params);
    }

    const std::string& dec_host_;
    const std::string& dec_port_;
    QueryQueue& in_queue_;
    ResultQueue& out_queue_;
    CalcThreadParam param_;
    std::shared_ptr<stdsc::ThreadException> te_;
};

CalcThread::CalcThread(QueryQueue& in_queue, ResultQueue& out_queue,
                       const std::string& dec_host, const std::string& dec_port)
    : pimpl_(new Impl(in_queue, out_queue, dec_host, dec_port))
{}

void CalcThread::start()
{
    pimpl_->param_.force_finish = false;
    super::start(pimpl_->param_, pimpl_->te_);
}

void CalcThread::stop()
{
    pimpl_->param_.force_finish = true;
}

void CalcThread::exec(CalcThreadParam& args, std::shared_ptr<stdsc::ThreadException> te) const
{
    pimpl_->exec(args, te);
}

} /* namespace fts_dec */
