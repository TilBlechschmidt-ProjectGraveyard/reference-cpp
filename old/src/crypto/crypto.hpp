#ifndef PROTOMESH_CRYPTO_HPP
#define PROTOMESH_CRYPTO_HPP

#define CBC 1
#define EBC 0
#define AES256 1

#include <random>
#include <array>
#include <utility>
#include <vector>
#include <algorithm>
#include <result.h>
#include <buffers/crypto/asym_generated.h>
#include <buffers/crypto/uuid_generated.h>
extern "C" {
    /// Implemented @ lib/AES/aes.c
    void AES_CBC_encrypt_buffer(uint8_t* output, uint8_t* input, uint32_t length, const uint8_t* key, const uint8_t* iv);
    void AES_CBC_decrypt_buffer(uint8_t* output, uint8_t* input, uint32_t length, const uint8_t* key, const uint8_t* iv);
}

#include "sha512.hpp"
#include "uECC.h"
#include "../api/network.hpp"

using namespace std;

/// Defining ECC key sizes
#define PRIV_KEY_SIZE 32
#define PUB_KEY_SIZE 64
#define COMPRESSED_PUB_KEY_SIZE (PRIV_KEY_SIZE + 1)
#define SIGNATURE_SIZE PUB_KEY_SIZE

/// Define the size in bytes of the short hash to identify a public key
/// Is required to be dividable by two.
#define PUB_HASH_SIZE (PUB_KEY_SIZE / 4)
#define PUB_HASH_T array<char, PUB_HASH_SIZE>  // First PUB_HASH_SIZE characters of the HASH of the hex representation of the public key

/// Define AES related constants
#define IV_SIZE 32  // Since we use AES256 the IV may have a size of 32 * sizeof(uint8_t) = 256.

/// Defining cryptography types
#define COMPRESSED_PUBLIC_KEY_T array<uint8_t, COMPRESSED_PUB_KEY_SIZE>
#define PRIVATE_KEY_T array<uint8_t, PRIV_KEY_SIZE>
#define SIGNATURE_T array<uint8_t, PUB_KEY_SIZE>
#define SHARED_KEY_T vector<uint8_t>
#define HASH vector<uint8_t>

/// Defining the elliptic curve to use
extern const struct uECC_Curve_t* ECC_CURVE;

/// Define the cryptography namespace
namespace Crypto {
    enum UUIDType : unsigned int {
        Generic=0,
        Device,
        Endpoint
    };
    class UUID {
    public:
        uint32_t a=0, b=0, c=0, d=0;
        UUIDType type = UUIDType::Generic;

        void generateRandom();

        static UUID Empty() { return {0, 0, 0, 0}; }
        UUID(uint32_t a, uint32_t b, uint32_t c, uint32_t d, UUIDType type = UUIDType::Generic) : a(a), b(b), c(c), d(d), type(type) {};
        UUID(UUIDType type);
        UUID();
        explicit UUID(const protoMesh::crypto::UUID* id);

        vector<uint8_t> toVector() const;

        explicit operator string() const;
        inline tuple<uint32_t, uint32_t, uint32_t, uint32_t> tie() const { return std::tie(a, b, c, d); }
        inline bool operator==(const UUID &other) const { return this->tie() == other.tie() && this->type == other.type; }
        inline bool operator!=(const UUID &other) const { return this->type != other.type || (this->tie() != other.tie() && this->type == other.type); }
//        inline bool operator>(const UUID &other) const { return this->tie() > other.tie(); }
//        inline bool operator<(const UUID &other) const { return this->tie() < other.tie(); }
    };
    inline std::ostream& operator<< (std::ostream &out, const UUID &uid) { out << string(uid); return out; }

    namespace hash {
        string sha512(vector<uint8_t> message);
        HASH sha512Vec(vector<uint8_t> message);
    }

    namespace serialize {
        string uint8ArrToString(uint8_t* arr, unsigned long len);
        vector<uint8_t> stringToUint8Array(string hex);
    }

    namespace sym {
        struct AESError {
            enum class Kind { IVTooSmall };
            Kind kind;
            std::string text;
            AESError(Kind kind, std::string text) : kind(kind), text(std::move(text)) {}
        };

        Result<vector<uint8_t>, AESError> encrypt(vector<uint8_t> text, vector<uint8_t> key, vector<uint8_t> iv);
        vector<uint8_t> decrypt(vector<uint8_t> ciphertext, vector<uint8_t> key);
    }

    namespace asym {
        bool verifyKeySize();


        struct PublicKeyDeserializationError {
            enum class Kind { KeySizeMismatch };
            Kind kind;
            std::string text;
            PublicKeyDeserializationError(Kind kind, std::string text) : kind(kind), text(std::move(text)) {}
        };

        struct PublicKey {
        public:
            array<uint8_t, PUB_KEY_SIZE> raw;

            static Result<PublicKey, PublicKeyDeserializationError> fromBuffer(const flatbuffers::Vector<uint8_t>* buffer);

            explicit PublicKey(COMPRESSED_PUBLIC_KEY_T compressedKey);
            explicit PublicKey(uint8_t* publicKey);
            explicit PublicKey(string publicKey); // takes a hex representation of a compressed pub key

            string getCompressedString();
            COMPRESSED_PUBLIC_KEY_T getCompressed();
            PUB_HASH_T getHash();

            flatbuffers::Offset<protoMesh::crypto::PublicKey> toBuffer(flatbuffers::FlatBufferBuilder* builder);
        };

        struct KeyPair {
        public:
            PRIVATE_KEY_T priv;
            PublicKey pub;
            inline KeyPair(uint8_t* privKey, uint8_t* pubKey) : pub(pubKey) {
                copy(privKey, privKey + PRIV_KEY_SIZE, begin(this->priv));
            };
        };

        // TODO --important-- provide a platform unrelated PRNG
        inline KeyPair generateKeyPair() {
            // Generate two keys
            uint8_t privateKey[PRIV_KEY_SIZE] = {0};
            uint8_t publicKey[PUB_KEY_SIZE] = {0};
            uECC_make_key(publicKey, privateKey, ECC_CURVE);

            return {std::move(privateKey), std::move(publicKey)};
        }

        SIGNATURE_T sign(vector<uint8_t> text, PRIVATE_KEY_T privKey);
        bool verify(vector<uint8_t> text, SIGNATURE_T signature, PublicKey* pubKey);

        SHARED_KEY_T generateSharedSecret(PublicKey publicKey, PRIVATE_KEY_T privateKey);
    }
}


#endif //PROTOMESH_CRYPTO_HPP