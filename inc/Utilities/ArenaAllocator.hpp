#pragma once

#include <iostream>
#include <cstdint>
#include <new> // Required for placement new
#include <memory> // Required for std::align
#include <cstring> // Required for std::memset

// some ideas for arena allocator:
//1. allow the the buffer to grow if we need more memory. Maybe it doesn't grow to the exact size needed, but rather we grow it to accommodate whatever is it we are allocating plus maybe 10 to 20 percent of its size
// (as long as its in alignment
//  that's another thing, we need to enforce strict alignment. 

// basically, uint8_t represents a single byte, hence, there is no weird roudning problems, so it is exactly one byte. 
namespace Aero::Utilities {
	class ArenaAllocator {
	private:
        // block of memory allocated
		uint8_t* buffer;
        // total capacity
		size_t capacity;
        // mem offset
		size_t offset;
	public:
        ArenaAllocator(size_t size) : capacity(size), offset(0) {
            buffer = new uint8_t[size];
        }

        ~ArenaAllocator() {
            delete[] buffer;
        }

        // this function ONLY hands out raw byte memory chunks
        void* allocate_explicit(size_t size, size_t alignment = alignof(std::max_align_t)) {
            size_t remaining_space = capacity - offset;

            void* ptr = buffer + offset;
            if (!std::align(alignment, size, ptr, remaining_space)) {
                return nullptr;
            }

            offset = (static_cast<uint8_t*>(ptr) - buffer) + size;
            return ptr;
        }

        void reset() {
            offset = 0; // clear everything instantly
        }

        // custom template function to construct objects directly in the arena
        template <typename T, typename... Args>
        T* allocate(Args&&... args) {
            void* rawMem = allocate_explicit(sizeof(T), alignof(T));
            if (!rawMem) {
                return nullptr;
            }

            // completely wipe memory of where we are going to allocate to ensure no garbage data is left there (e.g. Visual Studio's debug address pattern)
            std::memset(rawMem, 0, sizeof(T));

            // construct the object in place and forward arguments
            return new (rawMem) T(std::forward<Args>(args)...);
        }
	};
}