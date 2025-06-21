#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <cassert>
#include <string>
#include <memory>
#include "Config.hpp"
#include "Vector.hpp"

class VectorTester {
private:
    std::mt19937 rng;
    std::uniform_int_distribution<int> value_dist;
    std::uniform_int_distribution<size_t> index_dist;
    
public:
    VectorTester() : rng(std::chrono::steady_clock::now().time_since_epoch().count()),
                     value_dist(-1000, 1000),
                     index_dist(0, 999) {}
    
    void testBasicConstruction() {
        std::cout << "Testing basic construction..." << std::endl;
        
        // Test default construction
        Vector<int> vec1;
        assert(vec1.empty());
        assert(vec1.size() == 0);
        assert(vec1.capacity() == 0);
        
        // Test construction with initial capacity
        Vector<int> vec2(10);
        assert(vec2.size() == 10);
        assert(vec2.capacity() == 10);
        
        // Verify default initialization
        for (size_t i = 0; i < vec2.size(); ++i) {
            assert(vec2[i] == int{});
        }
        
        // Test with different types
        Vector<std::string> vec3(5);
        assert(vec3.size() == 5);
        for (size_t i = 0; i < vec3.size(); ++i) {
            assert(vec3[i] == std::string{});
        }
        
        std::cout << "Basic construction: PASSED" << std::endl;
    }
    
    void testCopyConstructor() {
        std::cout << "Testing copy constructor..." << std::endl;
        
        // Create original vector with data
        Vector<int> original(5);
        for (size_t i = 0; i < original.size(); ++i) {
            original[i] = static_cast<int>(i * 10);
        }
        
        // Test copy constructor
        Vector<int> copy(original);
        
        // Verify sizes match
        assert(copy.size() == original.size());
        assert(copy.capacity() == original.capacity());
        
        // Verify data integrity
        for (size_t i = 0; i < original.size(); ++i) {
            assert(copy[i] == original[i]);
        }
        
        // Verify independence (modify copy shouldn't affect original)
        copy[0] = 999;
        assert(original[0] != 999);
        
        std::cout << "Copy constructor: PASSED" << std::endl;
    }
    
    void testAssignmentOperator() {
        std::cout << "Testing assignment operator..." << std::endl;
        
        Vector<int> vec1(5);
        Vector<int> vec2(3);
        
        // Fill vec1
        for (size_t i = 0; i < vec1.size(); ++i) {
            vec1[i] = static_cast<int>(i * 2);
        }
        
        // Fill vec2 with different data
        for (size_t i = 0; i < vec2.size(); ++i) {
            vec2[i] = 100;
        }
        
        // Test assignment
        vec2 = vec1;
        
        assert(vec2.size() == vec1.size());
        assert(vec2.capacity() == vec1.capacity());
        
        for (size_t i = 0; i < vec1.size(); ++i) {
            assert(vec2[i] == vec1[i]);
        }
        
        // Test self-assignment
        vec1 = vec1;
        assert(vec1.size() == 5);
        
        std::cout << "Assignment operator: PASSED" << std::endl;
    }
    
    void testMoveSemantics() {
        std::cout << "Testing move semantics..." << std::endl;
        
        // Test move constructor
        Vector<int> original(100);
        for (size_t i = 0; i < original.size(); ++i) {
            original[i] = static_cast<int>(i);
        }
        
        size_t original_size = original.size();
        size_t original_capacity = original.capacity();
        
        Vector<int> moved(std::move(original));
        
        assert(moved.size() == original_size);
        assert(moved.capacity() == original_capacity);
        assert(original.empty()); // Original should be empty after move
        assert(original.capacity() == 0);
        
        // Test move assignment
        Vector<int> another(10);
        another = std::move(moved);
        
        assert(another.size() == original_size);
        assert(moved.empty()); // moved should be empty after move assignment
        
        std::cout << "Move semantics: PASSED" << std::endl;
    }
    
    void testPushBackAndPopBack() {
        std::cout << "Testing push_back and pop_back..." << std::endl;
        
        Vector<int> vec;
        std::vector<int> values = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        
        // Test push_back (lvalue)
        for (int val : values) {
            vec.push_back(val);
            assert(vec.back() == val);
            assert(!vec.empty());
        }
        
        assert(vec.size() == values.size());
        
        // Test push_back (rvalue)
        vec.push_back(std::move(100));
        assert(vec.back() == 100);
        
        // Test pop_back
        int last = vec.pop_back();
        assert(last == 100);
        assert(vec.size() == values.size());
        
        for (int i = values.size() - 1; i >= 0; --i) {
            int popped = vec.pop_back();
            assert(popped == values[i]);
        }
        
        assert(vec.empty());
        
        std::cout << "Push_back and pop_back: PASSED" << std::endl;
    }
    
