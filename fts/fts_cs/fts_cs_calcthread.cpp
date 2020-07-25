#include <unistd.h>
#include <algorithm> // for sort
#include <chrono>
#include <random>
#include <fstream>
#include <sys/types.h>   // for thread id
#include <sys/syscall.h> // for thread id
#include <stdsc/stdsc_log.hpp>
#include <fts_share/fts_seal_utility.hpp>
#include <fts_share/fts_commonparam.hpp>
#include <fts_share/fts_encdata.hpp>
#include <fts_cs/fts_cs_query.hpp>
#include <fts_cs/fts_cs_result.hpp>
#include <fts_cs/fts_cs_calcthread.hpp>
#include <fts_cs/fts_cs_dec_client.hpp>
#include <seal/seal.h>

namespace fts_cs
{

unsigned g_seed = std::chrono::system_clock::now().time_since_epoch().count();
std::mt19937 g_generator(g_seed);
    
static bool cmp(std::pair<int64_t, int64_t> a, std::pair<int64_t, int64_t> b)
{
    return a.second < b.second;
}
    
static std::vector<int64_t> get_randomvector(int64_t total)
{
    std::vector<std::pair<int64_t, int64_t> > input;
    std::vector<int64_t> output;
    for (int64_t i=0; i<total; ++i) {
        input.push_back({i, g_generator() % 1000000});
    }
    std::sort(input.begin(), input.end(), cmp);
    for (int64_t i=0; i<total; ++i) {
        output.push_back(input[i].first);
    }
    return output;
}

static void create_LUT(const std::vector<std::vector<int64_t>>& LUT,
                       const std::vector<int64_t>& randomVector,
                       std::vector<std::vector<int64_t>>& LUT_input,
                       std::vector<std::vector<int64_t>>& LUT_output,
                       const int64_t l, const int64_t k)
{
    std::vector<int64_t> sub_input;
    std::vector<int64_t> sub_output;
    int64_t total = randomVector.size();
    int64_t row_size = l;
    int64_t index = 0;

    STDSC_LOG_INFO("create new LUT. (l:%ld, k:%ld, total:%ld)",
                   l, k, total);
    
    for (int64_t i=0; i<k; ++i) {
        for(int64_t j=0; j<row_size; ++j) {
            int64_t s = randomVector[index];
            int64_t temp_in = LUT[0][s];
            sub_input.push_back(temp_in);
            int64_t temp_out=LUT[1][s];
            sub_output.push_back(temp_out);
            ++index;
        }
        sub_input.resize(l);
        LUT_input.push_back(sub_input);
        sub_input.clear();
        sub_output.resize(l);
        LUT_output.push_back(sub_output);
        sub_output.clear();
    }
}

    
struct CalcThread::Impl
{
    Impl(QueryQueue& in_queue, ResultQueue& out_queue,
         std::vector<std::vector<int64_t>>& oriLUT,
         const std::string& dec_host, const std::string& dec_port)
        : in_queue_(in_queue), out_queue_(out_queue),
          oriLUT_(oriLUT), dec_host_(dec_host), dec_port_(dec_port)
    {
    }

    void exec(CalcThreadParam& args, std::shared_ptr<stdsc::ThreadException> te)
    {
        auto th_id = syscall(SYS_gettid);
        STDSC_LOG_INFO("Launched calcuration thread. (thread ID: %d)", th_id);
        
        while (!args.force_finish) {

            STDSC_LOG_INFO("[th:%d] Try getting query from QueryQueue.", th_id);

            int32_t query_id;
            Query query;
            while (!in_queue_.pop(query_id, query)) {
                usleep(args.retry_interval_msec * 1000);
            }

            bool status = false;
            
            STDSC_LOG_INFO("[th:%d] Get query #%d.", th_id, query_id);

            seal::PublicKey pubkey;
            seal::GaloisKeys galoiskey;
            seal::RelinKeys relinkey;
            seal::EncryptionParameters params(seal::scheme_type::BFV);
            STDSC_LOG_INFO("[th:%d] Start preprocess of query #%d.", th_id, query_id);
            preprocess(query, pubkey, galoiskey, relinkey, params);
            STDSC_LOG_INFO("[th:%d] Finish preprocess of query #%d.", th_id, query_id);
            
            seal::Ciphertext new_PIR_query, new_PIR_index;
            std::vector<std::vector<int64_t>> LUT_output;
            STDSC_LOG_INFO("[th:%d] Start computationA of query #%d.", th_id, query_id);
            status = computeA(query_id, query,
                              pubkey, galoiskey, relinkey, params,
                              LUT_output, new_PIR_query, new_PIR_index);
            STDSC_LOG_INFO("[th:%d] Finish computationA of query #%d.", th_id, query_id);

            seal::Ciphertext sum_result;
            if (status) {
                STDSC_LOG_INFO("[th:%d] Start computationB of query #%d.", th_id, query_id);
                status = computeB(query_id, query,
                                pubkey, galoiskey, relinkey, params,
                                LUT_output, new_PIR_query, new_PIR_index, sum_result);
                STDSC_LOG_INFO("[th:%d] Finish computationB of query #%d.", th_id, query_id);
            }
                
            Result result(query_id, status, sum_result);
            out_queue_.push(query_id, result);

            STDSC_LOG_INFO("[th:%d] Set result of query #%d.", th_id, query_id);
        }
    }

