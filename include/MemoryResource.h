#ifndef MEMORYRESOURCE_H
#define MEMORYRESOURCE_H

#include <memory_resource>
#include <map>
#include <vector>
#include <cstddef>
#include <iostream>

extern bool MR_LOG_ENABLED;

class CustomMemoryResource : public std::pmr::memory_resource {
public:
    CustomMemoryResource() = default;
    ~CustomMemoryResource();

protected:
    void* do_allocate(size_t bytes, size_t alignment) override;
    void  do_deallocate(void* ptr, size_t bytes, size_t alignment) override;
    bool  do_is_equal(const std::pmr::memory_resource& other) const noexcept override;

private:
    struct BlockInfo {
        size_t size;
        size_t alignment;
    };

    std::map<void*, BlockInfo> allocated;
    std::map<std::pair<size_t,size_t>, std::vector<void*>> free_blocks;
};

#endif
