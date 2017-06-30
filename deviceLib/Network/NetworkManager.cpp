#include "NetworkManager.hpp"

Network NetworkManager::joinNetwork(string id, NETWORK_KEY_T key)  {
    string serializedKey(Crypto::serialize::uint8ArrToString(&key[0], NETWORK_KEY_SIZE));
    this->stor->set("networkKey::" + id, serializedKey);
    this->stor->set("lastJoinedNetwork", id);

    // TODO Mask this->net with the encryption

    cout << "Joining network: " << serializedKey << endl;

    Crypto::asym::PublicKey masterKey(key);

    return Network(this->net, this->stor, this->time, masterKey);
}

bool NetworkManager::lastJoinedAvailable()  {
    return this->stor->has("lastJoinedNetwork");
}

Network NetworkManager::joinLastNetwork()  {
    string lastNetworkID(this->stor->get("lastJoinedNetwork"));
    string serializedKey(this->stor->get("networkKey::" + lastNetworkID));

    vector<uint8_t> deserializedKey(Crypto::serialize::stringToUint8Array(serializedKey));
    array<uint8_t, 33> lastNetworkKey;
    copy(&deserializedKey[0], &deserializedKey[0] + NETWORK_KEY_SIZE, begin(lastNetworkKey));
    return this->joinNetwork(lastNetworkID, lastNetworkKey);
}