    void preprocess(const Query& query,
                    seal::PublicKey& pubkey,
                    seal::GaloisKeys& galoiskey,
                    seal::RelinKeys& relinkey,
                    seal::EncryptionParameters& params)
    {
        DecClient dec_client(dec_host_.c_str(), dec_port_.c_str());
        dec_client.connect();
        
        dec_client.get_pubkey(query.key_id_, pubkey);
        dec_client.get_galoiskey(query.key_id_, galoiskey);
        dec_client.get_relinkey(query.key_id_, relinkey);
        dec_client.get_param(query.key_id_, params);

#if defined ENABLE_LOCAL_DEBUG
        {
            fts_share::seal_utility::write_to_file("pubkey.txt", pubkey);
            fts_share::seal_utility::write_to_file("galoiskey.txt", galoiskey);
            fts_share::seal_utility::write_to_file("relinkey.txt", relinkey);
            fts_share::seal_utility::write_to_file("param.txt", params);
        }
#endif
    }
    
    bool computeA(const int32_t query_id, const Query& query,
                  const seal::PublicKey& pubkey,
                  const seal::GaloisKeys& galoiskey,
                  const seal::RelinKeys& relinkey,
                  const seal::EncryptionParameters& params,
                  std::vector<std::vector<int64_t>>& LUT_output,
                  seal::Ciphertext& new_PIR_query,
                  seal::Ciphertext& new_PIR_index)
    {
        DecClient dec_client(dec_host_.c_str(), dec_port_.c_str());
        dec_client.connect();

        // two inputのときは、ここで2つ取り出す
        auto& ciphertext_query = query.ctxts_[0];
        auto context = seal::SEALContext::Create(params);

        seal::Evaluator evaluator(context);
        seal::BatchEncoder batch_encoder(context);
        size_t slot_count = batch_encoder.slot_count();
        size_t row_size = slot_count / 2;
        std::cout << "  Plaintext matrix row size: " << row_size << std::endl;
        std::cout << "  Slot nums = " << slot_count << std::endl;

        int64_t l = row_size;
        int64_t k = ceil(FTS_COMMONPARAM_TABLE_SIZE_N / row_size);

        std::vector<int64_t> vi = get_randomvector(FTS_COMMONPARAM_TABLE_SIZE_N);
        
        std::vector<std::vector<int64_t>> LUT_input;
        create_LUT(oriLUT_, vi, LUT_input, LUT_output, l, k);

#if defined ENABLE_LOCAL_DEBUG
        //write shifted_output_table in a file
        {
            std::ofstream OutputTable;
            OutputTable.open("queryt");
            for(int i=0; i<k; ++i) {
                for(int j=0; j<l; ++j) {
                    OutputTable << LUT_output[i][j] <<' ';
                }
                OutputTable << std::endl;
            }
            OutputTable.close();
        }
#endif

        std::cout << "  Compute every row of table" << std::endl;
        
        std::vector<seal::Ciphertext> Result;
        for(int i=0; i<k; ++i) {
            seal::Ciphertext tep;
            Result.push_back(tep);
        }

        //omp_set_num_threads(NF);
        //#pragma omp parallel for
        for(int64_t i=0; i<k; ++i) {
            seal::Ciphertext res = ciphertext_query;
            seal::Plaintext poly_row;
            batch_encoder.encode(LUT_input[i], poly_row);
            evaluator.sub_plain_inplace(res, poly_row);
            evaluator.relinearize_inplace(res, relinkey);

            std::vector<int64_t> random_value_vec;
            for(size_t sk=0; sk<row_size; ++sk) {
                int64_t random_value = (g_generator() % 5 + 1);
                random_value_vec.push_back(random_value);
            }
            random_value_vec.resize(slot_count);
            seal::Plaintext poly_num;
            batch_encoder.encode(random_value_vec, poly_num);

            evaluator.multiply_plain_inplace(res, poly_num);
            evaluator.relinearize_inplace(res, relinkey);
            Result[i]=res;
        }

#if defined ENABLE_LOCAL_DEBUG
        {
            std::cout << "Saving..." << std::flush;
            std::ofstream outResult;
            outResult.open("queryc", std::ios::binary);
            for(int i=0; i<k; ++i) {
                Result[i].save(outResult);
            }
            outResult.close();
            std::cout << "OK" << std::endl;
        }
#endif

        std::cout << "  Send intermediate resutls to decryptor" << std::endl;
        fts_share::EncData enc_midresult(params, Result);
        fts_share::EncData enc_PIRquery(params), enc_PIRindex(params);
        auto res = dec_client.get_PIRquery(query.key_id_, query_id, enc_midresult,
                                           enc_PIRquery, enc_PIRindex);

        if (res != fts_share::kDecCalcResultSuccess) {
            STDSC_LOG_WARN("  Failed to calcurate PIR queries on decryptor. (errno: %d)",
                           static_cast<int32_t>(res));
            return false;
        }
        
        std::cout << "  Received PIR queries from decryptor" << std::endl;
        
#if defined ENABLE_LOCAL_DEBUG
        {
            auto& queryd = enc_PIRquery.data();
            auto& queryi = enc_PIRindex.data();
            fts_share::seal_utility::write_to_file("queryd", queryd);
            fts_share::seal_utility::write_to_file("queryi", queryi);
        }
#endif

        new_PIR_query = enc_PIRquery.data();
        new_PIR_index = enc_PIRindex.data();

        return true;
    }

