#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <cassert>
#include <memory>
#include <climits>
#include "Chunk.hpp"

class ChunkTester {
private:
    std::mt19937 rng;
    std::uniform_int_distribution<u8> instruction_dist;
    std::uniform_int_distribution<int> line_dist;
    
public:
    ChunkTester() : rng(std::chrono::steady_clock::now().time_since_epoch().count()),
                    instruction_dist(0, 255),
                    line_dist(1, 10000) {}
    
    void testBasicConstruction() {
        std::cout << "Testing basic construction..." << std::endl;
        
        // Test default construction
        Chunk chunk1;
        assert(chunk1.count == 0);

        {
            Chunk chunk2(10);
            assert(chunk2.count == 0);
            assert(chunk2.capacity() == 10);
            assert(chunk2.code != nullptr);
            assert(chunk2.lines != nullptr);
        }

        {
            Chunk chunk3(100);
            assert(chunk3.count == 0);
            assert(chunk3.capacity() == 100);
            assert(chunk3.code != nullptr);
            assert(chunk3.lines != nullptr);
        }

        {
            Chunk chunk4(1000);
            assert(chunk4.count == 0);
            assert(chunk4.capacity() == 1000);
            assert(chunk4.code != nullptr);
            assert(chunk4.lines != nullptr);
        }

        {
            Chunk chunk5(10000);
            assert(chunk5.count == 0);
            assert(chunk5.capacity() == 10000);
            assert(chunk5.code != nullptr);
            assert(chunk5.lines != nullptr);
        }

        
        
        
        std::cout << "Basic construction: PASSED" << std::endl;
    }
    
    void testCopyConstructor() {
        std::cout << "Testing copy constructor..." << std::endl;
        
        // Create original chunk with some data
        Chunk original(100);
        for (int i = 0; i < 50; ++i) {
            original.write(static_cast<u8>(i), i * 10);
        }
        
        // Test copy constructor
        Chunk copy(&original);
        
        // Verify copy
        assert(copy.count == original.count);
        assert(copy.capacity() == original.capacity());
        assert(copy.code != original.code); // Different memory
        assert(copy.lines != original.lines); // Different memory
        
        // Verify data integrity
        for (u32 i = 0; i < original.count; ++i) {
            assert(copy.code[i] == original.code[i]);
            assert(copy.lines[i] == original.lines[i]);
        }
        
        std::cout << "Copy constructor: PASSED" << std::endl;
    }
    
    void testCloneMethod() {
        std::cout << "Testing clone method..." << std::endl;
        
        // Create source chunk
        Chunk source(50);
        for (int i = 0; i < 25; ++i) {
            source.write(static_cast<u8>(i * 2), i * 100);
        }
        
        // Create target chunk
        Chunk target(10);
        target.write(99, 999); // Add some data to be overwritten
        
        // Test clone
        bool result = source.clone(&target);
        assert(result == true);
        
        // Verify clone
        assert(target.count == source.count);
        assert(target.capacity() == source.capacity());
        
        for (u32 i = 0; i < source.count; ++i) {
            assert(target.code[i] == source.code[i]);
            assert(target.lines[i] == source.lines[i]);
        }
        
        // Test clone with nullptr
        result = source.clone(nullptr);
        assert(result == false);
        
        std::cout << "Clone method: PASSED" << std::endl;
    }
    
    void testWriteAndAccess() {
        std::cout << "Testing write and access..." << std::endl;
        
        Chunk chunk(5);
        
        // Test basic write
        chunk.write(0x42, 100);
        assert(chunk.count == 1);
        assert(chunk[0] == 0x42);
        assert(chunk.lines[0] == 100);


        u8 instructions[] = {0x10, 0x20, 0x30, 0x40};
        int lines[] = {200, 300, 400, 500};
        
        // Test multiple writes
        for (size_t i = 0; i < 4; ++i) {
            chunk.write(instructions[i], lines[i]);
        }
        
        assert(chunk.count == 5);
        
        // Verify all data
        assert(chunk[0] == 0x42);
        for (size_t i = 0; i < 4; ++i) {
            assert(chunk[i + 1] == instructions[i]);
            assert(chunk.lines[i + 1] == lines[i]);
        }
        
  
        
        std::cout << "Write and access: PASSED" << std::endl;
    }
    
    void testCapacityGrowth() {
        std::cout << "Testing capacity growth..." << std::endl;
        
        Chunk chunk(2); // Small initial capacity
        
        // Fill beyond initial capacity
        for (int i = 0; i < 10; ++i) {
            u32 old_capacity = chunk.capacity();
            chunk.write(static_cast<u8>(i), i * 10);
            
            // Verify capacity grows when needed
            if (i >= 2) {
                assert(chunk.capacity() > old_capacity || chunk.capacity() >= chunk.count);
            }
        }
        
        assert(chunk.count == 10);
        
        // Verify all data is preserved
        for (int i = 0; i < 10; ++i) {
            assert(chunk[i] == static_cast<u8>(i));
            assert(chunk.lines[i] == i * 10);
        }
        
        std::cout << "Capacity growth: PASSED" << std::endl;
    }
    
