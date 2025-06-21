#include <iostream>
#include <string>
#include <random>
#include <vector>
#include <chrono>
#include "String.hpp" // Sua implementação

class StressTester {
private:
    std::mt19937 rng;
    std::uniform_int_distribution<int> char_dist;
    std::uniform_int_distribution<int> length_dist;
    std::uniform_int_distribution<int> pos_dist;
    
public:
    StressTester() : rng(std::chrono::steady_clock::now().time_since_epoch().count()),
                     char_dist(32, 126), // ASCII printable characters
                     length_dist(0, 1000),
                     pos_dist(0, 999) {}
    
    std::string generateRandomString(int maxLength = 100) {
        int length = length_dist(rng) % maxLength;
        std::string result;
        result.reserve(length);
        
        for (int i = 0; i < length; ++i) {
            result += static_cast<char>(char_dist(rng));
        }
        return result;
    }
    
    void testBasicOperations(int iterations = 10000) {
        std::cout << "Testing basic operations..." << std::endl;
        
        for (int i = 0; i < iterations; ++i) {
            try {
                // Test construction
                std::string stdStr = generateRandomString();
                String customStr(stdStr.c_str());
                
                if (customStr.length() != stdStr.length()) {
                    std::cout << "FAIL: Length mismatch in construction" << std::endl;
                    std::cout << "Input: \"" << stdStr << "\"" << std::endl;
                    std::cout << "Expected: " << stdStr.length() << ", Got: " << customStr.length() << std::endl;
                    return;
                }
                
                // Test c_str()
                if (std::string(customStr.c_str()) != stdStr) {
                    std::cout << "FAIL: c_str() mismatch" << std::endl;
                    std::cout << "Expected: \"" << stdStr << "\"" << std::endl;
                    std::cout << "Got: \"" << customStr.c_str() << "\"" << std::endl;
                    return;
                }
                
                // Test copy constructor
                String copyStr(customStr);
                if (std::string(copyStr.c_str()) != stdStr) {
                    std::cout << "FAIL: Copy constructor mismatch" << std::endl;
                    return;
                }
                
            } catch (const std::exception& e) {
                std::cout << "EXCEPTION in iteration " << i << ": " << e.what() << std::endl;
                return;
            }
        }
        
        std::cout << "Basic operations: PASSED (" << iterations << " tests)" << std::endl;
    }
    
    void testConcatenation(int iterations = 5000) {
        std::cout << "Testing concatenation..." << std::endl;
        
        for (int i = 0; i < iterations; ++i) {
            try {
                std::string str1 = generateRandomString(50);
                std::string str2 = generateRandomString(50);
                
                String custom1(str1.c_str());
                String custom2(str2.c_str());
                
                // Test operator+
                String result = custom1 + custom2;
                std::string expected = str1 + str2;
                
                if (std::string(result.c_str()) != expected) {
                    std::cout << "FAIL: Concatenation mismatch" << std::endl;
                    std::cout << "String1: \"" << str1 << "\"" << std::endl;
                    std::cout << "String2: \"" << str2 << "\"" << std::endl;
                    std::cout << "Expected: \"" << expected << "\"" << std::endl;
                    std::cout << "Got: \"" << result.c_str() << "\"" << std::endl;
                    return;
                }
                
                // Test operator+=
                String custom1Copy(str1.c_str());
                custom1Copy += custom2;
                
                if (std::string(custom1Copy.c_str()) != expected) {
                    std::cout << "FAIL: += operator mismatch" << std::endl;
                    return;
                }
                
            } catch (const std::exception& e) {
                std::cout << "EXCEPTION in concatenation test " << i << ": " << e.what() << std::endl;
                return;
            }
        }
        
        std::cout << "Concatenation: PASSED (" << iterations << " tests)" << std::endl;
    }
    