    bool computeB(const int32_t query_id, const Query& query,
                  const seal::PublicKey& pubkey,
                  const seal::GaloisKeys& galoiskey,
                  const seal::RelinKeys& relinkey,
                  const seal::EncryptionParameters& params,
                  const std::vector<std::vector<int64_t>>& LUT,
                  const seal::Ciphertext& new_PIR_query,
                  const seal::Ciphertext& new_PIR_index,
                  seal::Ciphertext& sum_result)
    {
        auto context = seal::SEALContext::Create(params);

        seal::Encryptor encryptor(context, pubkey);
        seal::Evaluator evaluator(context);
        seal::BatchEncoder batch_encoder(context);

        size_t slot_count = batch_encoder.slot_count();
        size_t row_size = slot_count / 2;
        std::cout << "  Plaintext matrix row size: " << row_size << std::endl;
        std::cout << "  Slot nums = " << slot_count << std::endl;

        int64_t k = ceil(FTS_COMMONPARAM_TABLE_SIZE_N / row_size);

        const seal::Ciphertext& new_query = new_PIR_query;
        const seal::Ciphertext& new_index = new_PIR_index;

        std::vector<seal::Ciphertext> res;
        for (int i=0; i<k; ++i) {
            seal::Ciphertext tep;
            res.push_back(tep);
        }

        std::vector<std::vector<int64_t>> tmpLUT(LUT.size());
        std::copy(LUT.begin(), LUT.end(), tmpLUT.begin());
        
        //omp_set_num_threads(NF);
        //#pragma omp parallel for
        for (int64_t i=0; i<k; ++i) {
            tmpLUT[i].resize(slot_count);
            seal::Plaintext poly_table_row;
            batch_encoder.encode(tmpLUT[i], poly_table_row);
            seal::Ciphertext temp = new_index;
            evaluator.rotate_rows_inplace(temp, -i, galoiskey);
            evaluator.multiply_inplace(temp, new_query);
            evaluator.relinearize_inplace(temp, relinkey);
            evaluator.multiply_plain_inplace(temp, poly_table_row);
            evaluator.relinearize_inplace(temp, relinkey);
            res[i]=temp;
        }

        sum_result = res[0];
        for(int i=1; i<k; ++i) {
            evaluator.add_inplace(sum_result, res[i]);
            evaluator.relinearize_inplace(sum_result, relinkey);
        }

#if defined ENABLE_LOCAL_DEBUG
        //write Final_result in a file
        {
            std::cout << "Saving final result..." << std::flush;
            std::ofstream Final_result;
            Final_result.open("queryr", std::ios::binary);
            sum_result.save(Final_result);
            Final_result.close();
            std::cout << "OK" << std::endl;
        }
#endif

        return true;
    }
    
    QueryQueue& in_queue_;
    ResultQueue& out_queue_;
    const std::vector<std::vector<int64_t>>& oriLUT_;
    const std::string& dec_host_;
    const std::string& dec_port_;
    CalcThreadParam param_;
    std::shared_ptr<stdsc::ThreadException> te_;
};

CalcThread::CalcThread(QueryQueue& in_queue, ResultQueue& out_queue,
                       std::vector<std::vector<int64_t>>& oriLUT,
                       const std::string& dec_host, const std::string& dec_port)
    : pimpl_(new Impl(in_queue, out_queue, oriLUT, dec_host, dec_port))
{}

void CalcThread::start()
{
    pimpl_->param_.force_finish = false;
    super::start(pimpl_->param_, pimpl_->te_);
}

void CalcThread::stop()
{
    STDSC_LOG_INFO("Stop calculation thread.");
    pimpl_->param_.force_finish = true;
}

void CalcThread::exec(CalcThreadParam& args, std::shared_ptr<stdsc::ThreadException> te) const
{
    pimpl_->exec(args, te);
}

} /* namespace fts_dec */
