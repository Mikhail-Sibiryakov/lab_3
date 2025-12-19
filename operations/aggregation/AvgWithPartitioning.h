#pragma once
#include <vector>
#include <unordered_map>
#include <functional>

template<typename Value_t>
struct Info {
    Value_t sum = 0;
    std::size_t cnt = 0;
};

template<typename Key_t, typename Value_t>
class Partition {
  public:
    Partition(std::size_t block_size) : block_size(block_size) {}

    void add_item(const std::pair<Key_t, Value_t>& item) {
        if (data.empty() || data.back().size() == block_size) {
            data.push_back({item});
        } else {
            data.back().push_back(item);
        }
    }

    void load_partition_into_map(std::unordered_map<Key_t, Info<Value_t>>& mp) const {
        for (const auto& bucket : data) {
            for (const auto& entry : bucket) {
                mp[entry.first].sum += entry.second;
                ++mp[entry.first].cnt;
            }
        }
    }

  private:
    std::size_t block_size;
    std::vector<std::vector<std::pair<Key_t, Value_t>>> data;
};

// Caclculate average value for each key.
template<typename Key_t, typename Value_t>
std::vector<std::pair<Key_t, Value_t>> AvgWithPartitioning(const std::vector<std::pair<Key_t, Value_t>>& kv_array
    , std::size_t block_size = 8)
{
    // кол-во партиций
    std::size_t number_partition = (kv_array.size() + block_size - 1) / block_size;

    std::vector<Partition<Key_t, Value_t>> parts(number_partition,
                                                 Partition<Key_t, Value_t>(block_size));
    std::hash<Key_t> hasher;
    // заполняем все партиции из входных данных
    for (const auto& entry : kv_array) {
        std::size_t ind = (hasher(entry.first) % number_partition);
        parts[ind].add_item(entry);
    }

    std::vector<std::pair<Key_t, Value_t>> res;
    std::unordered_map<Key_t, Info<Value_t>> mp;
    for (const auto& part : parts) {
        part.load_partition_into_map(mp);
        res.reserve(res.size() + mp.size());
        for (const auto& [key, info] : mp) {
            res.emplace_back(key, info.sum / static_cast<Value_t>(info.cnt));
        }
        mp.clear();
    }

    return res;
}
