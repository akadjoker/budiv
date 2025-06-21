#pragma once
#include "Config.hpp"
#include "Vector.hpp"
#include <memory>
template <typename Key, typename Value,
          typename KeyAlloc = Allocator<Key>,
          typename ValueAlloc =Allocator<Value>>
class UnorderedMap {
public:
    struct KeyValuePair
    {
        Key key;
        Value value;
        bool is_occupied;

        KeyValuePair(): is_occupied(false) {}
        KeyValuePair(const Key& k, const Value& v)
            : key(k), value(v), is_occupied(true)
        {}
        KeyValuePair(Key&& k, Value&& v)
            : key(std::move(k)), value(std::move(v)), is_occupied(true)
        {}
    };

    UnorderedMap(size_t initial_capacity = 16);
    ~UnorderedMap();
    UnorderedMap(const UnorderedMap& other);
    UnorderedMap& operator=(const UnorderedMap& other);
    UnorderedMap(UnorderedMap&& other) noexcept;
    UnorderedMap& operator=(UnorderedMap&& other) noexcept;

    // Core operations
    void insert(const Key& key, const Value& value);
    void insert(Key&& key, Value&& value);
    template <typename... Args> void emplace(const Key& key, Args&&... args);

    bool erase(const Key& key);
    void clear();

    Value* find(const Key& key);
    const Value* find(const Key& key) const;
    bool contains(const Key& key) const;
    bool contains(const Key& key, const Value& value) const;

    Value& operator[](const Key& key);
    const Value& at(const Key& key) const;

    size_t size() const;
    size_t capacity() const;
    bool empty() const;
    float load_factor() const;

    // Iteration support
    Vector<KeyValuePair> get_all_pairs() const;

private:
    KeyValuePair* buckets;
    size_t sz;
    size_t cap;
    KeyAlloc key_alloc;
    ValueAlloc value_alloc;

    static constexpr float MAX_LOAD_FACTOR =0.5f; // Reduzir para menos colisões

    size_t hash_function(const Key& key) const;
    size_t find_slot(const Key& key) const;
    size_t find_empty_slot(const Key& key) const;
    void rehash();
    void destroy_elements();
    void copy_from(const UnorderedMap& other);

    // Prefetch hint para melhor cache performance
    void prefetch_slot(size_t slot) const
    {
#ifdef __builtin_prefetch
        __builtin_prefetch(&buckets[slot], 0, 1);
#endif
    }
};

template <typename Key, typename Value, typename KeyAlloc, typename ValueAlloc>
UnorderedMap<Key, Value, KeyAlloc, ValueAlloc>::UnorderedMap(
    size_t initial_capacity)
    : buckets(nullptr), sz(0), cap(initial_capacity)
{
    // Ensure capacity is power of 2 for fast modulo
    if (cap < 16) cap = 16;

    // Round up to next power of 2
    cap = 1;
    while (cap < initial_capacity) cap <<= 1;

    buckets = static_cast<KeyValuePair*>(
        std::aligned_alloc(alignof(KeyValuePair), cap * sizeof(KeyValuePair)));

    // Initialize all buckets as unoccupied
    for (size_t i = 0; i < cap; ++i)
    {
        new (&buckets[i]) KeyValuePair();
    }
}

template <typename Key, typename Value, typename KeyAlloc, typename ValueAlloc>
UnorderedMap<Key, Value, KeyAlloc, ValueAlloc>::~UnorderedMap()
{
    destroy_elements();
    if (buckets)
    {
        std::free(buckets);
    }
}

template <typename Key, typename Value, typename KeyAlloc, typename ValueAlloc>
UnorderedMap<Key, Value, KeyAlloc, ValueAlloc>::UnorderedMap(
    const UnorderedMap& other)
    : buckets(nullptr), sz(0), cap(0)
{
    copy_from(other);
}

template <typename Key, typename Value, typename KeyAlloc, typename ValueAlloc>
UnorderedMap<Key, Value, KeyAlloc, ValueAlloc>&
UnorderedMap<Key, Value, KeyAlloc, ValueAlloc>::operator=(
    const UnorderedMap& other)
{
    if (this != &other)
    {
        destroy_elements();
        if (buckets)
        {
            std::free(buckets);
        }
        copy_from(other);
    }
    return *this;
}

