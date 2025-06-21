#include <iostream>
#include <unordered_map>
#include <random>
#include <chrono>
#include <cassert>
#include <string>
#include <memory>
#include <vector>
#include <algorithm>
#include "Map.hpp"

class UnorderedMapTester {
private:
    std::mt19937 rng;
    std::uniform_int_distribution<int> key_dist;
    std::uniform_int_distribution<int> value_dist;
    
public:
    UnorderedMapTester() : rng(std::chrono::steady_clock::now().time_since_epoch().count()),
                           key_dist(1, 10000),
                           value_dist(1, 1000) {}
    
    void testBasicConstruction() {
        std::cout << "Testing basic construction..." << std::endl;
        
        // Test default construction
        UnorderedMap<int, std::string> map1;
        assert(map1.empty());
        assert(map1.size() == 0);
        assert(map1.capacity() >= 16); // Minimum capacity
        
        // Test construction with initial capacity
        UnorderedMap<int, std::string> map2(32);
        assert(map2.empty());
        assert(map2.capacity() == 32);
        
        // Test with different types
        UnorderedMap<std::string, int> map3;
        UnorderedMap<double, double> map4;
        
        assert(map3.empty() && map4.empty());
        
        std::cout << "Basic construction: PASSED" << std::endl;
    }
    
    void testInsertAndFind() {
        std::cout << "Testing insert and find operations..." << std::endl;
        
        UnorderedMap<int, std::string> map;
        
        // Test basic insert
        map.insert(1, "one");
        map.insert(2, "two");
        map.insert(3, "three");
        
        assert(map.size() == 3);
        assert(!map.empty());
        
        // Test find
        std::string* found = map.find(1);
        assert(found != nullptr);
        assert(*found == "one");
        
        found = map.find(2);
        assert(found != nullptr);
        assert(*found == "two");
        
        // Test find non-existent key
        found = map.find(999);
        assert(found == nullptr);
        
        // Test contains
        assert(map.contains(1));
        assert(map.contains(2));
        assert(map.contains(3));
        assert(!map.contains(999));
        
        std::cout << "Insert and find: PASSED" << std::endl;
    }
    
    void testOperatorAccess() {
        std::cout << "Testing operator[] and at()..." << std::endl;
        
        UnorderedMap<std::string, int> map;
        
        // Test operator[] for insertion
        map["hello"] = 100;
        map["world"] = 200;
        map["test"] = 300;
        
        assert(map.size() == 3);
        
        // Test operator[] for access
        assert(map["hello"] == 100);
        assert(map["world"] == 200);
        assert(map["test"] == 300);
        
        // Test operator[] creates default value
        int value = map["new_key"];
        assert(value == 0); // Default int value
        assert(map.size() == 4);
        
        // Test at() method
        assert(map.at("hello") == 100);
        assert(map.at("world") == 200);
        
        std::cout << "Operator access: PASSED" << std::endl;
    }
    
    void testMoveSemantics() {
        std::cout << "Testing move semantics..." << std::endl;
        
        UnorderedMap<int, std::string> map1;
        map1.insert(1, "one");
        map1.insert(2, "two");
        map1.insert(3, "three");
        
        size_t original_size = map1.size();
        size_t original_capacity = map1.capacity();
        
        // Test move constructor
        UnorderedMap<int, std::string> map2(std::move(map1));
        
        assert(map2.size() == original_size);
        assert(map2.capacity() == original_capacity);
        assert(map1.empty()); // Original should be empty
        assert(map1.capacity() == 0);
        
        // Verify data integrity
        assert(*map2.find(1) == "one");
        assert(*map2.find(2) == "two");
        assert(*map2.find(3) == "three");
        
        // Test move assignment
        UnorderedMap<int, std::string> map3;
        map3.insert(99, "ninety-nine");
        
        map3 = std::move(map2);
        
        assert(map3.size() == original_size);
        assert(map2.empty()); // map2 should be empty after move
        assert(*map3.find(1) == "one");
        
        std::cout << "Move semantics: PASSED" << std::endl;
    }
    
