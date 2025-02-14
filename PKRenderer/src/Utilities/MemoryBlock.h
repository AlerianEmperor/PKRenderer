#pragma once
#include "NoCopy.h"
#include "BufferView.h"
#include <exception>

namespace PK::Utilities
{
    template<typename T>
    class MemoryBlock : NoCopy
    {
        public:
            MemoryBlock(size_t count) 
            {
                Validate(count); 
            }
            
            ~MemoryBlock()
            {
                if (m_data != nullptr)
                {
                    free(m_data);
                }
            }

            void Validate(size_t count, bool discard = false)
            {
                if (count <= m_count)
                {
                    return;
                }

                auto newCount = m_count == 0 ? count : m_count;

                while (newCount < count)
                {
                    newCount <<= 1;
                }

                auto newbuffer = calloc(newCount, sizeof(T));

                if (newbuffer == nullptr)
                {
                    throw std::runtime_error("Failed to allocate new buffer!");
                }

                if (m_data != nullptr)
                {
                    if (!discard)
                    {
                        memcpy(newbuffer, m_data, sizeof(T) * m_count);
                    }

                    free(m_data);
                }

                m_data = newbuffer;
                m_count = newCount;
            }

            void Clear() { memset(m_data, 0, sizeof(T) * m_count); }

            BufferView<T> GetView() { return { reinterpret_cast<T*>(m_data), m_count }; }

            ConstBufferView<T> GetView() const { return { reinterpret_cast<const T*>(m_data), m_count }; }

            T* GetOffset(size_t offset) { return reinterpret_cast<T*>(m_data) + offset; }

            T const* GetOffset(size_t offset) const { return reinterpret_cast<const T*>(m_data) + offset; }

            T* GetData() { return reinterpret_cast<T*>(m_data); }

            T const* GetData() const { return reinterpret_cast<const T*>(m_data); }

            T& operator [](size_t i) { return reinterpret_cast<T*>(m_data)[i]; }

            T const& operator [](size_t i) const { return reinterpret_cast<const T*>(m_data)[i]; }

            operator T* () { return GetData(); }

            operator T const* () const { return GetData(); }

            constexpr size_t GetCount() const { return m_count; }
            constexpr size_t GetSize() const { return m_count * sizeof(T); }

        private:
            void* m_data = nullptr;
            size_t m_count = 0ull;
    };
}