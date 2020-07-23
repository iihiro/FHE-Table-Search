#include <stdsc/stdsc_log.hpp>
#include <fts_cs/fts_cs_query.hpp>
#include <fts_cs/fts_cs_result.hpp>
#include <fts_cs/fts_cs_calcthread.hpp>
#include <seal/seal.h>

namespace fts_cs
{
    
struct CalcThread::Impl
{
    Impl(const QueryQueue& in_queue, ResultQueue& out_queue)
        : in_queue_(in_queue), out_queue_(out_queue)
    {
    }

    void exec(CalcThreadParam& args, std::shared_ptr<stdsc::ThreadException> te)
    {
        STDSC_LOG_INFO("Launched calc thread.");
        // 次回、in_queue_からpopしてダミー計算して、out_queue_へダミーの結果をpushするところから
    }

public:
    CalcThreadParam param_;
    std::shared_ptr<stdsc::ThreadException> te_;
    
private:
    const QueryQueue& in_queue_;
    ResultQueue& out_queue_;
};

CalcThread::CalcThread(const QueryQueue& in_queue, ResultQueue& out_queue)
    : pimpl_(new Impl(in_queue, out_queue))
{}

void CalcThread::start()
{
    super::start(pimpl_->param_, pimpl_->te_);
}

void CalcThread::stop()
{
    
}

void CalcThread::exec(CalcThreadParam& args, std::shared_ptr<stdsc::ThreadException> te) const
{
    pimpl_->exec(args, te);
}

} /* namespace fts_dec */