    void testCopySemantics() {
        std::cout << "Testing copy semantics..." << std::endl;
        
        UnorderedMap<int, std::string> map1;
        map1.insert(10, "ten");
        map1.insert(20, "twenty");
        map1.insert(30, "thirty");
        
        // Test copy constructor
        UnorderedMap<int, std::string> map2(map1);
        
        assert(map2.size() == map1.size());
        assert(*map2.find(10) == "ten");
        assert(*map2.find(20) == "twenty");
        assert(*map2.find(30) == "thirty");
        
        // Verify independence
        map2.insert(40, "forty");
        assert(map2.size() == 4);
        assert(map1.size() == 3);
        assert(map1.find(40) == nullptr);
        
        // Test copy assignment
        UnorderedMap<int, std::string> map3;
        map3.insert(99, "ninety-nine");
        
        map3 = map1;
        assert(map3.size() == map1.size());
        assert(*map3.find(10) == "ten");
        
        std::cout << "Copy semantics: PASSED" << std::endl;
    }
    
    void testEmplace() {
        std::cout << "Testing emplace operation..." << std::endl;
        
        UnorderedMap<int, std::string> map;
        
        // Test emplace with string construction
        map.emplace(1, "Hello");
        map.emplace(2, "World");
        map.emplace(3, 5, 'A'); // String with 5 'A's
        
        assert(map.size() == 3);
        assert(*map.find(1) == "Hello");
        assert(*map.find(2) == "World");
        assert(*map.find(3) == "AAAAA");
        
        // Test emplace with complex types
        UnorderedMap<std::string, std::vector<int>> vec_map;
        vec_map.emplace("numbers", 5, 42); // Vector with 5 elements of value 42
        
        assert(vec_map.size() == 1);
        auto* vec = vec_map.find("numbers");
        assert(vec != nullptr);
        assert(vec->size() == 5);
        assert((*vec)[0] == 42);
        
        std::cout << "Emplace: PASSED" << std::endl;
    }
    
    void testErase() {
        std::cout << "Testing erase operation..." << std::endl;
        
        UnorderedMap<int, std::string> map;
        
        // Insert test data
        for (int i = 1; i <= 10; ++i) {
            map.insert(i, "value" + std::to_string(i));
        }
        
        assert(map.size() == 10);
        
        // Test successful erase
        bool erased = map.erase(5);
        assert(erased == true);
        assert(map.size() == 9);
        assert(map.find(5) == nullptr);
        assert(!map.contains(5));
        
        // Test erase non-existent key
        erased = map.erase(999);
        assert(erased == false);
        assert(map.size() == 9);
        
        // Test erase first and last elements
        erased = map.erase(1);
        assert(erased == true);
        assert(map.size() == 8);
        
        erased = map.erase(10);
        assert(erased == true);
        assert(map.size() == 7);
        
        // Verify remaining elements
        for (int i = 2; i <= 9; ++i) {
            if (i != 5) { // 5 was already erased
                assert(map.contains(i));
            }
        }
        
        std::cout << "Erase: PASSED" << std::endl;
    }
    
    void testClear() {
        std::cout << "Testing clear operation..." << std::endl;
        
        UnorderedMap<int, std::string> map;
        
        // Insert many elements
        for (int i = 0; i < 100; ++i) {
            map.insert(i, "value" + std::to_string(i));
        }
        
        assert(map.size() == 100);
        assert(!map.empty());
        
        size_t old_capacity = map.capacity();
        map.clear();
        
        assert(map.size() == 0);
        assert(map.empty());
        assert(map.capacity() == old_capacity); // Capacity should be preserved
        
        // Should be able to use map after clear
        map.insert(42, "forty-two");
        assert(map.size() == 1);
        assert(*map.find(42) == "forty-two");
        
        std::cout << "Clear: PASSED" << std::endl;
    }
    