template <typename Key, typename Value, typename KeyAlloc, typename ValueAlloc>
UnorderedMap<Key, Value, KeyAlloc, ValueAlloc>::UnorderedMap(
    UnorderedMap&& other) noexcept
    : buckets(other.buckets), sz(other.sz), cap(other.cap)
{
    other.buckets = nullptr;
    other.sz = 0;
    other.cap = 0;
}

template <typename Key, typename Value, typename KeyAlloc, typename ValueAlloc>
UnorderedMap<Key, Value, KeyAlloc, ValueAlloc>&
UnorderedMap<Key, Value, KeyAlloc, ValueAlloc>::operator=(
    UnorderedMap&& other) noexcept
{
    if (this != &other)
    {
        destroy_elements();
        if (buckets)
        {
            std::free(buckets);
        }

        buckets = other.buckets;
        sz = other.sz;
        cap = other.cap;

        other.buckets = nullptr;
        other.sz = 0;
        other.cap = 0;
    }
    return *this;
}

template <typename Key, typename Value, typename KeyAlloc, typename ValueAlloc>
void UnorderedMap<Key, Value, KeyAlloc, ValueAlloc>::insert(const Key& key,
                                                            const Value& value)
{
    if (load_factor() >= MAX_LOAD_FACTOR)
    {
        rehash();
    }

    size_t slot = find_slot(key);
    if (buckets[slot].is_occupied && buckets[slot].key == key)
    {
        // Update existing value
        buckets[slot].value = value;
    }
    else
    {
        // Find empty slot and insert new pair
        slot = find_empty_slot(key);
        buckets[slot] = KeyValuePair(key, value);
        ++sz;
    }
}

template <typename Key, typename Value, typename KeyAlloc, typename ValueAlloc>
void UnorderedMap<Key, Value, KeyAlloc, ValueAlloc>::insert(Key&& key,
                                                            Value&& value)
{
    if (load_factor() >= MAX_LOAD_FACTOR)
    {
        rehash();
    }

    size_t slot = find_slot(key);
    if (buckets[slot].is_occupied && buckets[slot].key == key)
    {
        // Update existing value
        buckets[slot].value = std::move(value);
    }
    else
    {
        // Find empty slot and insert new pair
        slot = find_empty_slot(key);
        buckets[slot] = KeyValuePair(std::move(key), std::move(value));
        ++sz;
    }
}

template <typename Key, typename Value, typename KeyAlloc, typename ValueAlloc>
template <typename... Args>
void UnorderedMap<Key, Value, KeyAlloc, ValueAlloc>::emplace(const Key& key,
                                                             Args&&... args)
{
    if (load_factor() >= MAX_LOAD_FACTOR)
    {
        rehash();
    }

    size_t slot = find_slot(key);
    if (buckets[slot].is_occupied && buckets[slot].key == key)
    {
        // Update existing value
        buckets[slot].value = Value(std::forward<Args>(args)...);
    }
    else
    {
        // Find empty slot and insert new pair
        slot = find_empty_slot(key);
        buckets[slot].key = key;
        buckets[slot].value = Value(std::forward<Args>(args)...);
        buckets[slot].is_occupied = true;
        ++sz;
    }
}

template <typename Key, typename Value, typename KeyAlloc, typename ValueAlloc>
bool UnorderedMap<Key, Value, KeyAlloc, ValueAlloc>::erase(const Key& key)
{
    size_t slot = find_slot(key);
    if (buckets[slot].is_occupied && buckets[slot].key == key)
    {
        buckets[slot].~KeyValuePair();
        new (&buckets[slot]) KeyValuePair(); // Reset to unoccupied
        --sz;
        return true;
    }
    return false;
}

template <typename Key, typename Value, typename KeyAlloc, typename ValueAlloc>
void UnorderedMap<Key, Value, KeyAlloc, ValueAlloc>::clear()
{
    destroy_elements();
    sz = 0;
    // Reinitialize all buckets as unoccupied
    for (size_t i = 0; i < cap; ++i)
    {
        new (&buckets[i]) KeyValuePair();
    }
}

template <typename Key, typename Value, typename KeyAlloc, typename ValueAlloc>
Value* UnorderedMap<Key, Value, KeyAlloc, ValueAlloc>::find(const Key& key)
{
    size_t hash = hash_function(key);
    size_t slot = hash & (cap - 1);

    // Inline the search loop for better performance
    while (buckets[slot].is_occupied)
    {
        if (buckets[slot].key == key)
        {
            return &buckets[slot].value;
        }
        slot = (slot + 1) & (cap - 1);
    }

    return nullptr;
}

