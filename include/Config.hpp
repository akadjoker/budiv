#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <stddef.h>
#include <cstddef> // for std::nullptr_t
#include <cstdio>

#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <cstdio>

#include <cassert>
#include <time.h>

#include <stdarg.h>
//#include <stdexcept>


#include <cmath>

typedef uint8_t  u8;
typedef int8_t   c8;
typedef uint16_t u16;
typedef int16_t  s16;
typedef uint32_t u32;
typedef int32_t  s32;
typedef int64_t  s64;
typedef uint64_t u64;

#if defined(_DEBUG)
#include <assert.h>
//#define DEBUG_BREAK_IF(_CONDITION_) assert(!(_CONDITION_));
#define DEBUG_BREAK_IF(condition) if (condition) { fprintf(stderr, "Debug break: %s at %s:%d\n", #condition, __FILE__, __LINE__); std::exit(EXIT_FAILURE); }
#else
#define DEBUG_BREAK_IF(_CONDITION_)
#endif

inline size_t CalculateCapacityGrow(size_t capacity, size_t minCapacity)
{
    if (capacity < minCapacity)
        capacity = minCapacity;
    if (capacity < 8)
    {
        capacity = 8;
    }
    else
    {
        // Round up to the next power of 2 and multiply by 2 (http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2)
        capacity--;
        capacity |= capacity >> 1;
        capacity |= capacity >> 2;
        capacity |= capacity >> 4;
        capacity |= capacity >> 8;
        capacity |= capacity >> 16;
        capacity++;
    }
    return capacity;
}

 

template <typename T>
class Allocator
{
public:

    T *allocate(size_t n)
    {
        return static_cast<T *>(::operator new(n * sizeof(T)));
    }

    void deallocate(T *p, size_t n)
    {
        ::operator delete(p, n * sizeof(T));
    }

    template <typename... Args>
    void construct(T *p, Args &&...args)
    {
        new (p) T(std::forward<Args>(args)...);
    }

    void destroy(T *p)
    {

        p->~T();
    }
};

template <typename T, size_t PoolSize = 1024>
class PoolAllocator {
private:
    struct Block {
        alignas(T) char data[sizeof(T)];
    };
    
    static Block pool[PoolSize];
    static bool used[PoolSize];
    static size_t next_free;
    
public:
    T* allocate(size_t n) {
        if (n != 1) {
            // Fallback para malloc
            return static_cast<T*>(malloc(n * sizeof(T)));
        }
        
        // Busca linear simples (sem bitset da STL)
        for (size_t i = 0; i < PoolSize; ++i) {
            size_t idx = (next_free + i) % PoolSize;
            if (!used[idx]) {
                used[idx] = true;
                next_free = (idx + 1) % PoolSize;
                return reinterpret_cast<T*>(&pool[idx]);
            }
        }
        
        // Pool cheio, usar heap
        return static_cast<T*>(malloc(sizeof(T)));
    }
    
    void deallocate(T* p, size_t n) {
        if (n != 1) {
            free(p);
            return;
        }
        
        // Verificar se está no pool
        if (p >= reinterpret_cast<T*>(&pool[0]) && 
            p < reinterpret_cast<T*>(&pool[PoolSize])) {
            size_t idx = reinterpret_cast<Block*>(p) - &pool[0];
            used[idx] = false;
        } else {
            free(p);
        }
    }
    
    template <typename... Args>
    void construct(T* p, Args&&... args) {
        ::new (p) T(static_cast<Args&&>(args)...);
    }
    
    void destroy(T* p) {
        p->~T();
    }
};

 
template <typename T, size_t PoolSize>
typename PoolAllocator<T, PoolSize>::Block PoolAllocator<T, PoolSize>::pool[PoolSize];

template <typename T, size_t PoolSize>
bool PoolAllocator<T, PoolSize>::used[PoolSize] = {false};

template <typename T, size_t PoolSize>
size_t PoolAllocator<T, PoolSize>::next_free = 0;