    void testRehashing() {
        std::cout << "Testing automatic rehashing..." << std::endl;
        
        UnorderedMap<int, int> map(16); // Start with small capacity
        
        size_t initial_capacity = map.capacity();
        assert(initial_capacity == 16);
        
        // Insert elements to trigger rehashing
        // Load factor threshold is 0.75, so 16 * 0.75 = 12 elements should trigger rehash
        for (int i = 0; i < 20; ++i) {
            map.insert(i, i * 10);
        }
        
        assert(map.size() == 20);
        assert(map.capacity() > initial_capacity); // Should have grown
        
        // Verify all elements are still accessible
        for (int i = 0; i < 20; ++i) {
            int* found = map.find(i);
            assert(found != nullptr);
            assert(*found == i * 10);
        }
        
        // Check load factor
        assert(map.load_factor() <= 0.75f);
        
        std::cout << "Rehashing: PASSED" << std::endl;
    }
    
    void testGetAllPairs() {
        std::cout << "Testing get_all_pairs()..." << std::endl;
        
        UnorderedMap<std::string, int> map;
        
        map.insert("apple", 1);
        map.insert("banana", 2);
        map.insert("cherry", 3);
        map.insert("date", 4);
        
        auto pairs = map.get_all_pairs();
        assert(pairs.size() == 4);
        
        // Verify all pairs are present (order may vary)
        bool found_apple = false, found_banana = false, found_cherry = false, found_date = false;
        
        for (const auto& pair : pairs) {
            if (pair.key == "apple" && pair.value == 1) found_apple = true;
            else if (pair.key == "banana" && pair.value == 2) found_banana = true;
            else if (pair.key == "cherry" && pair.value == 3) found_cherry = true;
            else if (pair.key == "date" && pair.value == 4) found_date = true;
        }
        
        assert(found_apple && found_banana && found_cherry && found_date);
        
        std::cout << "Get all pairs: PASSED" << std::endl;
    }
    
    void testCollisionHandling() {
        std::cout << "Testing collision handling..." << std::endl;
        
        // Create a map with small capacity to force collisions
        UnorderedMap<int, std::string> map(8);
        
        // Insert keys that are likely to collide
        std::vector<int> keys = {8, 16, 24, 32, 40}; // These should hash to same bucket
        
        for (int key : keys) {
            map.insert(key, "value" + std::to_string(key));
        }
        
        assert(map.size() == keys.size());
        
        // Verify all keys can be found despite collisions
        for (int key : keys) {
            std::string* found = map.find(key);
            assert(found != nullptr);
            assert(*found == "value" + std::to_string(key));
        }
        
        // Test erase with collisions
        bool erased = map.erase(16);
        assert(erased == true);
        assert(map.size() == keys.size() - 1);
        
        // Verify other keys are still accessible
        for (int key : keys) {
            if (key != 16) {
                assert(map.contains(key));
            }
        }
        
        std::cout << "Collision handling: PASSED" << std::endl;
    }
    
    void testStressOperations(int iterations = 10000) {
        std::cout << "Testing stress operations (" << iterations << " iterations)..." << std::endl;
        
        UnorderedMap<int, int> map;
        std::unordered_map<int, int> reference;
        
        for (int i = 0; i < iterations; ++i) {
            int operation = key_dist(rng) % 4;
            int key = key_dist(rng);
            int value = value_dist(rng);
            
            if (operation == 0) {
                // Insert operation
                map.insert(key, value);
                reference[key] = value;
            } else if (operation == 1) {
                // Find operation
                int* found = map.find(key);
                auto ref_it = reference.find(key);
                
                if (ref_it != reference.end()) {
                    assert(found != nullptr);
                    assert(*found == ref_it->second);
                } else {
                    assert(found == nullptr);
                }
            } else if (operation == 2) {
                // Erase operation
                bool erased = map.erase(key);
                size_t ref_erased = reference.erase(key);
                assert(erased == (ref_erased > 0));
            } else {
                // Contains operation
                bool contains = map.contains(key);
                bool ref_contains = reference.find(key) != reference.end();
                assert(contains == ref_contains);
            }
            
            // Verify size consistency
            assert(map.size() == reference.size());
            assert(map.empty() == reference.empty());
        }
        
        std::cout << "Stress operations: PASSED" << std::endl;
    }
    
