#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <cassert>
#include <string>
#include <memory>
#include <thread>
#include <atomic>
#include "Raii.hpp"

class SmartPointerTester {
private:
    std::mt19937 rng;
    std::uniform_int_distribution<int> value_dist;
    
public:
    SmartPointerTester() : rng(std::chrono::steady_clock::now().time_since_epoch().count()),
                           value_dist(1, 1000) {}
    
    void testSharedPtrBasicConstruction() {
        std::cout << "Testing SharedPtr basic construction..." << std::endl;
        
        // Test default construction
        SharedPtr<int> ptr1;
        assert(ptr1.is_null());
        assert(ptr1.use_count() == 0);
        assert(!ptr1);
        
        // Test construction with pointer
        SharedPtr<int> ptr2(new int(42));
        assert(!ptr2.is_null());
        assert(ptr2.use_count() == 1);
        assert(ptr2);
        assert(*ptr2 == 42);
        
        // Test nullptr construction
        SharedPtr<int> ptr3(nullptr);
        assert(ptr3.is_null());
        assert(ptr3.use_count() == 0);
        
        std::cout << "SharedPtr basic construction: PASSED" << std::endl;
    }
    
    void testSharedPtrCopySemantics() {
        std::cout << "Testing SharedPtr copy semantics..." << std::endl;
        
        SharedPtr<int> ptr1(new int(100));
        assert(ptr1.use_count() == 1);
        
        // Test copy constructor
        SharedPtr<int> ptr2(ptr1);
        assert(ptr1.use_count() == 2);
        assert(ptr2.use_count() == 2);
        assert(*ptr1 == *ptr2);
        assert(ptr1.get() == ptr2.get());
        
        // Test copy assignment
        SharedPtr<int> ptr3;
        ptr3 = ptr1;
        assert(ptr1.use_count() == 3);
        assert(ptr2.use_count() == 3);
        assert(ptr3.use_count() == 3);
        
        // Test self-assignment
        ptr1 = ptr1;
        assert(ptr1.use_count() == 3);
        
        std::cout << "SharedPtr copy semantics: PASSED" << std::endl;
    }
    
    void testSharedPtrReferenceCount() {
        std::cout << "Testing SharedPtr reference counting..." << std::endl;
        
        SharedPtr<std::string> ptr1(new std::string("Hello"));
        assert(ptr1.use_count() == 1);
        assert(ptr1.unique());
        
        {
            SharedPtr<std::string> ptr2 = ptr1;
            assert(ptr1.use_count() == 2);
            assert(ptr2.use_count() == 2);
            assert(!ptr1.unique());
            assert(!ptr2.unique());
            
            {
                SharedPtr<std::string> ptr3(ptr1);
                assert(ptr1.use_count() == 3);
                assert(ptr2.use_count() == 3);
                assert(ptr3.use_count() == 3);
            } // ptr3 goes out of scope
            
            assert(ptr1.use_count() == 2);
            assert(ptr2.use_count() == 2);
        } // ptr2 goes out of scope
        
        assert(ptr1.use_count() == 1);
        assert(ptr1.unique());
        
        std::cout << "SharedPtr reference counting: PASSED" << std::endl;
    }
    
    void testSharedPtrNullptrAssignment() {
        std::cout << "Testing SharedPtr nullptr assignment..." << std::endl;
        
        SharedPtr<int> ptr1(new int(42));
        SharedPtr<int> ptr2 = ptr1;
        
        assert(ptr1.use_count() == 2);
        assert(ptr2.use_count() == 2);
        
        // Assign nullptr to one pointer
        ptr1 = nullptr;
        assert(ptr1.is_null());
        assert(ptr1.use_count() == 0);
        assert(ptr2.use_count() == 1);
        assert(*ptr2 == 42);
        
        std::cout << "SharedPtr nullptr assignment: PASSED" << std::endl;
    }
    
    void testUniquePtrBasicConstruction() {
        std::cout << "Testing UniquePtr basic construction..." << std::endl;
        
        // Test default construction
        UniquePtr<int> ptr1;
        assert(ptr1.is_null());
        assert(!ptr1);
        
        // Test construction with pointer
        UniquePtr<int> ptr2(new int(42));
        assert(!ptr2.is_null());
        assert(ptr2);
        assert(*ptr2 == 42);
        
        // Test nullptr construction
        UniquePtr<int> ptr3(nullptr);
        assert(ptr3.is_null());
        
        std::cout << "UniquePtr basic construction: PASSED" << std::endl;
    }
    