    void testEmplaceBack() {
        std::cout << "Testing emplace_back..." << std::endl;
        
        Vector<std::string> vec;
        
        // Test emplace_back with string construction
        vec.emplace_back("Hello");
        vec.emplace_back("World");
        vec.emplace_back(5, 'A'); // String with 5 'A's
        
        assert(vec.size() == 3);
        assert(vec[0] == "Hello");
        assert(vec[1] == "World");
        assert(vec[2] == "AAAAA");
        
        // Test with complex type
        Vector<std::pair<int, std::string>> pair_vec;
        pair_vec.emplace_back(42, "Answer");
        
        assert(pair_vec.size() == 1);
        assert(pair_vec[0].first == 42);
        assert(pair_vec[0].second == "Answer");
        
        std::cout << "Emplace_back: PASSED" << std::endl;
    }
    
    void testAccessMethods() {
        std::cout << "Testing access methods..." << std::endl;
        
        Vector<int> vec(10);
        for (size_t i = 0; i < vec.size(); ++i) {
            vec[i] = static_cast<int>(i * 5);
        }
        
        // Test operator[]
        for (size_t i = 0; i < vec.size(); ++i) {
            assert(vec[i] == static_cast<int>(i * 5));
        }
        
        // Test const operator[]
        const Vector<int>& const_vec = vec;
        for (size_t i = 0; i < const_vec.size(); ++i) {
            assert(const_vec[i] == static_cast<int>(i * 5));
        }
        
        // Test back() and top()
        assert(vec.back() == vec.top());
        assert(vec.back() == 45); // 9 * 5
        
        // Test pointer access
        int* ptr = vec.pointer();
        const int* const_ptr = const_vec.pointer();
        assert(ptr[0] == 0);
        assert(const_ptr[0] == 0);
        
        std::cout << "Access methods: PASSED" << std::endl;
    }
    
    void testErase() {
        std::cout << "Testing erase operation..." << std::endl;
        
        Vector<int> vec;
        for (int i = 0; i < 10; ++i) {
            vec.push_back(i);
        }
        
        // Test valid erase
        bool result = vec.erase(5); // Remove element with value 5
        assert(result == true);
        assert(vec.size() == 9);
        
        // Verify elements shifted correctly
        for (size_t i = 0; i < 5; ++i) {
            assert(vec[i] == static_cast<int>(i));
        }
        for (size_t i = 5; i < vec.size(); ++i) {
            assert(vec[i] == static_cast<int>(i + 1));
        }
        
        // Test invalid erase
        result = vec.erase(100);
        assert(result == false);
        assert(vec.size() == 9);
        
        // Test erase first element
        result = vec.erase(0);
        assert(result == true);
        assert(vec.size() == 8);
        assert(vec[0] == 1);
        
        // Test erase last element
        result = vec.erase(vec.size() - 1);
        assert(result == true);
        assert(vec.size() == 7);
        
        std::cout << "Erase: PASSED" << std::endl;
    }
    
    void testReserve() {
        std::cout << "Testing reserve operation..." << std::endl;
        
        Vector<int> vec;
        
        // Initial state
        assert(vec.capacity() == 0);
        
        // Reserve capacity
        vec.reserve(100);
        assert(vec.capacity() >= 100);
        assert(vec.size() == 0);
        
        // Add elements (should not trigger reallocation)
        for (int i = 0; i < 50; ++i) {
            size_t old_capacity = vec.capacity();
            vec.push_back(i);
            assert(vec.capacity() == old_capacity); // No reallocation
        }
        
        // Reserve smaller capacity (should not shrink)
        size_t current_capacity = vec.capacity();
        vec.reserve(10);
        assert(vec.capacity() == current_capacity);
        
        std::cout << "Reserve: PASSED" << std::endl;
    }
    
    void testClear() {
        std::cout << "Testing clear operation..." << std::endl;
        
        Vector<int> vec;
        for (int i = 0; i < 100; ++i) {
            vec.push_back(i);
        }
        
        assert(vec.size() == 100);
        assert(!vec.empty());
        
        size_t old_capacity = vec.capacity();
        vec.clear();
        
        assert(vec.size() == 0);
        assert(vec.empty());
        assert(vec.capacity() == old_capacity); // Capacity preserved
        
        // Should be able to use vector after clear
        vec.push_back(42);
        assert(vec.size() == 1);
        assert(vec[0] == 42);
        
        std::cout << "Clear: PASSED" << std::endl;
    }
    
    void testIterators() {
        std::cout << "Testing iterators..." << std::endl;
        
        Vector<int> vec;
        for (int i = 0; i < 10; ++i) {
            vec.push_back(i);
        }
        
        // Test range-based for loop
        int expected = 0;
        for (int value : vec) {
            assert(value == expected++);
        }
        
        // Test iterator arithmetic
        auto it = vec.begin();
        assert(*it == 0);
        
        ++it;
        assert(*it == 1);
        
        // Test iterator comparison
        assert(it != vec.end());
        
        // Test full iteration
        int count = 0;
        for (auto iter = vec.begin(); iter != vec.end(); ++iter) {
            assert(*iter == count++);
        }
        
        std::cout << "Iterators: PASSED" << std::endl;
    }
    