    void performanceTest() {
        std::cout << "\n=== PERFORMANCE TESTING ===" << std::endl;
        
        const size_t iterations = 1000000;
        
        // Test insert performance
        auto start = std::chrono::high_resolution_clock::now();
        
        UnorderedMap<int, int> map;
        for (size_t i = 0; i < iterations; ++i) {
            map.insert(static_cast<int>(i), static_cast<int>(i * 2));
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto insert_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        // Test find performance
        start = std::chrono::high_resolution_clock::now();
        
        volatile int sum = 0;
        for (size_t i = 0; i < iterations; ++i) {
            int* found = map.find(static_cast<int>(i));
            if (found) sum += *found;
        }
        
        end = std::chrono::high_resolution_clock::now();
        auto find_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        // Compare with std::unordered_map
        start = std::chrono::high_resolution_clock::now();
        std::unordered_map<int, int> std_map;
        for (size_t i = 0; i < iterations; ++i) {
            std_map[static_cast<int>(i)] = static_cast<int>(i * 2);
        }
        end = std::chrono::high_resolution_clock::now();
        auto std_insert_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        std::cout << "Insert performance: " << insert_time.count() << " μs (" << iterations << " operations)" << std::endl;
        std::cout << "Find performance: " << find_time.count() << " μs (" << iterations << " operations)" << std::endl;
        std::cout << "std::unordered_map insert: " << std_insert_time.count() << " μs" << std::endl;
        std::cout << "Performance ratio: " << (double)std_insert_time.count() / insert_time.count() << std::endl;
        
        std::cout << "Average insert: " << (double)insert_time.count() / iterations << " μs" << std::endl;
        std::cout << "Average find: " << (double)find_time.count() / iterations << " μs" << std::endl;
        std::cout << "Sum (prevent optimization): " << sum << std::endl;
    }
    
    void runAllTests() {
        std::cout << "=== UNORDERED MAP COMPREHENSIVE TESTING ===" << std::endl;
        std::cout << "Seed: " << rng() << std::endl << std::endl;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        testBasicConstruction();
        testInsertAndFind();
        testOperatorAccess();
        testMoveSemantics();
        testCopySemantics();
        testEmplace();
        testErase();
        testClear();
        testRehashing();
        testGetAllPairs();
        testCollisionHandling();
        testStressOperations();
        performanceTest();
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << std::endl << "=== ALL TESTS COMPLETED ===" << std::endl;
        std::cout << "Total time: " << duration.count() << "ms" << std::endl;
    }
};

 
class UnorderedMapLookupTester {
private:
    std::mt19937 rng;
    std::uniform_int_distribution<int> key_dist;
    std::uniform_int_distribution<int> value_dist;
    
public:
    UnorderedMapLookupTester() : rng(std::chrono::steady_clock::now().time_since_epoch().count()),
                                 key_dist(1, 100000),
                                 value_dist(1, 1000) {}
    
    void testBasicFindOperations() {
        std::cout << "Testing basic find operations..." << std::endl;
        
        UnorderedMap<int, std::string> map;
        
        // Insert test data
        map.insert(1, "one");
        map.insert(2, "two");
        map.insert(3, "three");
        map.insert(100, "hundred");
        map.insert(999, "nine-nine-nine");
        
        // Test successful finds
        std::string* found = map.find(1);
        assert(found != nullptr);
        assert(*found == "one");
        
        found = map.find(2);
        assert(found != nullptr);
        assert(*found == "two");
        
        found = map.find(999);
        assert(found != nullptr);
        assert(*found == "nine-nine-nine");
        
        // Test unsuccessful finds
        found = map.find(404);
        assert(found == nullptr);
        
        found = map.find(-1);
        assert(found == nullptr);
        
        found = map.find(1000);
        assert(found == nullptr);
        
        std::cout << "Basic find operations: PASSED" << std::endl;
    }
    