template <typename Key, typename Value, typename KeyAlloc, typename ValueAlloc>
const Value*
UnorderedMap<Key, Value, KeyAlloc, ValueAlloc>::find(const Key& key) const
{
    size_t hash = hash_function(key);
    size_t slot = hash & (cap - 1);

    // Inline the search loop for better performance
    while (buckets[slot].is_occupied)
    {
        if (buckets[slot].key == key)
        {
            return &buckets[slot].value;
        }
        slot = (slot + 1) & (cap - 1);
    }

    return nullptr;
}

template <typename Key, typename Value, typename KeyAlloc, typename ValueAlloc>
bool UnorderedMap<Key, Value, KeyAlloc, ValueAlloc>::contains(const Key& key) const
{
    size_t hash = hash_function(key);
    size_t slot = hash & (cap - 1);

    // Direct inline loop - faster than calling find()
    while (buckets[slot].is_occupied)
    {
        if (buckets[slot].key == key)
        {
            return true;
        }
        slot = (slot + 1) & (cap - 1);
    }

    return false;
}


template <typename Key, typename Value, typename KeyAlloc, typename ValueAlloc>
bool UnorderedMap<Key, Value, KeyAlloc, ValueAlloc>::contains(const Key& key, const Value& value) const
{
    size_t hash = hash_function(key);
    size_t slot = hash & (cap - 1);

    // Direct inline loop - faster than calling find()
    while (buckets[slot].is_occupied)
    {
        if (buckets[slot].key == key)
        {
            value ==  buckets[slot].value;
            return true;
        }
        slot = (slot + 1) & (cap - 1);
    }

    return false;
}


template <typename Key, typename Value, typename KeyAlloc, typename ValueAlloc>
Value&
UnorderedMap<Key, Value, KeyAlloc, ValueAlloc>::operator[](const Key& key)
{
    Value* found = find(key);
    if (found)
    {
        return *found;
    }

    // Insert default value if key doesn't exist
    insert(key, Value{});
    return *find(key);
}

template <typename Key, typename Value, typename KeyAlloc, typename ValueAlloc>
const Value&
UnorderedMap<Key, Value, KeyAlloc, ValueAlloc>::at(const Key& key) const
{
    const Value* found = find(key);
    DEBUG_BREAK_IF(found == nullptr); // Key not found
    return *found;
}

template <typename Key, typename Value, typename KeyAlloc, typename ValueAlloc>
size_t UnorderedMap<Key, Value, KeyAlloc, ValueAlloc>::size() const
{
    return sz;
}

template <typename Key, typename Value, typename KeyAlloc, typename ValueAlloc>
size_t UnorderedMap<Key, Value, KeyAlloc, ValueAlloc>::capacity() const
{
    return cap;
}

template <typename Key, typename Value, typename KeyAlloc, typename ValueAlloc>
bool UnorderedMap<Key, Value, KeyAlloc, ValueAlloc>::empty() const
{
    return sz == 0;
}

template <typename Key, typename Value, typename KeyAlloc, typename ValueAlloc>
float UnorderedMap<Key, Value, KeyAlloc, ValueAlloc>::load_factor() const
{
    return static_cast<float>(sz) / static_cast<float>(cap);
}

template <typename Key, typename Value, typename KeyAlloc, typename ValueAlloc>
Vector<typename UnorderedMap<Key, Value, KeyAlloc, ValueAlloc>::KeyValuePair>
UnorderedMap<Key, Value, KeyAlloc, ValueAlloc>::get_all_pairs() const
{
    Vector<KeyValuePair> result;
    result.reserve(sz);

    for (size_t i = 0; i < cap; ++i)
    {
        if (buckets[i].is_occupied)
        {
            result.push_back(buckets[i]);
        }
    }

    return result;
}

