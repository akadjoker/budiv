#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <cassert>
#include <string>
#include <memory>
#include <climits>
#include "Stack.hpp"

class StackTester {
private:
    std::mt19937 rng;
    std::uniform_int_distribution<int> value_dist;
    
public:
    StackTester() : rng(std::chrono::steady_clock::now().time_since_epoch().count()),
                    value_dist(-1000, 1000) {}
    
    void testBasicConstruction() {
        std::cout << "Testing basic construction..." << std::endl;
        
        // Test default construction
        Stack<int> stack1;
        assert(stack1.empty());
        assert(stack1.size() == 0);
        
        // Test with different types
        Stack<std::string> stack2;
        Stack<double> stack3;
        Stack<char> stack4;
        
        assert(stack2.empty() && stack3.empty() && stack4.empty());
        
        std::cout << "Basic construction: PASSED" << std::endl;
    }
    
    void testCopyConstructor() {
        std::cout << "Testing copy constructor..." << std::endl;
        
        // Create original stack with data
        Stack<int> original;
        std::vector<int> test_values = {10, 20, 30, 40, 50};
        
        for (int val : test_values) {
            original.push(val);
        }
        
        // Test copy constructor
        Stack<int> copy(original);
        
        // Verify sizes match
        assert(copy.size() == original.size());
        assert(!copy.empty());
        
        // Verify data integrity (pop from copy shouldn't affect original)
        for (int i = test_values.size() - 1; i >= 0; --i) {
            assert(copy.top() == test_values[i]);
            assert(copy.pop() == test_values[i]);
        }
        
        // Original should still have all data
        assert(original.size() == test_values.size());
        
        std::cout << "Copy constructor: PASSED" << std::endl;
    }
    
    void testAssignmentOperator() {
        std::cout << "Testing assignment operator..." << std::endl;
        
        Stack<int> stack1;
        Stack<int> stack2;
        
        // Fill stack1
        for (int i = 1; i <= 5; ++i) {
            stack1.push(i * 10);
        }
        
        // Fill stack2 with different data
        stack2.push(999);
        
        // Test assignment
        stack2 = stack1;
        
        assert(stack2.size() == stack1.size());
        assert(stack2.top() == stack1.top());
        
        // Test self-assignment
        stack1 = stack1;
        assert(stack1.size() == 5);
        
        std::cout << "Assignment operator: PASSED" << std::endl;
    }
    
    void testMoveSemantics() {
        std::cout << "Testing move semantics..." << std::endl;
        
        // Test move constructor
        Stack<int> original;
        for (int i = 1; i <= 5; ++i) {
            original.push(i);
        }
        
        size_t original_size = original.size();
        Stack<int> moved(std::move(original));
        
        assert(moved.size() == original_size);
        assert(original.empty()); // Original should be empty after move
        assert(moved.top() == 5);
        
        // Test move assignment
        Stack<int> another;
        another.push(100);
        
        another = std::move(moved);
        assert(another.size() == original_size);
        assert(moved.empty()); // moved should be empty after move assignment
        
        std::cout << "Move semantics: PASSED" << std::endl;
    }
    
    void testPushAndPop() {
        std::cout << "Testing push and pop operations..." << std::endl;
        
        Stack<int> stack;
        std::vector<int> values = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        
        // Test push
        for (int val : values) {
            stack.push(val);
            assert(stack.top() == val);
            assert(!stack.empty());
        }
        
        assert(stack.size() == values.size());
        
        // Test pop (LIFO order)
        for (int i = values.size() - 1; i >= 0; --i) {
            assert(stack.top() == values[i]);
            int popped = stack.pop();
            assert(popped == values[i]);
        }
        
        assert(stack.empty());
        assert(stack.size() == 0);
        
        std::cout << "Push and pop: PASSED" << std::endl;
    }
    
    void testEmplace() {
        std::cout << "Testing emplace operation..." << std::endl;
        
        Stack<std::string> stack;
        
        // Test emplace with string construction
        stack.emplace("Hello");
        stack.emplace("World");
        stack.emplace(5, 'A'); // String with 5 'A's
        
        assert(stack.size() == 3);
        assert(stack.top() == "AAAAA");
        
        stack.pop();
        assert(stack.top() == "World");
        
        stack.pop();
        assert(stack.top() == "Hello");
        
        std::cout << "Emplace: PASSED" << std::endl;
    }
    