    void testConstFindOperations() {
        std::cout << "Testing const find operations..." << std::endl;
        
        UnorderedMap<std::string, int> map;
        map.insert("apple", 1);
        map.insert("banana", 2);
        map.insert("cherry", 3);
        
        const UnorderedMap<std::string, int>& const_map = map;
        
        // Test const find
        const int* found = const_map.find("apple");
        assert(found != nullptr);
        assert(*found == 1);
        
        found = const_map.find("banana");
        assert(found != nullptr);
        assert(*found == 2);
        
        // Test const find with non-existent key
        found = const_map.find("orange");
        assert(found == nullptr);
        
        std::cout << "Const find operations: PASSED" << std::endl;
    }
    
    void testContainsOperations() {
        std::cout << "Testing contains operations..." << std::endl;
        
        UnorderedMap<int, double> map;
        
        // Insert some data
        for (int i = 0; i < 50; i += 5) {
            map.insert(i, i * 1.5);
        }
        
        // Test contains for existing keys
        for (int i = 0; i < 50; i += 5) {
            assert(map.contains(i));
        }
        
        // Test contains for non-existing keys
        for (int i = 1; i < 50; i += 5) {
            assert(!map.contains(i));
        }
        
        // Test edge cases
        assert(!map.contains(-1));
        assert(!map.contains(1000));
        assert(!map.contains(51));
        
        std::cout << "Contains operations: PASSED" << std::endl;
    }
    
    void testOperatorBracketAccess() {
        std::cout << "Testing operator[] access..." << std::endl;
        
        UnorderedMap<std::string, int> map;
        
        // Test insertion via operator[]
        map["first"] = 1;
        map["second"] = 2;
        map["third"] = 3;
        
        assert(map.size() == 3);
        
        // Test access via operator[]
        assert(map["first"] == 1);
        assert(map["second"] == 2);
        assert(map["third"] == 3);
        
        // Test default value creation
        int value = map["new_key"];
        assert(value == 0); // Default int value
        assert(map.size() == 4);
        assert(map.contains("new_key"));
        
        // Test modification via operator[]
        map["first"] = 100;
        assert(map["first"] == 100);
        
        std::cout << "Operator[] access: PASSED" << std::endl;
    }
    
    void testAtMethodAccess() {
        std::cout << "Testing at() method access..." << std::endl;
        
        UnorderedMap<int, std::string> map;
        map.insert(10, "ten");
        map.insert(20, "twenty");
        map.insert(30, "thirty");
        
        // Test successful at() calls
        assert(map.at(10) == "ten");
        assert(map.at(20) == "twenty");
        assert(map.at(30) == "thirty");
        
        // Test const at()
        const UnorderedMap<int, std::string>& const_map = map;
        assert(const_map.at(10) == "ten");
        assert(const_map.at(20) == "twenty");
        
        // Note: at() with non-existent key should trigger DEBUG_BREAK_IF
        // In production, this would throw an exception
        
        std::cout << "At() method access: PASSED" << std::endl;
    }
    
    void testLookupWithCollisions() {
        std::cout << "Testing lookup with hash collisions..." << std::endl;
        
        // Create a small map to force collisions
        UnorderedMap<int, std::string> map(8);
        
        // Insert keys that will likely collide
        std::vector<int> keys = {8, 16, 24, 32, 40, 48, 56, 64};
        
        for (int i = 0; i < keys.size(); ++i) {
            map.insert(keys[i], "value" + std::to_string(keys[i]));
        }
        
        // Test that all keys can be found despite collisions
        for (int key : keys) {
            std::string* found = map.find(key);
            assert(found != nullptr);
            assert(*found == "value" + std::to_string(key));
            assert(map.contains(key));
        }
        
        // Test that non-existent keys return nullptr
        assert(map.find(1) == nullptr);
        assert(map.find(9) == nullptr);
        assert(map.find(100) == nullptr);
        
        std::cout << "Lookup with collisions: PASSED" << std::endl;
    }
    
