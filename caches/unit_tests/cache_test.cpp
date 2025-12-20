#include <gtest/gtest.h>
#include "../CacheImp.h"
#include <random>

class CacheTest : public ::testing::Test {
protected:
    void SetUp() override {
    }

    void TearDown() override {
    }
};

TEST_F(CacheTest, PutAndGet) {
    ExampleCache<int, int> cache;
    EXPECT_EQ(cache.size(), 0);
}

TEST_F(CacheTest, PutAndGetExtend) {
    ExampleCache<int, std::string> cache(3);
    
    cache.set(1, "one");
    cache.set(2, "two");
    
    EXPECT_EQ(cache.size(), 2);
    EXPECT_EQ(cache.get(1), "one");
    EXPECT_EQ(cache.get(2), "two");
    
    cache.set(1, "updated_one");
    EXPECT_EQ(cache.size(), 2);
    EXPECT_EQ(cache.get(1), "updated_one");
}

TEST_F(CacheTest, ClockSurvivalWork) {
    ExampleCache<int, int> cache(3);

    cache.set(1, 10);
    cache.set(2, 20);
    cache.set(3, 30);

    cache.set(4, 40); 
    cache.get(2); // даем второй шанс ключу 2, мы его трогаем, и поэтому выселен будет не он
    cache.set(5, 50);

    EXPECT_TRUE(cache.contains(2));
    EXPECT_TRUE(cache.contains(4));
    EXPECT_TRUE(cache.contains(5));
    EXPECT_FALSE(cache.contains(3));
    EXPECT_FALSE(cache.contains(1));
}

TEST_F(CacheTest, SurvivalScenarios) {
    ExampleCache<int, int> cache(3);
    cache.set(1, 10);
    cache.set(2, 20);
    cache.set(3, 30);

    cache.get(1);
    cache.get(2);

    cache.set(4, 40); 
    EXPECT_TRUE(cache.contains(1));
    EXPECT_TRUE(cache.contains(2));
    EXPECT_FALSE(cache.contains(3));
    EXPECT_TRUE(cache.contains(4));
}

TEST_F(CacheTest, ClearAndRestart) {
    ExampleCache<int, int> cache(2);
    cache.set(1, 10);
    cache.set(2, 20);
    cache.clear();
    
    EXPECT_EQ(cache.size(), 0);
    EXPECT_FALSE(cache.contains(1));
    
    EXPECT_NO_THROW(cache.set(3, 30));
    EXPECT_EQ(cache.size(), 1);
    EXPECT_EQ(cache.get(3), 30);
}

/* нагрузочные тест, что кеш не ломается со временем из-за накопившихся ошибок
Всегда корректно работает clear(), размер кеша, сохраняется работоспособность */
TEST_F(CacheTest, HeavyLoadStressTest) {
    const std::size_t cache_capacity = 100;
    const int num_operations = 100000;
    ExampleCache<int, int> cache(cache_capacity);

    std::mt19937 gen(42);
    std::uniform_int_distribution<> key_dist(1, 500);
    std::uniform_int_distribution<> val_dist(1, 10000);

    for (int i = 0; i < num_operations; ++i) {
        int key = key_dist(gen);
        int val = val_dist(gen);

        if (i % 2 == 0) {
            cache.set(key, val);
        } else {
            if (cache.contains(key)) {
                int retrieved = cache.get(key);
            }
        }

        ASSERT_LE(cache.size(), cache_capacity);

        if (i == num_operations / 2) {
            cache.clear();
            ASSERT_EQ(cache.size(), 0);
        }
    }

    cache.set(999, 999);
    EXPECT_TRUE(cache.contains(999));
    EXPECT_EQ(cache.get(999), 999);
}