// Private methods
template <typename Key, typename Value, typename KeyAlloc, typename ValueAlloc>
size_t UnorderedMap<Key, Value, KeyAlloc, ValueAlloc>::hash_function(const Key& key) const
{
    // Otimizada para diferentes tipos
    if constexpr (std::is_integral_v<Key>)
    {
        // Multiplicative hash para inteiros
        size_t hash = static_cast<size_t>(key);
        hash ^= hash >> 16;
        hash *= 0x85ebca6b;
        hash ^= hash >> 13;
        hash *= 0xc2b2ae35;
        hash ^= hash >> 16;
        return hash;
    }
    else if constexpr (std::is_same_v<Key, String>)
    {
        // FNV-1a otimizada para strings
        constexpr size_t FNV_OFFSET_BASIS = 14695981039346656037UL;
        constexpr size_t FNV_PRIME = 1099511628211UL;

        size_t hash = FNV_OFFSET_BASIS;
        const char* cstr = key.c_str();
        while (*cstr)
        {
            hash ^= static_cast<size_t>(*cstr);
            hash *= FNV_PRIME;
            ++cstr;
        }
        return hash;
    }
    //     for (char c : key)
    //     {
    //         hash ^= static_cast<size_t>(c);
    //         hash *= FNV_PRIME;
    //     }
    //     return hash;
    // }
    else
    {
        // Fallback melhorado
        const char* bytes = reinterpret_cast<const char*>(&key);
        size_t hash = 14695981039346656037UL;
        for (size_t i = 0; i < sizeof(Key); ++i)
        {
            hash ^= static_cast<size_t>(bytes[i]);
            hash *= 1099511628211UL;
        }
        return hash;
    }
}

template <typename Key, typename Value, typename KeyAlloc, typename ValueAlloc>
size_t
UnorderedMap<Key, Value, KeyAlloc, ValueAlloc>::find_slot(const Key& key) const
{
    size_t hash = hash_function(key);
    size_t slot = hash & (cap - 1); // Assumes cap is power of 2

    // Optimized linear probing with prefetching
    while (buckets[slot].is_occupied)
    {
        if (buckets[slot].key == key)
        {
            return slot;
        }
        slot = (slot + 1) & (cap - 1);
#ifdef __builtin_prefetch
        __builtin_prefetch(&buckets[slot], 0, 1);
#endif
    }

    return slot;
}

template <typename Key, typename Value, typename KeyAlloc, typename ValueAlloc>
size_t UnorderedMap<Key, Value, KeyAlloc, ValueAlloc>::find_empty_slot(
    const Key& key) const
{
    size_t hash = hash_function(key);
    size_t slot = hash & (cap - 1); // Fast modulo for power of 2

    // Linear probing to find empty slot
    while (buckets[slot].is_occupied)
    {
        slot = (slot + 1) & (cap - 1);
    }

    return slot;
}

template <typename Key, typename Value, typename KeyAlloc, typename ValueAlloc>
void UnorderedMap<Key, Value, KeyAlloc, ValueAlloc>::rehash()
{
    KeyValuePair* old_buckets = buckets;
    size_t old_cap = cap;
 

    // Double the capacity
    cap *= 2;
    sz = 0;

    buckets = static_cast<KeyValuePair*>(
        std::aligned_alloc(alignof(KeyValuePair), cap * sizeof(KeyValuePair)));

    // Initialize new buckets
    for (size_t i = 0; i < cap; ++i)
    {
        new (&buckets[i]) KeyValuePair();
    }

    // Rehash all existing elements
    for (size_t i = 0; i < old_cap; ++i)
    {
        if (old_buckets[i].is_occupied)
        {
            insert(std::move(old_buckets[i].key),
                   std::move(old_buckets[i].value));
        }
    }

    // Clean up old buckets
    for (size_t i = 0; i < old_cap; ++i)
    {
        old_buckets[i].~KeyValuePair();
    }
    std::free(old_buckets);
}

template <typename Key, typename Value, typename KeyAlloc, typename ValueAlloc>
void UnorderedMap<Key, Value, KeyAlloc, ValueAlloc>::destroy_elements()
{
    if (buckets)
    {
        for (size_t i = 0; i < cap; ++i)
        {
            buckets[i].~KeyValuePair();
        }
    }
}

template <typename Key, typename Value, typename KeyAlloc, typename ValueAlloc>
void UnorderedMap<Key, Value, KeyAlloc, ValueAlloc>::copy_from(
    const UnorderedMap& other)
{
    sz = other.sz;
    cap = other.cap;

    buckets = static_cast<KeyValuePair*>(
        std::aligned_alloc(alignof(KeyValuePair), cap * sizeof(KeyValuePair)));

    for (size_t i = 0; i < cap; ++i)
    {
        new (&buckets[i]) KeyValuePair(other.buckets[i]);
    }
}