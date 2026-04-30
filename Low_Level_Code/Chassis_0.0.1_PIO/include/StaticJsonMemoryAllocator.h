#ifndef STATIC_JSON_MEMORY_ALLOCATOR_H
#define STATIC_JSON_MEMORY_ALLOCATOR_H

#include <ArduinoJson.h>
#include <cstddef>     // for alignof/alignas if needed
#include "Configs/NetworkConfig.h"

class StaticJsonMemoryAllocator : public ArduinoJson::Allocator {
public:
    void* allocate(size_t size) override;
    void deallocate(void* pointer) override;
    void* reallocate(void* ptr, size_t new_size) override;

    void reset();  // for reuse

private:
    static uint8_t buffer_[];
    uint8_t* current_ = buffer_;
    static constexpr size_t capacity = NetworkConfig::MQTT_MAX_JSON_PAYLOAD;
};

#endif