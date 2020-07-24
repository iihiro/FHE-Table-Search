#include <unistd.h>
#include <algorithm> // for sort
//#include <utility> // for pair
#include <chrono>
#include <random>
#include <fstream>
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

            seal::PublicKey pubkey;
            seal::GaloisKeys galoiskey;
            seal::RelinKeys relinkey;
            seal::EncryptionParameters params(seal::scheme_type::BFV);
            preprocess(query, pubkey, galoiskey, relinkey, params);
            
            seal::Ciphertext new_PIR_query, new_PIR_index;
            std::vector<std::vector<int64_t>> LUT_output;
            computeA(query_id, query,
                     pubkey, galoiskey, relinkey, params,
                     LUT_output, new_PIR_query, new_PIR_index);

            seal::Ciphertext sum_result;
            computeB(query_id, query,
                     pubkey, galoiskey, relinkey, params,
                     LUT_output, new_PIR_query, new_PIR_index, sum_result);

            //fts_share::EncData enc_result(params, result);
            Result result(query_id, sum_result);
            out_queue_.push(query_id, result);
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

        // for debug
        {
            fts_share::seal_utility::write_to_file("pubkey.txt", pubkey);
            fts_share::seal_utility::write_to_file("galoiskey.txt", galoiskey);
            fts_share::seal_utility::write_to_file("relinkey.txt", relinkey);
            fts_share::seal_utility::write_to_file("param.txt", params);
        }
    }

    
    void computeB(const int32_t query_id, const Query& query,
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
        std::cout << "Plaintext matrix row size: " << row_size << std::endl;
        std::cout << "Slot nums = " << slot_count << std::endl;

        //int64_t l = row_size;
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

        //result sum
        //seal::Ciphertext& sum_result = result;
        sum_result = res[0];
        for(int i=1; i<k; ++i) {
            evaluator.add_inplace(sum_result, res[i]);
            evaluator.relinearize_inplace(sum_result, relinkey);
        }

        // auto end1=chrono::high_resolution_clock::now();
        // chrono::duration<double> diff = end1-start1;
        // cout << "Select output time is: " << diff.count() << "s" << endl;

        //write Final_result in a file
        {
            std::cout << "Saving final result..." << std::flush;
            std::ofstream Final_result;
            Final_result.open("queryr", std::ios::binary);
            //Final_result << fin_res ;
            sum_result.save(Final_result);
            Final_result.close();
            std::cout << "OK" << std::endl;
        }
    }
    
    void computeA(const int32_t query_id, const Query& query,
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

        //seal::PublicKey pubkey;
        //seal::GaloisKeys galoiskey;
        //seal::RelinKeys relinkey;
        //seal::EncryptionParameters params(seal::scheme_type::BFV);
        //
        //dec_client.get_pubkey(query.key_id_, pubkey);
        //dec_client.get_galoiskey(query.key_id_, galoiskey);
        //dec_client.get_relinkey(query.key_id_, relinkey);
        //dec_client.get_param(query.key_id_, params);
        //
        //// for debug
        //{
        //    fts_share::seal_utility::write_to_file("pubkey.txt", pubkey);
        //    fts_share::seal_utility::write_to_file("galoiskey.txt", galoiskey);
        //    fts_share::seal_utility::write_to_file("relinkey.txt", relinkey);
        //    fts_share::seal_utility::write_to_file("param.txt", params);
        //}

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
        //std::vector<std::vector<int64_t>> LUT_output;koko
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

        {
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

        fts_share::EncData enc_midresult(params, Result);
        fts_share::EncData enc_PIRquery(params), enc_PIRindex(params);
        dec_client.set_midresults(query.key_id_, query_id, enc_midresult,
                                  enc_PIRquery, enc_PIRindex);

        {
            auto& queryd = enc_PIRquery.data();
            auto& queryi = enc_PIRindex.data();
            fts_share::seal_utility::write_to_file("queryd", queryd);
            fts_share::seal_utility::write_to_file("queryi", queryi);
        }

        new_PIR_query = enc_PIRquery.data();
        new_PIR_index = enc_PIRindex.data();
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