    void testLookupAfterRehashing() {
        std::cout << "Testing lookup after rehashing..." << std::endl;
        
        UnorderedMap<int, int> map(16);
        
        // Insert enough elements to trigger rehashing
        std::vector<std::pair<int, int>> test_data;
        for (int i = 0; i < 20; ++i) {
            int key = key_dist(rng);
            int value = value_dist(rng);
            test_data.push_back({key, value});
            map.insert(key, value);
        }
        
        // Verify all elements can still be found after potential rehashing
        for (const auto& pair : test_data) {
            int* found = map.find(pair.first);
            assert(found != nullptr);
            assert(*found == pair.second);
            assert(map.contains(pair.first));
        }
        
        std::cout << "Lookup after rehashing: PASSED" << std::endl;
    }
    
    void testLookupAfterErase() {
        std::cout << "Testing lookup after erase operations..." << std::endl;
        
        UnorderedMap<int, std::string> map;
        
        // Insert test data
        for (int i = 1; i <= 20; ++i) {
            map.insert(i, "value" + std::to_string(i));
        }
        
        // Erase some elements
        std::vector<int> to_erase = {2, 5, 8, 12, 15, 18};
        for (int key : to_erase) {
            bool erased = map.erase(key);
            assert(erased == true);
        }
        
        // Test that erased elements cannot be found
        for (int key : to_erase) {
            assert(map.find(key) == nullptr);
            assert(!map.contains(key));
        }
        
        // Test that remaining elements can still be found
        for (int i = 1; i <= 20; ++i) {
            bool should_exist = std::find(to_erase.begin(), to_erase.end(), i) == to_erase.end();
            
            if (should_exist) {
                std::string* found = map.find(i);
                assert(found != nullptr);
                assert(*found == "value" + std::to_string(i));
                assert(map.contains(i));
            }
        }
        
        std::cout << "Lookup after erase: PASSED" << std::endl;
    }
    
    void testLookupWithDifferentTypes() {
        std::cout << "Testing lookup with different key types..." << std::endl;
        
        // Test with string keys
        UnorderedMap<std::string, int> string_map;
        string_map.insert("hello", 1);
        string_map.insert("world", 2);
        string_map.insert("test", 3);
        
        assert(*string_map.find("hello") == 1);
        assert(*string_map.find("world") == 2);
        assert(string_map.find("nonexistent") == nullptr);
        
        // Test with double keys (using fallback hash)
        UnorderedMap<double, std::string> double_map;
        double_map.insert(1.5, "one-point-five");
        double_map.insert(2.7, "two-point-seven");
        double_map.insert(3.14, "pi");
        
        assert(*double_map.find(1.5) == "one-point-five");
        assert(*double_map.find(3.14) == "pi");
        assert(double_map.find(999.0) == nullptr);
        
        std::cout << "Lookup with different types: PASSED" << std::endl;
    }
    
    void testStressLookup(int iterations = 100000) {
        std::cout << "Testing stress lookup (" << iterations << " operations)..." << std::endl;
        
        UnorderedMap<int, int> map;
        std::vector<std::pair<int, int>> inserted_data;
        
        // Insert random data
        for (int i = 0; i < iterations / 10; ++i) {
            int key = key_dist(rng);
            int value = value_dist(rng);
            
            // Avoid duplicate keys for this test
            if (map.find(key) == nullptr) {
                map.insert(key, value);
                inserted_data.push_back({key, value});
            }
        }
        
        // Perform many lookup operations
        int successful_lookups = 0;
        int failed_lookups = 0;
        
        for (int i = 0; i < iterations; ++i) {
            int lookup_key = key_dist(rng);
            int* found = map.find(lookup_key);
            
            if (found) {
                successful_lookups++;
                // Verify the value is correct
                auto it = std::find_if(inserted_data.begin(), inserted_data.end(),
                    [lookup_key](const std::pair<int, int>& p) {
                        return p.first == lookup_key;
                    });
                assert(it != inserted_data.end());
                assert(*found == it->second);
            } else {
                failed_lookups++;
            }
        }
        
        std::cout << "Successful lookups: " << successful_lookups << std::endl;
        std::cout << "Failed lookups: " << failed_lookups << std::endl;
        std::cout << "Total data size: " << inserted_data.size() << std::endl;
        
        std::cout << "Stress lookup: PASSED" << std::endl;
    }
    