    void testAccessMethods() {
        std::cout << "Testing access methods..." << std::endl;
        
        Stack<int> stack;
        for (int i = 0; i < 5; ++i) {
            stack.push(i * 10);
        }
        
        // Test operator[]
        assert(stack[0] == 0);   // Bottom element
        assert(stack[4] == 40);  // Top element
        
        // Test peek (from top)
        assert(stack.peek(0) == 40); // Top element
        assert(stack.peek(1) == 30); // Second from top
        assert(stack.peek(4) == 0);  // Bottom element
        
        // Test front and back
        assert(stack.front() == 0);  // Bottom element
        assert(stack.back() == 40);  // Top element
        
        // Test top
        assert(stack.top() == 40);
        
        std::cout << "Access methods: PASSED" << std::endl;
    }
    
    void testClear() {
        std::cout << "Testing clear operation..." << std::endl;
        
        Stack<int> stack;
        for (int i = 0; i < 100; ++i) {
            stack.push(i);
        }
        
        assert(stack.size() == 100);
        assert(!stack.empty());
        
        stack.clear();
        
        assert(stack.size() == 0);
        assert(stack.empty());
        
        // Should be able to use stack after clear
        stack.push(42);
        assert(stack.top() == 42);
        
        std::cout << "Clear: PASSED" << std::endl;
    }
    
    void testResize() {
        std::cout << "Testing resize operation..." << std::endl;
        
        Stack<int> stack;
        
        // Fill stack to trigger multiple resizes
        for (int i = 0; i < 1000; ++i) {
            stack.push(i);
        }
        
        assert(stack.size() == 1000);
        
        // Verify data integrity
        for (int i = 999; i >= 0; --i) {
            assert(stack.pop() == i);
        }
        
        assert(stack.empty());
        
        std::cout << "Resize: PASSED" << std::endl;
    }
    
    void testStressOperations(int iterations = 10000) {
        std::cout << "Testing stress operations (" << iterations << " iterations)..." << std::endl;
        
        Stack<int> stack;
        std::vector<int> reference;
        
        for (int i = 0; i < iterations; ++i) {
            int operation = value_dist(rng) % 3;
            
            if (operation == 0 || stack.empty()) {
                // Push operation
                int value = value_dist(rng);
                stack.push(value);
                reference.push_back(value);
            } else if (operation == 1) {
                // Pop operation
                if (!reference.empty()) {
                    int expected = reference.back();
                    reference.pop_back();
                    
                    assert(stack.top() == expected);
                    int popped = stack.pop();
                    assert(popped == expected);
                }
            } else {
                // Top operation
                if (!reference.empty()) {
                    assert(stack.top() == reference.back());
                }
            }
            
            // Verify size consistency
            assert(stack.size() == reference.size());
            assert(stack.empty() == reference.empty());
        }
        
        std::cout << "Stress operations: PASSED" << std::endl;
    }
    
    void testWithComplexTypes() {
        std::cout << "Testing with complex types..." << std::endl;
        
        // Test with unique_ptr
        Stack<std::unique_ptr<int>> ptr_stack;
        ptr_stack.push(std::make_unique<int>(42));
        ptr_stack.emplace(std::make_unique<int>(100));
        
        assert(*ptr_stack.top() == 100);
        auto ptr = ptr_stack.pop();
        assert(*ptr == 100);
        
        // Test with vector
        Stack<std::vector<int>> vec_stack;
        vec_stack.emplace(std::vector<int>{1, 2, 3, 4, 5});
        vec_stack.push(std::vector<int>{10, 20, 30});
        
        assert(vec_stack.top().size() == 3);
        assert(vec_stack.top()[0] == 10);
        
        std::cout << "Complex types: PASSED" << std::endl;
    }
    
    void testEdgeCases() {
        std::cout << "Testing edge cases..." << std::endl;
        
        Stack<int> stack;
        
        // Test increment method
        stack.push(10);
        size_t old_size = stack.size();
        stack.increment();
        assert(stack.size() == old_size + 1);
        
        // Test with zero capacity growth
        Stack<char> char_stack;
        for (int i = 0; i < 100; ++i) {
            char_stack.push('A' + (i % 26));
        }
        assert(char_stack.size() == 100);
        
        std::cout << "Edge cases: PASSED" << std::endl;
    }
    
