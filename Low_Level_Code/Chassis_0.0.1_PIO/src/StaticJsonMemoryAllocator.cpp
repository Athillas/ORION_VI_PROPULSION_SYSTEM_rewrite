#include "StaticJsonMemoryAllocator.h"

uint8_t StaticJsonMemoryAllocator::buffer_[StaticJsonMemoryAllocator::capacity];

void* StaticJsonMemoryAllocator::allocate(size_t size) {
    // align size to platform requirement (simplified)
    size = (size + alignof(std::max_align_t) - 1) & ~(alignof(std::max_align_t) - 1);
    if (current_ + size > buffer_ + capacity) return nullptr;
    void* ptr = current_;
    current_ += size;
    return ptr;
}

void StaticJsonMemoryAllocator::deallocate(void*) {
    // monotonic – ignore
}

void* StaticJsonMemoryAllocator::reallocate(void* ptr, size_t new_size) {
    // Only supports in-place growth if ptr is the last allocated block.
    uint8_t* bytePtr = static_cast<uint8_t*>(ptr);
    if (bytePtr && bytePtr + (new_size) <= buffer_ + capacity && bytePtr == current_) {
        // Expand the last allocation (simple bump)
        current_ = bytePtr + new_size;
        return ptr;
    }
    // Otherwise we cannot handle – return null.
    return nullptr;
}

void StaticJsonMemoryAllocator::reset() {
    current_ = buffer_;
}