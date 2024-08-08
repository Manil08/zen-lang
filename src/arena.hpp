#pragma once
using namespace std;

class ArenaAllocator
{
public:
    inline explicit ArenaAllocator(const size_t bytes)
        : m_size(bytes)
    {
        m_buffer=static_cast<byte*>(malloc(m_size));
        m_offset=m_buffer;
    }

    // // Using the below two functions to prevent accidental copying of Arena Allocator
    // inline ArenaAllocator(const ArenaAllocator& other) = delete;
    //
    // inline ArenaAllocator operator=(const ArenaAllocator& other) = delete;

    template<typename T>
    inline T* alloc() {
        void* offset = m_offset;
        m_offset += sizeof(T);
        return static_cast<T*>(offset);
    }

    inline ~ArenaAllocator() {
        free(m_buffer);
    }

private:
    byte* m_buffer;
    byte* m_offset;
    size_t m_size;
};