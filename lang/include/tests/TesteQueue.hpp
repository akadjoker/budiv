#include <iostream>
#include <queue>
#include <random>
#include <chrono>
#include <cassert>
#include <string>
#include <memory>
#include "Config.hpp"
#include "Queue.hpp"

class QueueTester {
private:
    std::mt19937 rng;
    std::uniform_int_distribution<int> value_dist;
    
public:
    QueueTester() : rng(std::chrono::steady_clock::now().time_since_epoch().count()),
                    value_dist(-1000, 1000) {}
    
    void testBasicConstruction() {
        std::cout << "Testing basic construction..." << std::endl;
        
        // Test default construction
        Queue<int> queue1;
        assert(queue1.empty());
        assert(queue1.size() == 0);
        
        // Test with different types
        Queue<std::string> queue2;
        Queue<double> queue3;
        Queue<char> queue4;
        
        assert(queue2.empty() && queue3.empty() && queue4.empty());
        
        std::cout << "Basic construction: PASSED" << std::endl;
    }
    
    void testCopyConstructor() {
        std::cout << "Testing copy constructor..." << std::endl;
        
        // Create original queue with data
        Queue<int> original;
        std::vector<int> test_values = {10, 20, 30, 40, 50};
        
        for (int val : test_values) {
            original.push(val);
        }
        
        // Test copy constructor
        Queue<int> copy(original);
        
        // Verify sizes match
        assert(copy.size() == original.size());
        assert(!copy.empty());
        
        // Verify data integrity (FIFO order)
        for (int expected : test_values) {
            assert(copy.front() == expected);
            assert(copy.pop() == expected);
        }
        
        // Original should still have all data
        assert(original.size() == test_values.size());
        
        std::cout << "Copy constructor: PASSED" << std::endl;
    }
    
    void testAssignmentOperator() {
        std::cout << "Testing assignment operator..." << std::endl;
        
        Queue<int> queue1;
        Queue<int> queue2;
        
        // Fill queue1
        for (int i = 1; i <= 5; ++i) {
            queue1.push(i * 10);
        }
        
        // Fill queue2 with different data
        queue2.push(999);
        
        // Test assignment
        queue2 = queue1;
        
        assert(queue2.size() == queue1.size());
        assert(queue2.front() == queue1.front());
        assert(queue2.back() == queue1.back());
        
        // Test self-assignment
        queue1 = queue1;
        assert(queue1.size() == 5);
        
        std::cout << "Assignment operator: PASSED" << std::endl;
    }
    
    void testMoveSemantics() {
        std::cout << "Testing move semantics..." << std::endl;
        
        // Test move constructor
        Queue<int> original;
        for (int i = 1; i <= 5; ++i) {
            original.push(i);
        }
        
        size_t original_size = original.size();
        Queue<int> moved(std::move(original));
        
        assert(moved.size() == original_size);
        assert(original.empty()); // Original should be empty after move
        assert(moved.front() == 1);
        assert(moved.back() == 5);
        
        // Test move assignment
        Queue<int> another;
        another.push(100);
        
        another = std::move(moved);
        assert(another.size() == original_size);
        assert(moved.empty()); // moved should be empty after move assignment
        
        std::cout << "Move semantics: PASSED" << std::endl;
    }
    
    void testPushAndPop() {
        std::cout << "Testing push and pop operations..." << std::endl;
        
        Queue<int> queue;
        std::vector<int> values = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        
        // Test push
        for (int val : values) {
            queue.push(val);
            assert(queue.back() == val);
            assert(!queue.empty());
        }
        
        assert(queue.size() == values.size());
        assert(queue.front() == values[0]); // First in
        assert(queue.back() == values.back()); // Last in
        
        // Test pop (FIFO order)
        for (int expected : values) {
            assert(queue.front() == expected);
            int popped = queue.pop();
            assert(popped == expected);
        }
        
        assert(queue.empty());
        assert(queue.size() == 0);
        
        std::cout << "Push and pop: PASSED" << std::endl;
    }
    