    void testUniquePtrMoveSemantics() {
        std::cout << "Testing UniquePtr move semantics..." << std::endl;
        
        UniquePtr<std::string> ptr1(new std::string("Hello"));
        std::string* raw_ptr = ptr1.get();
        
        // Test move constructor
        UniquePtr<std::string> ptr2(std::move(ptr1));
        assert(ptr1.is_null());
        assert(!ptr2.is_null());
        assert(ptr2.get() == raw_ptr);
        assert(*ptr2 == "Hello");
        
        // Test move assignment
        UniquePtr<std::string> ptr3;
        ptr3 = std::move(ptr2);
        assert(ptr2.is_null());
        assert(!ptr3.is_null());
        assert(ptr3.get() == raw_ptr);
        assert(*ptr3 == "Hello");
        
        std::cout << "UniquePtr move semantics: PASSED" << std::endl;
    }
    
    void testUniquePtrReleaseAndReset() {
        std::cout << "Testing UniquePtr release and reset..." << std::endl;
        
        UniquePtr<int> ptr1(new int(42));
        int* raw_ptr = ptr1.release();
        
        assert(ptr1.is_null());
        assert(*raw_ptr == 42);
        
        // Test reset
        ptr1.reset(raw_ptr);
        assert(!ptr1.is_null());
        assert(*ptr1 == 42);
        
        // Test reset with new pointer
        ptr1.reset(new int(100));
        assert(*ptr1 == 100);
        
        // Test reset with nullptr
        ptr1.reset();
        assert(ptr1.is_null());
        
        std::cout << "UniquePtr release and reset: PASSED" << std::endl;
    }
    
    void testMakeUnique() {
        std::cout << "Testing Make_Unique..." << std::endl;
        
        auto ptr1 = Make_Unique<int>(42);
        assert(*ptr1 == 42);
        
        auto ptr2 = Make_Unique<std::string>("Hello World");
        assert(*ptr2 == "Hello World");
        
        auto ptr3 = Make_Unique<std::vector<int>>(5, 10);
        assert(ptr3->size() == 5);
        assert((*ptr3)[0] == 10);
        
        std::cout << "Make_Unique: PASSED" << std::endl;
    }
    
    void testMakeShared() {
        std::cout << "Testing Make_Shared..." << std::endl;
        
        auto ptr1 = Make_Shared<int>(42);
        assert(*ptr1 == 42);
        assert(ptr1.use_count() == 1);
        
        auto ptr2 = Make_Shared<std::string>("Hello World");
        assert(*ptr2 == "Hello World");
        
        auto ptr3 = Make_Shared<std::vector<int>>(5, 10);
        assert(ptr3->size() == 5);
        assert((*ptr3)[0] == 10);
        
        std::cout << "Make_Shared: PASSED" << std::endl;
    }
    
    void testPolymorphicConversion() {
        std::cout << "Testing polymorphic conversion..." << std::endl;
        
        struct Base {
            virtual ~Base() = default;
            virtual int getValue() const { return 1; }
        };
        
        struct Derived : public Base {
            int getValue() const override { return 2; }
        };
        
        // Test SharedPtr polymorphic conversion
        SharedPtr<Derived> derived_shared(new Derived);
        SharedPtr<Base> base_shared = derived_shared;
        
        assert(base_shared.use_count() == 2);
        assert(derived_shared.use_count() == 2);
        assert(base_shared->getValue() == 2);
        
        // Test UniquePtr polymorphic conversion
        UniquePtr<Derived> derived_unique(new Derived);
        UniquePtr<Base> base_unique = std::move(derived_unique);
        
        assert(derived_unique.is_null());
        assert(!base_unique.is_null());
        assert(base_unique->getValue() == 2);
        
        std::cout << "Polymorphic conversion: PASSED" << std::endl;
    }
    
    void testMemoryLeaks() {
        std::cout << "Testing memory leak prevention..." << std::endl;
        
        // Test with tracking object
        static int construction_count = 0;
        static int destruction_count = 0;
        
        struct TrackingObject {
            TrackingObject() { construction_count++; }
            ~TrackingObject() { destruction_count++; }
        };
        
        construction_count = 0;
        destruction_count = 0;
        
        {
            // Test SharedPtr
            SharedPtr<TrackingObject> ptr1(new TrackingObject);
            {
                SharedPtr<TrackingObject> ptr2 = ptr1;
                SharedPtr<TrackingObject> ptr3 = ptr2;
            } // ptr2 and ptr3 go out of scope
        } // ptr1 goes out of scope
        
        assert(construction_count == 1);
        assert(destruction_count == 1);
        
        construction_count = 0;
        destruction_count = 0;
        
        {
            // Test UniquePtr
            UniquePtr<TrackingObject> ptr1(new TrackingObject);
            UniquePtr<TrackingObject> ptr2 = std::move(ptr1);
        } // ptr2 goes out of scope
        
        assert(construction_count == 1);
        assert(destruction_count == 1);
        
        std::cout << "Memory leak prevention: PASSED" << std::endl;
    }
    
