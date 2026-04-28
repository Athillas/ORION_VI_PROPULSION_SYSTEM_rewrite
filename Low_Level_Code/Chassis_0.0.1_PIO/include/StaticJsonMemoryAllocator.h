// StaticJsonMemoryAllocator.h
#ifndef STATIC_JSON_MEMORY_ALLOCATOR_H
#define STATIC_JSON_MEMORY_ALLOCATOR_H

#include "ArduinoJson.h"
#include "Configs/NetworkConfig.h"

// Custom allocator that uses a fixed stack buffer.
class StaticJsonMemoryAllocator : public ArduinoJson::Allocator {
public:
    void* allocate(size_t size) override {
        if (current_ + size > buffer_ + sizeof(buffer_)) {
            // Out of memory: return nullptr to signal overflow.
            return nullptr;
        }
        void* ptr = current_;
        current_ += size;
        return ptr;
    }

    void deallocate(void* pointer) override {
        // Monotonic allocator: ignore deallocation.
    }

    void* reallocate(void* ptr, size_t new_size) override {
        // Not supported for this simple allocator.
        // Returning nullptr tells ArduinoJson that reallocation failed.
        return nullptr;
    }

private:
    static uint8_t buffer_[NetworkConfig::MQTT_MAX_JSON_PAYLOAD];
    uint8_t* current_ = buffer_;
};

// Define the static buffer (exactly once in a .cpp file).
uint8_t StaticJsonMemoryAllocator::buffer_[NetworkConfig::MQTT_MAX_JSON_PAYLOAD];

#endif