#include <fts_cs/fts_cs_query.hpp>
#include <seal/seal.h>

namespace fts_cs
{

Query::Query(const int32_t key_id, const int32_t func_no,
             const std::vector<seal::Ciphertext>& ctxts)
    : key_id_(key_id),
      func_no_(func_no)
{
    ctxts_.resize(ctxts.size());
    std::copy(ctxts.begin(), ctxts.end(), ctxts_.begin());
}

} /* namespace fts_cs */
