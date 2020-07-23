#include <vector>
#include <fts_cs/fts_cs_query.hpp>
#include <fts_cs/fts_cs_result.hpp>
#include <fts_cs/fts_cs_calcthread.hpp>
#include <fts_cs/fts_cs_calcmanager.hpp>


namespace fts_cs
{
    struct CalcManager::Impl
    {
        Impl() = default;

        QueryQueue qque_;
        ResultQueue rque_;
    };

    CalcManager::CalcManager()
        :pimpl_(new Impl())
    {}

    void CalcManager::start_threads(const uint32_t thread_num)
    {
        // 次回、thread_num個のcalcthreadを起動時に立てて、qque_からpopしてダミー計算して、rque_へpushするダミーを作るところから
    }
    
    void CalcManager::stop_threads()
    {
    }
    
    int32_t CalcManager::put(const Query& query)
    {
        return pimpl_->qque_.push(query);
    }
    
    bool CalcManager::try_get(const int32_t query_id, Result& result) const
    {
        return pimpl_->rque_.try_get(query_id, result);
    }
    
} /* namespace fts_cs */
