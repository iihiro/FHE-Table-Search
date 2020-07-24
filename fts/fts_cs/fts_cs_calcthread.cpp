#include <unistd.h>
#include <stdsc/stdsc_log.hpp>
#include <fts_cs/fts_cs_query.hpp>
#include <fts_cs/fts_cs_result.hpp>
#include <fts_cs/fts_cs_calcthread.hpp>
#include <seal/seal.h>

namespace fts_cs
{
    
struct CalcThread::Impl
{
    Impl(QueryQueue& in_queue, ResultQueue& out_queue)
        : in_queue_(in_queue), out_queue_(out_queue)
    {
    }

    void exec(CalcThreadParam& args, std::shared_ptr<stdsc::ThreadException> te)
    {
        STDSC_LOG_INFO("Launched calc thread.");

        while (!args.force_finish) {

            //printf("retry_interval_msec: %u\n", args.retry_interval_msec);
            int32_t query_id;
            Query query;
            while (!in_queue_.pop(query_id, query)) {
                usleep(args.retry_interval_msec * 1000);
            }

            // queryを使って処理をする
            // resultが生成される
            Result result;
            
            out_queue_.push(query_id, result);
        }
    }


public:
    CalcThreadParam param_;
    std::shared_ptr<stdsc::ThreadException> te_;
    
private:
    QueryQueue& in_queue_;
    ResultQueue& out_queue_;
};

CalcThread::CalcThread(QueryQueue& in_queue, ResultQueue& out_queue)
    : pimpl_(new Impl(in_queue, out_queue))
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
