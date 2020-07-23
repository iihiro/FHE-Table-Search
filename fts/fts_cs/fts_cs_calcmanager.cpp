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
    }
    
    void CalcManager::stop_threads()
    {
    }
    
    int32_t CalcManager::push_query(const Query& query)
    {
        return pimpl_->qque_.push(query);
    }
    
    bool CalcManager::pop_result(const int32_t query_id, Result& result) const
    {
        return true;
    }
    
} /* namespace fts_cs */