    void testReserve() {
        std::cout << "Testing reserve..." << std::endl;
        
        Chunk chunk(10);
        
        // Add some data
        for (int i = 0; i < 5; ++i) {
            chunk.write(static_cast<u8>(i), i);
        }
        
 
        
        // Reserve larger capacity
        chunk.reserve(100);
        assert(chunk.capacity() >= 100);
        assert(chunk.count == 5);
        
        // Verify data integrity
        for (int i = 0; i < 5; ++i) {
            assert(chunk[i] == static_cast<u8>(i));
            assert(chunk.lines[i] == i);
        }
        
        // Reserve smaller capacity (should not shrink)
        chunk.reserve(5);
        assert(chunk.capacity() >= 100);
        
        std::cout << "Reserve: PASSED" << std::endl;
    }
    
    void testStressWrite(int iterations = 10000) {
        std::cout << "Testing stress write (" << iterations << " iterations)..." << std::endl;
        
        Chunk chunk(1); // Start small to force many reallocations

        u8 expected_instructions[10000];
        int expected_lines[10000];
        


        
        // std::vector<u8> expected_instructions;
        // std::vector<int> expected_lines;
        
        for (int i = 0; i < iterations; ++i) {
            u8 instruction = instruction_dist(rng);
            int line = line_dist(rng);
            
            chunk.write(instruction, line);
            
            expected_instructions[i] = instruction;
            expected_lines[i] = line;

            //expected_instructions.push_back(instruction);
           // expected_lines.push_back(line);
            
            // Verify count
            assert(chunk.count == static_cast<u32>(i + 1));
        }
        
        // Verify all data
        for (int i = 0; i < iterations; ++i) 
        {
            assert(chunk[i] == expected_instructions[i]);
            assert(chunk.lines[i] == expected_lines[i]);
        }
        
        std::cout << "Stress write: PASSED" << std::endl;
    }
    
    void testEdgeCases() {
        std::cout << "Testing edge cases..." << std::endl;
        
        // Test with capacity 0 (should still work)
        Chunk chunk;
        chunk.write(0xFF, 1);
        assert(chunk.count == 1);
        assert(chunk[0] == 0xFF);
        
        // Test with very large capacity
        Chunk large_chunk(1000000);
        large_chunk.write(0xAA, 12345);
        assert(large_chunk.count == 1);
        assert(large_chunk[0] == 0xAA);
        
        // Test boundary values
        Chunk boundary_chunk(10);
        boundary_chunk.write(0x00, INT_MIN);
        boundary_chunk.write(0xFF, INT_MAX);
        
        assert(boundary_chunk[0] == 0x00);
        assert(boundary_chunk[1] == 0xFF);
        assert(boundary_chunk.lines[0] == INT_MIN);
        assert(boundary_chunk.lines[1] == INT_MAX);
        
        std::cout << "Edge cases: PASSED" << std::endl;
    }
    
    void testMemoryIntegrity() {
        std::cout << "Testing memory integrity..." << std::endl;
        
        const int num_chunks = 100;
        std::vector<std::unique_ptr<Chunk>> chunks;
        
        // Create many chunks
        for (int i = 0; i < num_chunks; ++i) {
            chunks.push_back(std::make_unique<Chunk>(i + 1));
            
            // Fill with unique data
            for (int j = 0; j <= i; ++j) {
                chunks[i]->write(static_cast<u8>(i + j), i * 1000 + j);
            }
        }
        
        // Verify all chunks maintain their data
        for (int i = 0; i < num_chunks; ++i) {
            assert(chunks[i]->count == static_cast<u32>(i + 1));
            
            for (int j = 0; j <= i; ++j) {
                assert((*chunks[i])[j] == static_cast<u8>(i + j));
                assert(chunks[i]->lines[j] == i * 1000 + j);
            }
        }
        
        std::cout << "Memory integrity: PASSED" << std::endl;
    }
    
    void performanceTest() {
        std::cout << "\n=== PERFORMANCE TESTING ===" << std::endl;
        
        const int iterations = 1000000;
        
        // Test write performance
        auto start = std::chrono::high_resolution_clock::now();
        
        Chunk chunk(1);
        for (int i = 0; i < iterations; ++i) {
            chunk.write(static_cast<u8>(i & 0xFF), i);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        std::cout << "Write performance: " << duration.count() << " μs (" 
                  << iterations << " writes)" << std::endl;
        std::cout << "Average: " << (double)duration.count() / iterations << " μs per write" << std::endl;
        
        // Test access performance
        start = std::chrono::high_resolution_clock::now();
        
        volatile u8 sum = 0; // Prevent optimization
        for (int i = 0; i < iterations; ++i) {
            sum += chunk[i % chunk.count];
        }
        
        end = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        std::cout << "Access performance: " << duration.count() << " μs (" 
                  << iterations << " accesses)" << std::endl;
        std::cout << "Average: " << (double)duration.count() / iterations << " μs per access" << std::endl;
    }
    
    void runAllTests() {
        std::cout << "=== CHUNK STRESS TESTING ===" << std::endl;
        std::cout << "Seed: " << rng() << std::endl << std::endl;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        testBasicConstruction();
         testCopyConstructor();
        testCloneMethod();
        testWriteAndAccess();
        testCapacityGrowth();
        testReserve();
        testStressWrite();
        testEdgeCases();
        testMemoryIntegrity();
        performanceTest();
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << std::endl << "=== ALL TESTS COMPLETED ===" << std::endl;
        std::cout << "Total time: " << duration.count() << "ms" << std::endl;
    }
};

 