    void performanceLookupTest() {
        std::cout << "\n=== LOOKUP PERFORMANCE TESTING ===" << std::endl;
        
        const size_t num_elements = 100000;
        const size_t num_lookups = 1000000;
        
        // Prepare test data
        UnorderedMap<int, int> map;
        std::unordered_map<int, int> std_map;
        std::vector<int> keys;
        
        // Insert data into both maps
        for (size_t i = 0; i < num_elements; ++i) {
            int key = static_cast<int>(i);
            int value = key * 2;
            map.insert(key, value);
            std_map[key] = value;
            keys.push_back(key);
        }
        
        // Add some random keys for failed lookups
        std::vector<int> lookup_keys = keys;
        for (size_t i = 0; i < num_lookups / 10; ++i) {
            lookup_keys.push_back(key_dist(rng) + num_elements);
        }
        
        // Shuffle lookup keys
        std::shuffle(lookup_keys.begin(), lookup_keys.end(), rng);
        
        // Test your UnorderedMap find performance
        auto start = std::chrono::high_resolution_clock::now();
        
        volatile int sum = 0;
        for (size_t i = 0; i < num_lookups; ++i) {
            int key = lookup_keys[i % lookup_keys.size()];
            int* found = map.find(key);
            if (found) {
                sum += *found;
            }
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto your_find_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        // Test your UnorderedMap contains performance
        start = std::chrono::high_resolution_clock::now();
        
        volatile int contains_count = 0;
        for (size_t i = 0; i < num_lookups; ++i) {
            int key = lookup_keys[i % lookup_keys.size()];
            if (map.contains(key)) {
                contains_count++;
            }
        }
        
        end = std::chrono::high_resolution_clock::now();
        auto your_contains_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        // Test std::unordered_map performance
        start = std::chrono::high_resolution_clock::now();
        
        volatile int std_sum = 0;
        for (size_t i = 0; i < num_lookups; ++i) {
            int key = lookup_keys[i % lookup_keys.size()];
            auto it = std_map.find(key);
            if (it != std_map.end()) {
                std_sum += it->second;
            }
        }
        
        end = std::chrono::high_resolution_clock::now();
        auto std_find_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        // Results
        std::cout << "Your find() performance: " << your_find_time.count() << " μs (" << num_lookups << " lookups)" << std::endl;
        std::cout << "Your contains() performance: " << your_contains_time.count() << " μs (" << num_lookups << " lookups)" << std::endl;
        std::cout << "std::unordered_map find: " << std_find_time.count() << " μs" << std::endl;
        
        std::cout << "Find performance ratio: " << (double)std_find_time.count() / your_find_time.count() << std::endl;
        std::cout << "Contains vs find ratio: " << (double)your_contains_time.count() / your_find_time.count() << std::endl;
        
        std::cout << "Average find time: " << (double)your_find_time.count() / num_lookups << " μs" << std::endl;
        std::cout << "Average contains time: " << (double)your_contains_time.count() / num_lookups << " μs" << std::endl;
        
        std::cout << "Sum (prevent optimization): " << sum << std::endl;
        std::cout << "Contains count: " << contains_count << std::endl;
    }
    
    void runAllLookupTests() {
        std::cout << "=== UNORDERED MAP LOOKUP & SEARCH TESTING ===" << std::endl;
        std::cout << "Seed: " << rng() << std::endl << std::endl;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        testBasicFindOperations();
        testConstFindOperations();
        testContainsOperations();
        testOperatorBracketAccess();
        testAtMethodAccess();
        testLookupWithCollisions();
        testLookupAfterRehashing();
        testLookupAfterErase();
        testLookupWithDifferentTypes();
        testStressLookup();
        performanceLookupTest();
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << std::endl << "=== ALL LOOKUP TESTS COMPLETED ===" << std::endl;
        std::cout << "Total time: " << duration.count() << "ms" << std::endl;
    }
};
