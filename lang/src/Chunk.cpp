#include "Chunk.hpp"
#include "Utils.hpp"

Chunk::Chunk(u32 capacity)
    :  m_capacity(capacity), count(0)
{
    code  = (u8*)  std::malloc(capacity * sizeof(u8));
    lines = (int*) std::malloc(capacity * sizeof(int));

 
}

Chunk::Chunk(Chunk *other)
{

    code  = (u8*)  std::malloc(other->m_capacity * sizeof(u8));
    lines = (int*) std::malloc(other->m_capacity * sizeof(int));
    
    m_capacity = other->m_capacity;
    count = other->count;

    std::memcpy(code, other->code, other->m_capacity * sizeof(u8));
    std::memcpy(lines, other->lines, other->m_capacity * sizeof(int));
}


bool Chunk::clone(Chunk *other)
{
     if (!other)
        return false;

    
    std::free(other->code);
    std::free(other->lines);

    
    other->m_capacity = m_capacity;
    other->count = count;

    other->code = (u8*) std::malloc(m_capacity * sizeof(u8));
    other->lines = (int*) std::malloc(m_capacity * sizeof(int));

    if (!other->code || !other->lines)
    {
        std::free(other->code);
        std::free(other->lines);
        DEBUG_BREAK_IF(other->code == nullptr || other->lines == nullptr);
        return false;
    }

    std::memcpy(other->code, code, count * sizeof(u8));
    std::memcpy(other->lines, lines, count * sizeof(int));

    return true;
    
}


Chunk::~Chunk()
{
    std::free(code);
    std::free(lines);

  //  printf("destroy chunk  \n");
}

void Chunk::reserve(u32 capacity)
{
    if (capacity > m_capacity)
    {
       

        u8 *newCode  = (u8*) (std::realloc(code,  capacity * sizeof(u8)));
        int *newLine = (int*)(std::realloc(lines, capacity * sizeof(int)));

        if (!newCode || !newLine)
        {
            std::free(newCode);
            std::free(newLine);
            DEBUG_BREAK_IF(newCode == nullptr || newLine == nullptr);
            return;
        }



        code = newCode;
        lines = newLine;      
        m_capacity = capacity;
    }
}



void Chunk::write(u8 instruction, int line)
{
    if (m_capacity < count + 1)
    {
        int oldCapacity = m_capacity;
        m_capacity = GROW_CAPACITY(oldCapacity);
        u8 *newCode  = (u8*) (std::realloc(code,  m_capacity * sizeof(u8)));
        int *newLine = (int*)(std::realloc(lines, m_capacity * sizeof(int)));
        if (!newCode || !newLine)
        {
            std::free(newCode);
            std::free(newLine);
            DEBUG_BREAK_IF(newCode == nullptr || newLine == nullptr);
            return;
        }
        code = newCode;
        lines = newLine;      

    }
    
    code[count]  = instruction;
    lines[count] = line;
    count++;
}

u8 Chunk::operator[](u32 index)
{
    DEBUG_BREAK_IF(index > m_capacity);
    return code[index];
}
