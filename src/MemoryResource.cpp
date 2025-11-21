#include "../include/MemoryResource.h"
#include <new>
#include <iostream>
#include <stdexcept>

bool MR_LOG_ENABLED = true;

void* CustomMemoryResource::do_allocate(size_t bytes, size_t alignment) {
    auto key = std::make_pair(bytes, alignment);

    auto it = free_blocks.find(key);
    if (it != free_blocks.end() && !it->second.empty()) {
        void* ptr = it->second.back();
        it->second.pop_back();

        allocated[ptr] = {bytes, alignment};

        if (MR_LOG_ENABLED)
            std::cout << "[MR] reuse: " << ptr << " (" << bytes << ")\n";

        return ptr;
    }

    void* ptr = ::operator new(bytes, std::align_val_t(alignment));
    allocated[ptr] = {bytes, alignment};

    if (MR_LOG_ENABLED)
        std::cout << "[MR] new:   " << ptr << " (" << bytes << ")\n";

    return ptr;
}

void CustomMemoryResource::do_deallocate(void* ptr, size_t bytes, size_t alignment) {
    (void)bytes; // Явно помечаем как неиспользуемый параметр
    (void)alignment; // Явно помечаем как неиспользуемый параметр
    
    auto it = allocated.find(ptr);
    if (it == allocated.end())
        throw std::logic_error("deallocate on non-allocated ptr");

    auto info = it->second;
    allocated.erase(it);

    free_blocks[{info.size, info.alignment}].push_back(ptr);

    if (MR_LOG_ENABLED)
        std::cout << "[MR] free:  " << ptr << "\n";
}

bool CustomMemoryResource::do_is_equal(const std::pmr::memory_resource& other) const noexcept {
    return this == &other;
}

CustomMemoryResource::~CustomMemoryResource() {
    std::cout << "[MR] CLEANUP\n";

    for (auto& p : allocated) {
        ::operator delete(p.first, std::align_val_t(p.second.alignment));
        std::cout << "  delete active " << p.first << "\n";
    }

    for (auto& kv : free_blocks) {
        for (void* ptr : kv.second) {
            ::operator delete(ptr, std::align_val_t(kv.first.second));
            std::cout << "  delete free   " << ptr << "\n";
        }
    }
}