//
// Created by eliezer pearl on 7/9/18.
// Heavily based on iblt.cpp and iblt.h in https://github.com/mwcote/IBLT-Research.
//

#include "IBLT.h"

IBLT::IBLT(size_t expectedNumEntries)
{
    // 3x expectedNumEntries gives very low probability of decoding failure
    size_t nEntries = expectedNumEntries + expectedNumEntries/2;
    // ... make nEntries exactly divisible by N_HASH
    while (N_HASH * (nEntries/N_HASH) != nEntries) ++nEntries;
    hashTable.resize(nEntries);
}

IBLT::~IBLT() = default;

hashVal IBLT::_hash(const hashVal& initial, long kk) {
    if(kk == -1) return initial;
    std::hash<std::string> shash;
    return _hash(shash(toStr(initial)), kk-1);
}

hashVal IBLT::hashK(const ZZ& item, long kk) {
    std::hash<std::string> shash; // stl uses MurmurHashUnaligned2
    return _hash(shash(toStr(item)), kk-1);
}

void IBLT::_insert(long plusOrMinus, ZZ key, ZZ value) {
    long bucketsPerHash = hashTable.size() / N_HASH;

    for(int ii=0; ii < N_HASH; ii++){
        hashVal hk = hashK(key, ii);
        long startEntry = ii * bucketsPerHash;
        int loc = startEntry + (hk%bucketsPerHash);
        IBLT::HashTableEntry& entry = hashTable.at(startEntry + (hk%bucketsPerHash));

        entry.count += plusOrMinus;
        entry.keySum ^= key;
        entry.keyCheck ^= hashK(key, N_HASHCHECK);
        if (entry.empty()) {
            entry.valueSum.kill();
        }
        else {
            entry.valueSum ^= value;
        }
    }
}

void IBLT::insert(ZZ key, ZZ value)
{
    _insert(1, key, value);
}

void IBLT::erase(ZZ key, ZZ value)
{
    //Logger::gLog(Logger::METHOD, std::string("entering: erase"));
    _insert(-1, key, value);
}

bool IBLT::get(ZZ key, ZZ& result){
    long bucketsPerHash = hashTable.size()/N_HASH;
    for (long ii = 0; ii < N_HASH; ii++) {
        long startEntry = ii*bucketsPerHash;
        unsigned long hk = hashK(key, ii);
        const IBLT::HashTableEntry& entry = hashTable[startEntry + (hk%bucketsPerHash)];

        if (entry.empty()) {
            // Definitely not in table. Leave
            // result empty, return true.

            return true;
        }
        else if (entry.isPure()) {
            if (entry.keySum == key) {
                // Found!
                result = entry.valueSum;
                return true;
            }
            else {
                // Definitely not in table
                result.kill();
                return true;
            }
        }
    }

    // Don't know if k is in table or not; "peel" the IBLT to try to find it:
    long nErased;
    do {
        nErased = 0;
        for (IBLT::HashTableEntry &entry : this->hashTable) {
            if (entry.isPure()) {
                if (entry.keySum == key) {
                    string s = toStr(entry.valueSum);
                    result = entry.valueSum;
                    return true;
                }
                nErased++;
                this->_insert(-entry.count, entry.keySum, entry.valueSum);
            }
        }
    } while (nErased > 0);
    return false;
}

bool IBLT::HashTableEntry::isPure() const
{
    if (count == 1 || count == -1) {
        hashVal check = hashK(keySum, N_HASHCHECK);
        return (keyCheck == check);
    }
    return false;
}

bool IBLT::HashTableEntry::empty() const
{
    return (count == 0 && IsZero(keySum) && keyCheck == 0);
}

// For debugging during development:
//string IBLT::DumpTable() const
//{
//    stringstream result;
//
//    result << "count keySum keyCheckMatch\n";
//    for(const IBLT::HashTableEntry& entry : hashTable) {
//        result << entry.count << " " << entry.keySum << " ";
//        result << (hashK(entry.keySum, N_HASHCHECK) == entry.keyCheck ? "true" : "false");
//        result << "\n";
//    }
//
//    return result.str();
//}

bool IBLT::listEntries(vector<pair<ZZ, ZZ>> &positive, vector<pair<ZZ, ZZ>> &negative){
    long nErased = 0;
    do {
        nErased = 0;
        for(IBLT::HashTableEntry& entry : this->hashTable) {
            if (entry.isPure()) {
                if (entry.count == 1) {
                    positive.emplace_back(std::make_pair(entry.keySum, entry.valueSum));
                }
                else {
                    negative.emplace_back(std::make_pair(entry.keySum, entry.valueSum));
                }
                this->_insert(-entry.count, entry.keySum, entry.valueSum);
                ++nErased;
            }
        }
    } while (nErased > 0);

    // If any buckets for one of the hash functions is not empty,
    // then we didn't peel them all:
    for (IBLT::HashTableEntry& entry : this->hashTable) {
        if (!entry.empty()) return false;
    }
    return true;
}

IBLT& IBLT::operator-=(const IBLT& other) {
    for (unsigned long ii = 0; ii < hashTable.size(); ii++) {
        IBLT::HashTableEntry& e1 = this->hashTable.at(ii);
        const IBLT::HashTableEntry& e2 = other.hashTable.at(ii);
        e1.count -= e2.count;
        e1.keySum ^= e2.keySum;
        e1.keyCheck ^= e2.keyCheck;
        if (e1.empty()) {
            e1.valueSum.kill();
        }
        else {
            e1.valueSum ^= e2.valueSum;
        }
    }
    return *this;
}

IBLT IBLT::operator-(const IBLT& other) const {
    IBLT result(*this);
    return result -= other;
}

size_t IBLT::size() {
    return hashTable.size();
}