    void testEmplace() {
        std::cout << "Testing emplace operation..." << std::endl;
        
        Queue<std::string> queue;
        
        // Test emplace with string construction
        queue.emplace("Hello");
        queue.emplace("World");
        queue.emplace(5, 'A'); // String with 5 'A's
        
        assert(queue.size() == 3);
        assert(queue.front() == "Hello");
        
        queue.pop();
        assert(queue.front() == "World");
        
        queue.pop();
        assert(queue.front() == "AAAAA");
        
        std::cout << "Emplace: PASSED" << std::endl;
    }
    
    void testFrontAndBack() {
        std::cout << "Testing front and back access..." << std::endl;
        
        Queue<int> queue;
        
        // Test with single element
        queue.push(42);
        assert(queue.front() == 42);
        assert(queue.back() == 42);
        
        // Test with multiple elements
        queue.push(100);
        queue.push(200);
        
        assert(queue.front() == 42);  // First element
        assert(queue.back() == 200);  // Last element
        
        // Test after pop
        queue.pop();
        assert(queue.front() == 100);
        assert(queue.back() == 200);
        
        std::cout << "Front and back access: PASSED" << std::endl;
    }
    
    void testCircularBuffer() {
        std::cout << "Testing circular buffer behavior..." << std::endl;
        
        Queue<int> queue;
        
        // Fill queue to force initial resize
        for (int i = 0; i < 10; ++i) {
            queue.push(i);
        }
        
        // Pop some elements to create space at front
        for (int i = 0; i < 5; ++i) {
            assert(queue.pop() == i);
        }
        
        // Push more elements (should wrap around)
        for (int i = 10; i < 15; ++i) {
            queue.push(i);
        }
        
        // Verify FIFO order is maintained
        for (int i = 5; i < 15; ++i) {
            assert(queue.front() == i);
            assert(queue.pop() == i);
        }
        
        assert(queue.empty());
        
        std::cout << "Circular buffer: PASSED" << std::endl;
    }
    
    void testExceptionHandling() {
        std::cout << "Testing exception handling..." << std::endl;
        
        Queue<int> queue;
        
        // Test pop on empty queue
        try {
            queue.pop();
            assert(false); // Should not reach here
        } catch (const std::out_of_range& e) {
            // Expected exception
        }
        
        // Test front on empty queue
        try {
            queue.front();
            assert(false); // Should not reach here
        } catch (const std::out_of_range& e) {
            // Expected exception
        }
        
        // Test back on empty queue
        try {
            queue.back();
            assert(false); // Should not reach here
        } catch (const std::out_of_range& e) {
            // Expected exception
        }
        
        std::cout << "Exception handling: PASSED" << std::endl;
    }
    
    void testStressOperations(int iterations = 10000) {
        std::cout << "Testing stress operations (" << iterations << " iterations)..." << std::endl;
        
        Queue<int> queue;
        std::queue<int> reference;
        
        for (int i = 0; i < iterations; ++i) {
            int operation = value_dist(rng) % 3;
            
            if (operation == 0 || queue.empty()) {
                // Push operation
                int value = value_dist(rng);
                queue.push(value);
                reference.push(value);
            } else if (operation == 1) {
                // Pop operation
                if (!reference.empty()) {
                    int expected = reference.front();
                    reference.pop();
                    
                    assert(queue.front() == expected);
                    int popped = queue.pop();
                    assert(popped == expected);
                }
            } else {
                // Front/back operations
                if (!reference.empty()) {
                    assert(queue.front() == reference.front());
                    assert(queue.back() == reference.back());
                }
            }
            
            // Verify size consistency
            assert(queue.size() == reference.size());
            assert(queue.empty() == reference.empty());
        }
        
        std::cout << "Stress operations: PASSED" << std::endl;
    }
    
    void testWithComplexTypes() {
        std::cout << "Testing with complex types..." << std::endl;
        
        // Test with unique_ptr
        Queue<std::unique_ptr<int>> ptr_queue;
        ptr_queue.push(std::make_unique<int>(42));
        ptr_queue.emplace(std::make_unique<int>(100));
        
        assert(*ptr_queue.front() == 42);
        auto ptr = ptr_queue.pop();
        assert(*ptr == 42);
        
        // Test with vector
        Queue<std::vector<int>> vec_queue;
        vec_queue.emplace(std::vector<int>{1, 2, 3, 4, 5});
        vec_queue.push(std::vector<int>{10, 20, 30});
        
        assert(vec_queue.front().size() == 5);
        assert(vec_queue.front()[0] == 1);
        
        std::cout << "Complex types: PASSED" << std::endl;
    }
    
