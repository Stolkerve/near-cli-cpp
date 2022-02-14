#include "KeyPair.h"

#include <sodium/crypto_sign_ed25519.h>
#include <sodium/crypto_scalarmult_curve25519.h>

#include "utils.h"

KeyPair::KeyPair() {

}

void KeyPair::SignRandomEd25519() {
    std::vector<uint8_t> ed25519_pk(crypto_sign_ed25519_PUBLICKEYBYTES);
    std::vector<uint8_t> ed25519_skpk(crypto_sign_ed25519_SECRETKEYBYTES);
    crypto_sign_ed25519_keypair(ed25519_pk.data(), ed25519_skpk.data());

    publicKey = "ed25519:";
    privateKey = "ed25519:";

    publicKey.append(EncodeBase58(ed25519_pk));
    privateKey.append(EncodeBase58(ed25519_skpk));
}

void KeyPair::SignEd25519(const std::string& ed25519Pk, const std::string& ed25519Spk) {
    publicKey = ed25519Pk;
    privateKey = ed25519Spk;
}

