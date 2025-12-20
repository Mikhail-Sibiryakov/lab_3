#pragma once
#include "ICache.h"
#include <unordered_map>
#include <vector>
#include <algorithm>

template<typename Key, typename Value>
struct Item {
    Key key;
    Value val;
    int count = 0;
};

template<typename Key, typename Value>
class ExampleCache : public ICache<Key, Value> {
public:
    ExampleCache(std::size_t sz_storage = 5) : max_size_storage(sz_storage) {}
    
    void set(const Key& key, const Value& value) override {
        auto it = mp.find(key);

        // обновить знач, если ключ был
        if (it != mp.end()) {
            storage[it->second].val = value;
            storage[it->second].count = std::min(storage[it->second].count + 1, max_count);
            return;
        }
        
        // если в кеше есть свободные места
        if (mp.size() < max_size_storage) {
            mp[key] = storage.size();
            storage.push_back({key, value, 1});
            return;
        }

        // ищем жертву для выселения
        while (storage[ptr].count != 0) {
            --storage[ptr].count;
            ptr = (ptr + 1) % storage.size();
        }
        mp.erase(storage[ptr].key);
        storage[ptr] = {key, value, 1};
        mp[key] = ptr;
        ptr = (ptr + 1) % storage.size();
    }
    
    Value get(const Key& key) override {
        auto it = mp.find(key);
        // Тут надо делать что-нибудь, если ключа нет (ну или если такого не бывает, то и ладно)
        if (it == mp.end()) {
            return Value();
        }
        storage[it->second].count = std::min(storage[it->second].count + 1, max_count);
        return storage[it->second].val;
    }
    
    bool contains(const Key& key) const override {
        auto it = mp.find(key);
        bool res = it != mp.end();
        if (res) {
            storage[it->second].count = std::min(storage[it->second].count + 1, max_count);
        }
        return res;
    }
    
    void clear() override {
        mp.clear();
        storage.clear();
        ptr = 0;
    }
    
    size_t size() const override {
        return mp.size();
    }

private:
    mutable std::vector<Item<Key, Value>> storage;
    std::unordered_map<Key, std::size_t> mp;
    std::size_t ptr = 0;
    std::size_t max_size_storage;
    const int max_count = 5;
};