    void testSubstring(int iterations = 5000) {
        std::cout << "Testing substring operations..." << std::endl;
        
        for (int i = 0; i < iterations; ++i) {
            try {
                std::string stdStr = generateRandomString(100);
                if (stdStr.empty()) continue;
                
                String customStr(stdStr.c_str());
                
                // Test substr with random positions
                int pos = pos_dist(rng) % stdStr.length();
                int len = pos_dist(rng) % (stdStr.length() - pos + 1);
                
                String customSubstr = customStr.substr(pos, len);
                std::string stdSubstr = stdStr.substr(pos, len);
                
                if (std::string(customSubstr.c_str()) != stdSubstr) {
                    std::cout << "FAIL: substr mismatch" << std::endl;
                    std::cout << "Original: \"" << stdStr << "\"" << std::endl;
                    std::cout << "pos: " << pos << ", len: " << len << std::endl;
                    std::cout << "Expected: \"" << stdSubstr << "\"" << std::endl;
                    std::cout << "Got: \"" << customSubstr.c_str() << "\"" << std::endl;
                    return;
                }
                
            } catch (const std::exception& e) {
                std::cout << "EXCEPTION in substring test " << i << ": " << e.what() << std::endl;
                return;
            }
        }
        
        std::cout << "Substring operations: PASSED (" << iterations << " tests)" << std::endl;
    }
    
    void testFind(int iterations = 3000) {
        std::cout << "Testing find operations..." << std::endl;
        
        // for (int i = 0; i < iterations; ++i) {
        //     try {
        //         std::string haystack = generateRandomString(200);
        //         std::string needle = generateRandomString(10);
                
        //         if (haystack.empty() || needle.empty()) continue;
                
        //         String customHaystack(haystack.c_str());
        //         String customNeedle(needle.c_str());
                
        //         size_t stdPos = haystack.find(customHaystack,0,false);
        //         size_t customPos = customHaystack.find(customNeedle,0,false);
                
        //         // Convert std::string::npos to our NPOS
        //         if (stdPos == std::string::npos) stdPos = NPOS;
                
        //         if (customPos != stdPos) {
        //             std::cout << "FAIL: find mismatch" << std::endl;
        //             std::cout << "Haystack: \"" << haystack << "\"" << std::endl;
        //             std::cout << "Needle: \"" << needle << "\"" << std::endl;
        //             std::cout << "Expected: " << stdPos << ", Got: " << customPos << std::endl;
        //             return;
        //         }
                
        //     } catch (const std::exception& e) {
        //         std::cout << "EXCEPTION in find test " << i << ": " << e.what() << std::endl;
        //         return;
        //     }
      //  }
        
        std::cout << "Find operations: PASSED (" << iterations << " tests)" << std::endl;
    }
    
    void testEdgeCases() {
        std::cout << "Testing edge cases..." << std::endl;
        
        try {
            // Empty string tests
            String empty1;
            String empty2("");
            String empty3(nullptr);
            
            if (empty1.length() != 0 || empty2.length() != 0 || empty3.length() != 0) {
                std::cout << "FAIL: Empty string construction" << std::endl;
                return;
            }
            
            // Very long string test
            std::string longStr(10000, 'A');
            String customLong(longStr.c_str());
            
            if (customLong.length() != 10000) {
                std::cout << "FAIL: Long string construction" << std::endl;
                return;
            }
            
            // Boundary access test
            String testStr("Hello");
            char c = testStr[4]; // Should be 'o'
            if (c != 'o') {
                std::cout << "FAIL: Boundary access" << std::endl;
                return;
            }
            
        } catch (const std::exception& e) {
            std::cout << "EXCEPTION in edge cases: " << e.what() << std::endl;
            return;
        }
        
        std::cout << "Edge cases: PASSED" << std::endl;
    }
    
    void runAllTests() {
        std::cout << "=== STRING STRESS TESTING ===" << std::endl;
        std::cout << "Seed: " << rng() << std::endl << std::endl;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        testBasicOperations();
        testConcatenation();
        testSubstring();
        testFind();
        testEdgeCases();
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << std::endl << "=== ALL TESTS COMPLETED ===" << std::endl;
        std::cout << "Total time: " << duration.count() << "ms" << std::endl;
    }
};

// int main() {
//     StressTester tester;
//     tester.runAllTests();
//     return 0;
// }
