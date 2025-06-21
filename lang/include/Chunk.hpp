#pragma once

#include "Config.hpp"

class Chunk
{
    u32 m_capacity;

public:
    Chunk(u32 capacity = 512);
    Chunk(Chunk *other);
    ~Chunk();

    void clear();
    void reserve(u32 capacity);

    void write(u8 instruction, int line);

    u32 capacity() const { return m_capacity; }

    u8 operator[](u32 index);

    bool clone(Chunk *other);

    u8 *code;
    int *lines;
    u32 count;
};
