#ifndef OPEN_HOME_REGISTRY_HPP
#define OPEN_HOME_REGISTRY_HPP

#include <string>
#include <vector>
#include <map>
#include "RegistryEntry.hpp"
#include "../const.hpp"
#include "../crypto/crypto.hpp"
#include "../api/storage.hpp"
#include "../api/network.hpp"
#include "../api/time.hpp"
#include <random>

using namespace std;

class Registry {
    // Variables
    StorageProvider* stor;
    NetworkProvider *net;
    REL_TIME_PROV_T relTimeProvider;

    BCAST_SOCKET_T bcast;
    long nextBroadcast;

    string name;
    UUID instanceIdentifier;

    map<string, string> headState;

    struct {
        long lastRequestTimestamp;
        UUID requestID;
        size_t min;
        size_t max;
        UUID communicationTarget;
    } synchronizationStatus;

    // Functions
    void updateHead(bool save);
    bool addEntry(RegistryEntry e, bool save = true);

    string getHeadUUID();

    tuple<vector<unsigned long>, unsigned long> getBlockBorders(string parentUUID = "");

    string requestHash(size_t index, string target, UUID requestID);
    void onBinarySearchResult(size_t index);
    void broadcastEntries(size_t index);
    bool isSyncInProgress();

public:
    Registry(string name, StorageProvider *stor, NetworkProvider *net,
             REL_TIME_PROV_T relTimeProvider);

    string get(string key);
    void set(string key, string value, Crypto::asym::KeyPair pair);
    void del(string key, Crypto::asym::KeyPair pair);
    bool has(string key);

    bool addSerializedEntry(string serialized, bool save = true);

    vector<string> hashChain;
    string getHeadHash() const;

    void clear();

    void sync();

    void onData(string incomingData);

    inline void print() {
        cout << this->instanceIdentifier << " | " << this->entries.size() << endl;
        for (unsigned int i = 0; i < this->entries.size(); i++) {
            cout << string(this->entries[i]) << endl;
        }
//        for (auto &entry : this->entries) cout << string(entry) << endl;
    };
    vector<RegistryEntry> entries;
};


#endif //OPEN_HOME_REGISTRY_HPP