    void testResizeAndWrapAround() {
        std::cout << "Testing resize and wrap-around scenarios..." << std::endl;
        
        Queue<int> queue;
        
        // Force multiple resizes with wrap-around
        for (int cycle = 0; cycle < 3; ++cycle) {
            // Fill queue
            for (int i = 0; i < 100; ++i) {
                queue.push(cycle * 100 + i);
            }
            
            // Remove half
            for (int i = 0; i < 50; ++i) {
                int expected = cycle * 100 + i;
                assert(queue.pop() == expected);
            }
        }
        
        // Verify remaining elements
        int expected_start = 50; // From first cycle
        for (int cycle = 0; cycle < 3; ++cycle) {
            for (int i = expected_start; i < 100; ++i) {
                int expected = cycle * 100 + i;
                assert(queue.front() == expected);
                assert(queue.pop() == expected);
            }
            expected_start = 0; // Reset for subsequent cycles
        }
        
        assert(queue.empty());
        
        std::cout << "Resize and wrap-around: PASSED" << std::endl;
    }
    
    void performanceTest() {
        std::cout << "\n=== PERFORMANCE TESTING ===" << std::endl;
        
        const size_t iterations = 1000000;
        
        // Test push performance
        auto start = std::chrono::high_resolution_clock::now();
        
        Queue<int> queue;
        for (size_t i = 0; i < iterations; ++i) {
            queue.push(static_cast<int>(i));
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto push_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        // Test pop performance
        start = std::chrono::high_resolution_clock::now();
        
        volatile int sum = 0; // Prevent optimization
        for (size_t i = 0; i < iterations; ++i) {
            sum += queue.pop();
        }
        
        end = std::chrono::high_resolution_clock::now();
        auto pop_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        // Test mixed operations (realistic workload)
        start = std::chrono::high_resolution_clock::now();
        
        Queue<int> mixed_queue;
        for (size_t i = 0; i < iterations / 2; ++i) {
            mixed_queue.push(static_cast<int>(i));
            mixed_queue.push(static_cast<int>(i + 1000));
            mixed_queue.pop(); // Keep queue size manageable
        }
        
        end = std::chrono::high_resolution_clock::now();
        auto mixed_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        // Compare with std::queue
        start = std::chrono::high_resolution_clock::now();
        std::queue<int> std_queue;
        for (size_t i = 0; i < iterations; ++i) {
            std_queue.push(static_cast<int>(i));
        }
        end = std::chrono::high_resolution_clock::now();
        auto std_push_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        std::cout << "Push performance: " << push_time.count() << " μs (" << iterations << " operations)" << std::endl;
        std::cout << "Pop performance: " << pop_time.count() << " μs (" << iterations << " operations)" << std::endl;
        std::cout << "Mixed operations: " << mixed_time.count() << " μs (" << iterations/2 << " cycles)" << std::endl;
        std::cout << "std::queue push: " << std_push_time.count() << " μs" << std::endl;
        std::cout << "Performance ratio: " << (double)std_push_time.count() / push_time.count() << std::endl;
        
        std::cout << "Average push: " << (double)push_time.count() / iterations << " μs" << std::endl;
        std::cout << "Average pop: " << (double)pop_time.count() / iterations << " μs" << std::endl;
    }
    
    void runAllTests() {
        std::cout << "=== QUEUE COMPREHENSIVE TESTING ===" << std::endl;
        std::cout << "Seed: " << rng() << std::endl << std::endl;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        testBasicConstruction();
        testCopyConstructor();
        testAssignmentOperator();
        testMoveSemantics();
        testPushAndPop();
        testEmplace();
        testFrontAndBack();
        testCircularBuffer();
        testExceptionHandling();
        testStressOperations();
        testWithComplexTypes();
        testResizeAndWrapAround();
        performanceTest();
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << std::endl << "=== ALL TESTS COMPLETED ===" << std::endl;
        std::cout << "Total time: " << duration.count() << "ms" << std::endl;
    }
};

 