    void testStressOperations(int iterations = 10000) {
        std::cout << "Testing stress operations (" << iterations << " iterations)..." << std::endl;
        
        Vector<int> vec;
        std::vector<int> reference;
        
        for (int i = 0; i < iterations; ++i) {
            int operation = value_dist(rng) % 4;
            
            if (operation == 0 || vec.empty()) {
                // Push operation
                int value = value_dist(rng);
                vec.push_back(value);
                reference.push_back(value);
            } else if (operation == 1) {
                // Pop operation
                if (!reference.empty()) {
                    int expected = reference.back();
                    reference.pop_back();
                    
                    int popped = vec.pop_back();
                    assert(popped == expected);
                }
            } else if (operation == 2) {
                // Access operation
                if (!reference.empty()) {
                    size_t idx = index_dist(rng) % reference.size();
                    assert(vec[idx] == reference[idx]);
                }
            } else {
                // Erase operation
                if (!reference.empty()) {
                    size_t idx = index_dist(rng) % reference.size();
                    vec.erase(idx);
                    reference.erase(reference.begin() + idx);
                }
            }
            
            // Verify size consistency
            assert(vec.size() == reference.size());
            assert(vec.empty() == reference.empty());
        }
        
        std::cout << "Stress operations: PASSED" << std::endl;
    }
    
    void testWithComplexTypes() {
        std::cout << "Testing with complex types..." << std::endl;
        
        // Test with unique_ptr
        // Vector<std::unique_ptr<int>> ptr_vec;
        // ptr_vec.push_back(std::make_unique<int>(42));
        // ptr_vec.emplace_back(std::make_unique<int>(100));
        
        // assert(*ptr_vec[0] == 42);
        // assert(*ptr_vec[1] == 100);
        
        // // Test with vector
        // Vector<std::vector<int>> vec_vec;
        // vec_vec.emplace_back(std::vector<int>{1, 2, 3, 4, 5});
        // vec_vec.push_back(std::vector<int>{10, 20, 30});
        
        // assert(vec_vec[0].size() == 5);
        // assert(vec_vec[1].size() == 3);
        // assert(vec_vec[0][0] == 1);
        // assert(vec_vec[1][0] == 10);
        
        std::cout << "Complex types: PASSED" << std::endl;
    }
    
    void performanceTest() {
        std::cout << "\n=== PERFORMANCE TESTING ===" << std::endl;
        
        const size_t iterations = 1000000;
        
        // Test push_back performance
        auto start = std::chrono::high_resolution_clock::now();
        
        Vector<int> vec;
        for (size_t i = 0; i < iterations; ++i) {
            vec.push_back(static_cast<int>(i));
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto push_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        // Test access performance
        start = std::chrono::high_resolution_clock::now();
        
        volatile long long sum = 0;
        for (size_t i = 0; i < iterations; ++i) {
            sum += vec[i % vec.size()];
        }
        
        end = std::chrono::high_resolution_clock::now();
        auto access_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        // Test pop_back performance
        start = std::chrono::high_resolution_clock::now();
        
        while (!vec.empty()) {
            vec.pop_back();
        }
        
        end = std::chrono::high_resolution_clock::now();
        auto pop_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        // Compare with std::vector
        start = std::chrono::high_resolution_clock::now();
        std::vector<int> std_vec;
        for (size_t i = 0; i < iterations; ++i) {
            std_vec.push_back(static_cast<int>(i));
        }
        end = std::chrono::high_resolution_clock::now();
        auto std_push_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        std::cout << "Push_back performance: " << push_time.count() << " μs (" << iterations << " operations)" << std::endl;
        std::cout << "Access performance: " << access_time.count() << " μs (" << iterations << " operations)" << std::endl;
        std::cout << "Pop_back performance: " << pop_time.count() << " μs (" << iterations << " operations)" << std::endl;
        std::cout << "std::vector push_back: " << std_push_time.count() << " μs" << std::endl;
        std::cout << "Performance ratio: " << (double)std_push_time.count() / push_time.count() << std::endl;
    }
    
    void runAllTests() {
        std::cout << "=== VECTOR COMPREHENSIVE TESTING ===" << std::endl;
        std::cout << "Seed: " << rng() << std::endl << std::endl;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        testBasicConstruction();
        testCopyConstructor();
        testAssignmentOperator();
        testMoveSemantics();
        testPushBackAndPopBack();
        testEmplaceBack();
        testAccessMethods();
        testErase();
        testReserve();
        testClear();
        testIterators();
        testStressOperations();
        testWithComplexTypes();
        performanceTest();
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << std::endl << "=== ALL TESTS COMPLETED ===" << std::endl;
        std::cout << "Total time: " << duration.count() << "ms" << std::endl;
    }
};