    void testStressOperations(int iterations = 10000) {
        std::cout << "Testing stress operations (" << iterations << " iterations)..." << std::endl;
        
        std::vector<SharedPtr<int>> shared_ptrs;
        std::vector<UniquePtr<int>> unique_ptrs;
        
        // Stress test SharedPtr
        for (int i = 0; i < iterations; ++i) {
            int value = value_dist(rng);
            shared_ptrs.push_back(Make_Shared<int>(value));
            
            if (i > 0 && i % 100 == 0) {
                // Create copies to test reference counting
                for (int j = 0; j < 10 && j < shared_ptrs.size(); ++j) {
                    SharedPtr<int> copy = shared_ptrs[j];
                    assert(copy.use_count() >= 2);
                }
            }
        }
        
        // Stress test UniquePtr
        for (int i = 0; i < iterations; ++i) {
            int value = value_dist(rng);
            unique_ptrs.push_back(Make_Unique<int>(value));
            
            if (i > 0 && i % 100 == 0) {
                // Move some pointers
                UniquePtr<int> moved = std::move(unique_ptrs[i - 1]);
                assert(unique_ptrs[i - 1].is_null());
                assert(!moved.is_null());
                unique_ptrs[i - 1] = std::move(moved);
            }
        }
        
        std::cout << "Stress operations: PASSED" << std::endl;
    }
    
    void performanceTest() {
        std::cout << "\n=== PERFORMANCE TESTING ===" << std::endl;
        
        const size_t iterations = 1000000;
        
        // Test SharedPtr creation performance
        auto start = std::chrono::high_resolution_clock::now();
        
        for (size_t i = 0; i < iterations; ++i) {
            SharedPtr<int> ptr(new int(static_cast<int>(i)));
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto shared_creation_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        // Test UniquePtr creation performance
        start = std::chrono::high_resolution_clock::now();
        
        for (size_t i = 0; i < iterations; ++i) {
            UniquePtr<int> ptr(new int(static_cast<int>(i)));
        }
        
        end = std::chrono::high_resolution_clock::now();
        auto unique_creation_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        // Test std::shared_ptr for comparison
        start = std::chrono::high_resolution_clock::now();
        
        for (size_t i = 0; i < iterations; ++i) {
            std::shared_ptr<int> ptr(new int(static_cast<int>(i)));
        }
        
        end = std::chrono::high_resolution_clock::now();
        auto std_shared_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        // Test std::unique_ptr for comparison
        start = std::chrono::high_resolution_clock::now();
        
        for (size_t i = 0; i < iterations; ++i) {
            std::unique_ptr<int> ptr(new int(static_cast<int>(i)));
        }
        
        end = std::chrono::high_resolution_clock::now();
        auto std_unique_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        std::cout << "SharedPtr creation: " << shared_creation_time.count() << " μs (" << iterations << " operations)" << std::endl;
        std::cout << "UniquePtr creation: " << unique_creation_time.count() << " μs (" << iterations << " operations)" << std::endl;
        std::cout << "std::shared_ptr: " << std_shared_time.count() << " μs" << std::endl;
        std::cout << "std::unique_ptr: " << std_unique_time.count() << " μs" << std::endl;
        
        std::cout << "SharedPtr ratio: " << (double)std_shared_time.count() / shared_creation_time.count() << std::endl;
        std::cout << "UniquePtr ratio: " << (double)std_unique_time.count() / unique_creation_time.count() << std::endl;
    }

    void validateOptimizations() {
    volatile int sum = 0;  // Impede otimização
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (size_t i = 0; i < 1000000; ++i) {
        SharedPtr<int> ptr(new int(static_cast<int>(i)));
        sum += *ptr;  // Força uso do valor
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Validated SharedPtr: " << time.count() << " μs" << std::endl;
    std::cout << "Sum (prevent optimization): " << sum << std::endl;
}

    
    void runAllTests() {
        std::cout << "=== SMART POINTERS COMPREHENSIVE TESTING ===" << std::endl;
        std::cout << "Seed: " << rng() << std::endl << std::endl;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        testSharedPtrBasicConstruction();
        testSharedPtrCopySemantics();
        testSharedPtrReferenceCount();
        testSharedPtrNullptrAssignment();
        testUniquePtrBasicConstruction();
        testUniquePtrMoveSemantics();
        testUniquePtrReleaseAndReset();
        testMakeUnique();
        testMakeShared();
        testPolymorphicConversion();
        testMemoryLeaks();
        testStressOperations();
        performanceTest();
        validateOptimizations();

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << std::endl << "=== ALL TESTS COMPLETED ===" << std::endl;
        std::cout << "Total time: " << duration.count() << "ms" << std::endl;
    }
};

 