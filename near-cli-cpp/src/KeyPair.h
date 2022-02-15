#ifndef KEY_PAIR_H
#define KEY_PAIR_H

#include <string>
#include <vector>

class KeyPair {
public:
    KeyPair();

    /**
     * Create a random keypair.
     *
     * The keypair is base on Ed25519 and encode in base58
     */
    void SignRandomEd25519();

    /**
     * Store a keypair Ed25519 encode in base58
     * 
     * @param ed25519Pk Public key
     * @param ed25519Spk Private key
     */
    void SignEd25519(const std::string& ed25519Pk, const std::string& ed25519Spk);

    const std::string& GetPublicKey() const { return m_PublicKey; }
    const std::string& GetPrivateKey() const { return m_PrivateKey; }
private:
    std::string m_PublicKey = "";
    std::string m_PrivateKey = "";
};

#endif