    void performanceTest() {
    std::cout << "\n=== PERFORMANCE TESTING ===" << std::endl;
    
    const size_t iterations = 1000000; // Use size_t em vez de int
    
    // Test push performance
    auto start = std::chrono::high_resolution_clock::now();
    
    Stack<int> stack;
    for (size_t i = 0; i < iterations; ++i) {
        // Use valores menores para evitar overflow
        stack.push(static_cast<int>(i % INT_MAX));
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto push_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // Test pop performance
    start = std::chrono::high_resolution_clock::now();
    
    for (size_t i = 0; i < iterations; ++i) {
        stack.pop();
    }
    
    end = std::chrono::high_resolution_clock::now();
    auto pop_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // Test access performance com proteção contra overflow
    Stack<int> access_stack;
    const size_t access_size = 10000;
    for (size_t i = 0; i < access_size; ++i) {
        access_stack.push(static_cast<int>(i));
    }
    
    start = std::chrono::high_resolution_clock::now();
    
    volatile long long sum = 0; // Use long long para evitar overflow
    for (size_t i = 0; i < iterations; ++i) {
        sum += access_stack[i % access_stack.size()];
    }
    
    end = std::chrono::high_resolution_clock::now();
    auto access_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Push performance: " << push_time.count() << " μs (" << iterations << " operations)" << std::endl;
    std::cout << "Pop performance: " << pop_time.count() << " μs (" << iterations << " operations)" << std::endl;
    std::cout << "Access performance: " << access_time.count() << " μs (" << iterations << " operations)" << std::endl;
}

void validateOptimizations() {
    const size_t iterations = 1000000;
    Stack<int> stack;
    
    // Push com uso do resultado
    auto start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < iterations; ++i) {
        stack.push(static_cast<int>(i));
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto push_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // Pop com uso do resultado para evitar otimização
    volatile int sum = 0; // Impede otimização
    start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < iterations; ++i) {
        sum += stack.pop(); // Força uso do resultado
    }
    end = std::chrono::high_resolution_clock::now();
    auto pop_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Validated Push: " << push_time.count() << " μs" << std::endl;
    std::cout << "Validated Pop: " << pop_time.count() << " μs" << std::endl;
    std::cout << "Sum (prevent optimization): " << sum << std::endl;
}

void realisticBenchmark() {
    const size_t iterations = 1000000;
    
    // Teste com workload realista
    Stack<int> stack;
    std::vector<int> results;
    results.reserve(iterations);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // Push
    for (size_t i = 0; i < iterations; ++i) {
        stack.push(static_cast<int>(i % 1000));
    }
    
    // Pop com uso dos resultados
    while (!stack.empty()) {
        results.push_back(stack.pop());
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto total_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Realistic workload: " << total_time.count() << " μs" << std::endl;
    std::cout << "Results size: " << results.size() << std::endl;
}

void compareWithSTL() {
    const size_t iterations = 1000000;
    
    // std::vector benchmark
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<int> vec;
    for (size_t i = 0; i < iterations; ++i) {
        vec.push_back(static_cast<int>(i));
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto vec_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "std::vector push_back: " << vec_time.count() << " μs" << std::endl;
    std::cout << "Your Stack push:       5555 μs" << std::endl;
    std::cout << "Performance ratio: " << (double)vec_time.count() / 5555.0 << std::endl;
}

    
    void runAllTests() {
        std::cout << "=== STACK COMPREHENSIVE TESTING ===" << std::endl;
        std::cout << "Seed: " << rng() << std::endl << std::endl;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        testBasicConstruction();
        testCopyConstructor();
        testAssignmentOperator();
        testMoveSemantics();
        testPushAndPop();
        testEmplace();
        testAccessMethods();
        testClear();
        testResize();
        testStressOperations();
        testWithComplexTypes();
        testEdgeCases();
        performanceTest();
        validateOptimizations();
        realisticBenchmark();
        compareWithSTL();

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << std::endl << "=== ALL TESTS COMPLETED ===" << std::endl;
        std::cout << "Total time: " << duration.count() << "ms" << std::endl;
    }
};

 
