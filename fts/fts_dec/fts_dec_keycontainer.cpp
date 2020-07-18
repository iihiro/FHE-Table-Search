#include <unordered_map>
#include <stdsc/stdsc_exception.hpp>
#include <stdsc/stdsc_log.hpp>
#include <fts_dec/fts_dec_keycontainer.hpp>

#include <seal/seal.h>


#define CHECK_KIND(k) do {                                               \
        if (!((k) < kNumOfKind)) {                                       \
            std::ostringstream oss;                                      \
            oss << "Err: Invalid securekey kind. (kind: " << (k) << ")"; \
            STDSC_THROW_INVPARAM(oss.str().c_str());                     \
        }                                                                \
    } while(0)


namespace fts_dec
{

void print_parameters(std::shared_ptr<seal::SEALContext> context)
{
    // Verify parameters
    if (!context)
    {
        throw invalid_argument("context is not set");
    }
    auto &context_data = *context->context_data();

    /*
    Which scheme are we using?
    */
    std::string scheme_name;
    switch (context_data.parms().scheme())
    {
    case seal::scheme_type::BFV:
        scheme_name = "BFV";
        break;
    case seal::scheme_type::CKKS:
        scheme_name = "CKKS";
        break;
    default:
        throw invalid_argument("unsupported scheme");
    }

    std::cout << "/ Encryption parameters:" << std::endl;
    std::cout << "| scheme: " << scheme_name << std::endl;
    std::cout << "| poly_modulus_degree: " <<
        context_data.parms().poly_modulus_degree() << std::endl;

    /*
    Print the size of the true (product) coefficient modulus.
    */
    std::cout << "| coeff_modulus size: " << context_data.
        total_coeff_modulus_bit_count() << " bits" << std::endl;

    /*
    For the BFV scheme print the plain_modulus parameter.
    */
    if (context_data.parms().scheme() == seal::scheme_type::BFV)  {
        std::cout << "| plain_modulus: " << context_data.
            parms().plain_modulus().value() << std::endl;
    }

    std::cout << "\\ noise_standard_deviation: " << context_data.
        parms().noise_standard_deviation() << std::endl;
    std::cout << std::endl;
}
    
struct KeyContainer::Impl
{
    enum Kind_t : int32_t
    {
        kKindUnknown   = -1,
        kKindPubKey    = 0,
        kKindSecKey    = 1,
        kKindGaloisKey = 2,
        kKindRelinKey  = 3,
        kKindParam     = 4,
        kNumOfKind,
    };
    
    struct KeyFilenames
    {    
        KeyFilenames(const int32_t id)
        {
            filenames_.emplace(kKindPubKey,    std::string("pubkey_")    + id);
            filenames_.emplace(kKindSecKey,    std::string("seckey_")    + id);
            filenames_.emplace(kKindGaloisKey, std::string("galoiskey_") + id);
            filenames_.emplace(kKindRelinKey,  std::string("relinkey_")  + id);
            filenames_.emplace(kKindParam,     std::string("param_")     + id);
        }

        std::string filename(const Kind_t kind) const
        {
            CHECK_KIND(kind);
            return filenames_.at(kind);
        }
        
        std::unordered_map<Kind_t, std::string> filenames_;
    };
    
    Impl()
    {}

    int32_t new(const std::size_t poly_mod_degree,
                const std::size_t coef_mod_192,
                const std::size_t plain_mod)
    {
        int32_t keyID = generate_keyID();
        generate_keyfiles(poly_mod_degree, coef_mod_192, plain_mod, map_.at(keyID));
        map_.emplace(keyID, KeyFilenames(keyID));
        return keyID;
    }
    
    void delete(const int32_t keyID)
    {
        remove_keyfiles(map_.at(keyID));
        map_.erase(keyID);
    }

    template <T>
    void get(const int32_t keyID, const Kind_t kind, T& data) const
    {
        auto& filename = map_.at(keyID).filename(kind);
        if (!fts_share::utility::file_exist(filename)) {
            std::ostringstream oss;
            oss << "File is not found. (" << filename << ")";
            STDSC_THROW_FILE(ret, oss.str());
        }
        std::ifstream ifs(filename);
        data.unsafe_load(ifs);
        ifs.close();
    }

