#include <unistd.h>
#include <algorithm> // for sort
//#include <utility> // for pair
#include <chrono>
#include <random>
#include <fstream>
#include <stdsc/stdsc_log.hpp>
#include <fts_share/fts_seal_utility.hpp>
#include <fts_share/fts_commonparam.hpp>
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
    std::cout << k << ' ' << l << std::endl;
    std::vector<int64_t> sub_input;
    std::vector<int64_t> sub_output;
    int64_t Total = randomVector.size();
    std::cout << Total << std::endl;
    int64_t row_size = l;
    int64_t index = 0;

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
    std::cout << "end" << std::endl;
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
        dec_client.get_galoiskey(query.key_id_, galoiskey);
        dec_client.get_relinkey(query.key_id_, relinkey);
        dec_client.get_param(query.key_id_, params);

        // for debug
        {
            fts_share::seal_utility::write_to_file("pubkey.txt", pubkey);
            fts_share::seal_utility::write_to_file("galoiskey.txt", galoiskey);
            fts_share::seal_utility::write_to_file("relinkey.txt", relinkey);
            fts_share::seal_utility::write_to_file("param.txt", params);
        }

        // two inputのときは、ここで2つ取り出す
        auto& ciphertext_query = query.ctxts_[0];
        auto context = seal::SEALContext::Create(params);

        seal::Evaluator evaluator(context);
        seal::BatchEncoder batch_encoder(context);
        size_t slot_count = batch_encoder.slot_count();
        size_t row_size = slot_count / 2;
        std::cout << "Plaintext matrix row size: " << row_size << std::endl;
        std::cout << "Slot nums = " << slot_count << std::endl;

        int64_t l = row_size;
        int64_t k = ceil(FTS_COMMONPARAM_TABLE_SIZE_N / row_size);

        std::vector<int64_t> vi = get_randomvector(FTS_COMMONPARAM_TABLE_SIZE_N);
        
        std::cout << "Make new LUT ..." << std::flush;
        std::vector<std::vector<int64_t>> LUT_input;
        std::vector<std::vector<int64_t>> LUT_output;
        create_LUT(oriLUT_, vi, LUT_input, LUT_output, l, k);
        std::cout << "end" << std::endl;

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

        //Make Threads to compute every row of table
        std::cout << "Make threads for each row and compute" << std::endl;
        std::vector<seal::Ciphertext> Result;
        
        for(int i=0; i<k; ++i) {
            seal::Ciphertext tep;
            Result.push_back(tep);
        }

        //omp_set_num_threads(NF);
        //#pragma omp parallel for
        for(int64_t i=0; i<k; ++i) {
            //LUT_input[i].resize(slot_count);
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
            // int64_t random_value=(generator()%5+1);
            // Plaintext poly_num = encoder.encode(random_value);

            evaluator.multiply_plain_inplace(res, poly_num);
            evaluator.relinearize_inplace(res, relinkey);
            Result[i]=res;
        }

        //Get vector<Ctxt> Result
        std::cout << "Saving..." << std::flush;
        //save in a file
        std::ofstream outResult;
        outResult.open("queryc", std::ios::binary);
        for(int i=0; i<k; ++i) {
            Result[i].save(outResult);
            //outResult<<endl;
        }
        outResult.close();
        std::cout << "OK" << std::endl;
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
    pimpl_->param_.force_finish = true;
}

void CalcThread::exec(CalcThreadParam& args, std::shared_ptr<stdsc::ThreadException> te) const
{
    pimpl_->exec(args, te);
}

} /* namespace fts_dec */
