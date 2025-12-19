#include <gtest/gtest.h>
#include <utility>
#include <vector>
#include "../AvgWithPartitioning.h"

class CacheTest : public ::testing::Test {
protected:
    void SetUp() override {
    }

    void TearDown() override {
    }
};

TEST_F(CacheTest, Simple) {
    std::vector<std::pair<int, int>> tmp;
    auto res = AvgWithPartitioning<int, int>(tmp).size();
    EXPECT_EQ(res, 0);
}

/*TODO*/

// Тест на один ключ
TEST_F(CacheTest, SingleKeyAverage) {
    std::vector<std::pair<int, double>> tmp = {
        {1, 10.0}, {1, 20.0}, {1, 30.0}
    };
    auto res = AvgWithPartitioning<int, double>(tmp, 2); 
    
    ASSERT_EQ(res.size(), 1);
    EXPECT_EQ(res[0].first, 1);
    EXPECT_DOUBLE_EQ(res[0].second, 20.0);
}

// Тест на несколько ключей и маленькие блоки (проверка логики создания новых блоков)
TEST_F(CacheTest, MultipleKeysSmallBlocks) {
    std::vector<std::pair<std::string, float>> tmp = {
        {"A", 10.0f}, {"B", 20.0f}, {"A", 20.0f}, 
        {"C", 5.0f},  {"B", 40.0f}, {"C", 15.0f}
    };
    
    auto res = AvgWithPartitioning<std::string, float>(tmp, 1);

    std::map<std::string, float> final_map;
    for(const auto& p : res) final_map[p.first] = p.second;

    EXPECT_EQ(final_map.size(), 3);
    EXPECT_FLOAT_EQ(final_map["A"], 15.0f); // (10+20)/2
    EXPECT_FLOAT_EQ(final_map["B"], 30.0f); // (20+40)/2
    EXPECT_FLOAT_EQ(final_map["C"], 10.0f); // (5+15)/2
}

/* Большой стресс тест, тут ключи от 0 до 9 и значения у них просто числа с шагом в 10
Получается, для каждого ключа своя арифметическая прогрессия и поэтому можно довольно
легко проверить парвильность работы (среднее арифм. прогресси считаем по формуле)*/
TEST_F(CacheTest, StressTestDetailed) {
    std::vector<std::pair<int, double>> tmp;
    const int n = 1000;
    const int num_keys = 10;
    
    for (int i = 0; i < n; ++i) {
        tmp.push_back({i % num_keys, static_cast<double>(i)});
    }

    auto res = AvgWithPartitioning<int, double>(tmp);
    
    EXPECT_EQ(res.size(), num_keys);

    std::map<int, double> results;
    for (const auto& p : res) {
        results[p.first] = p.second;
    }

    for (int k = 0; k < num_keys; ++k) {
        double expected_avg = (k + (k + 990.0)) / 2.0;
        
        ASSERT_TRUE(results.count(k));
        EXPECT_DOUBLE_EQ(results[k], expected_avg);
    }
}

// Если будет только один ключ, но много записей
TEST_F(CacheTest, AllKeysSame) {
    std::vector<std::pair<int, double>> tmp;
    for(int i = 0; i < 100; ++i) tmp.push_back({42, 10.0});
    
    auto res = AvgWithPartitioning<int, double>(tmp, 5);
    
    ASSERT_EQ(res.size(), 1);
    EXPECT_EQ(res[0].first, 42);
    EXPECT_DOUBLE_EQ(res[0].second, 10.0);
}