    void get_param(const int32_t keyID, seal::EncryptionParameters& param) const
    {
        auto& filename = map_.at(Kind_t::kKindParam).filename(kind);
        if (!fts_share::utility::file_exist(filename)) {
            std::ostringstream oss;
            oss << "File is not found. (" << filename << ")";
            STDSC_THROW_FILE(ret, oss.str());
        }
        std::ifstream ifs(filename);
        param = EncryptionParameters::Load(ifs);
        ifs.close();
    }

    size_t size(const int32_t keyID, const Kind_t kind) const
    {
        auto& filename = map_.at(Kind_t::kKindParam).filename(kind);
        if (!fts_share::utility::file_exist(filename)) {
            std::ostringstream oss;
            oss << "File is not found. (" << filename << ")";
            STDSC_THROW_FILE(ret, oss.str());
        }
        return fts_share::utility::file_size(filename);
    }
    
private:
    int32_t generate_keyID()
    {
        return 123;
    }

    void generate_keyfiles(const std::size_t poly_mod_degree,
                           const std::size_t coef_mod_192,
                           const std::size_t plain_mod,
                           const KeyFilenames& filenames)
    {
        STDSC_LOG_INFO("Generating keys");
        seal::EncryptionParameters parms(scheme_type::BFV);
        parms.set_poly_modulus_degree(poly_mod_degree);
        parms.set_coeff_modulus(DefaultParams::coeff_modulus_192(coef_mod_192));
        parms.set_plain_modulus(plain_mod);

        auto context = seal::SEALContext::Create(parms);
        print_parameters(context);

        seal::KeyGenerator keygen(context);
        seal::PublicKey public_key = keygen.public_key();
        seal::SecretKey secret_key = keygen.secret_key();
        auto gal_keys = keygen.galois_keys(16);
        auto relin_keys16 = keygen.relin_keys(16);
        seal::BatchEncoder batch_encoder(context);

        size_t slot_count = batch_encoder.slot_count();
        size_t row_size = slot_count / 2;
        std::cout << "Plaintext matrix row size: " << row_size << std::endl;
        std::cout << "Slot nums = " << slot_count << std::endl;

        std::cout << "Save public key and secret key..." << flush;
        std::ofstream pkFile(filenames.filename(Kind_t::kKindPubKey), ios::binary);
        public_key.save(pkFile);
        pkFile.close();

        std::ofstream skFile(filenames.filename(Kind_t::kKindSecKey), ios::binary);
        secret_key.save(skFile);
        skFile.close();

        std::ofstream parmsFile(filenames.filename(Kind_t::kKindParam), ios::binary);
        seal::EncryptionParameters::Save(parms, parmsFile);
        parmsFile.close();

        std::ofstream galFile(filenames.filename(Kind_t::kKindGaloisKey), ios::binary);
        gal_keys.save(galFile);
        galFile.close();

        std::ofstream relinFile(filenames.filename(Kind_t::kKindRelinKey), ios::binary);
        relin_keys16.save(relinFile);
        relinFile.close();

        std::cout << "End" << std::endl;
    }

    void remove_keyfiles(const KeyFilenames& filenames)
    {
        int32_t bgn = static_cast<int32_t>(Kind_t::kKindPubKey);
        int32_t end = static_cast<int32_t>(Kind_t::kKindParam);

        for (auto i=bgn; i<=end; ++i) {
            auto key = static_cast<Kind_t>(i);
            auto ret = fts_share::utility::remove_file(filenames.at(key));
            if (!ret) {
                std::ostringstream oss;
                oss << "Failed to remove file. (" << filenames.at(key) << ")";
                STDSC_THROW_FILE(ret, oss.str());
            }
        }
    }
    
private:
    std::unordered_map<int32_t, KeyFilenames> map_;
};

KeyContainer::KeyContainer()
    : pimpl_(new Impl())
{}

int32_t KeyContainer::new()
{
    return pimpl_->new();
}

void KeyContainer::delete(const int32_t keyID)
{
    pimpl_->delete(keyID);
}

} /* namespace fts_dec */
