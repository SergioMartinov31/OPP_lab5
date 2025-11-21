#include "gtest/gtest.h"
#include "../include/DynamicArray.h"
#include "../include/MemoryResource.h"
#include "../include/Person.h"

class CountingMemoryResource : public CustomMemoryResource {
public:
    size_t allocCount = 0;
    size_t deallocCount = 0;

    void* do_allocate(size_t bytes, size_t alignment) override {
        ++allocCount;
        return CustomMemoryResource::do_allocate(bytes, alignment);
    }

    void do_deallocate(void* ptr, size_t bytes, size_t alignment) override {
        ++deallocCount;
        CustomMemoryResource::do_deallocate(ptr, bytes, alignment);
    }

    bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override {
        return this == &other;
    }
};


class TestObject {
public:
    static int constructed;
    static int destroyed;
    static int copied;
    static int moved;
    
    int value;
    std::string name;
    
    TestObject(int v = 0, std::string n = "") : value(v), name(n) {
        constructed++;
    }
    
    TestObject(const TestObject& other) : value(other.value), name(other.name) {
        copied++;
    }
    
    TestObject(TestObject&& other) noexcept : value(other.value), name(std::move(other.name)) {
        moved++;
        other.value = -1;
    }
    
    TestObject& operator=(const TestObject& other) {
        value = other.value;
        name = other.name;
        copied++;
        return *this;
    }
    
    TestObject& operator=(TestObject&& other) noexcept {
        value = other.value;
        name = std::move(other.name);
        moved++;
        other.value = -1;
        return *this;
    }
    
    ~TestObject() {
        destroyed++;
    }
    
    static void resetCounters() {
        constructed = destroyed = copied = moved = 0;
    }
    
    bool operator==(const TestObject& other) const {
        return value == other.value && name == other.name;
    }
};

int TestObject::constructed = 0;
int TestObject::destroyed = 0;
int TestObject::copied = 0;
int TestObject::moved = 0;


TEST(DynamicArrayBasic, CreateEmptyArray) {
    CountingMemoryResource mem;
    DynamicArray<int> arr(&mem);
    
    EXPECT_TRUE(arr.empty());
    EXPECT_EQ(arr.size(), 0u);
    EXPECT_EQ(arr.capacity(), 0u);
}

TEST(DynamicArrayBasic, PushBackIncreasesSize) {
    CountingMemoryResource mem;
    DynamicArray<int> arr(&mem);
    
    arr.push_back(42);
    EXPECT_FALSE(arr.empty());
    EXPECT_EQ(arr.size(), 1u);
    EXPECT_GE(arr.capacity(), 1u);
    
    arr.push_back(100);
    EXPECT_EQ(arr.size(), 2u);
}

TEST(DynamicArrayBasic, AccessElements) {
    CountingMemoryResource mem;
    DynamicArray<int> arr(&mem);
    
    arr.push_back(10);
    arr.push_back(20);
    arr.push_back(30);
    
    EXPECT_EQ(arr[0], 10);
    EXPECT_EQ(arr[1], 20);
    EXPECT_EQ(arr[2], 30);
    
    
    const DynamicArray<int>& carr = arr;
    EXPECT_EQ(carr[1], 20);
}

TEST(DynamicArrayBasic, AtMethodBoundsCheck) {
    CountingMemoryResource mem;
    DynamicArray<int> arr(&mem);
    
    arr.push_back(5);
    
    EXPECT_NO_THROW(arr.at(0));
    EXPECT_THROW(arr.at(1), std::out_of_range);
    EXPECT_THROW(arr.at(100), std::out_of_range);
}


TEST(DynamicArrayPerson, StorePersons) {
    CountingMemoryResource mem;
    DynamicArray<Person> people(&mem);
    
    people.push_back(Person("Alice", 25));
    people.push_back(Person("Bob", 30));
    people.emplace_back("Charlie", 35);
    
    EXPECT_EQ(people.size(), 3u);
    EXPECT_EQ(people[0].name, "Alice");
    EXPECT_EQ(people[1].age, 30);
    EXPECT_EQ(people[2].name, "Charlie");
}

TEST(DynamicArrayPerson, ModifyPersons) {
    CountingMemoryResource mem;
    DynamicArray<Person> people(&mem);
    
    people.push_back(Person("John", 40));
    people[0].age = 41;
    people[0].name = "Johnny";
    
    EXPECT_EQ(people[0].name, "Johnny");
    EXPECT_EQ(people[0].age, 41);
}


TEST(DynamicArrayIterators, ForwardIteration) {
    CountingMemoryResource mem;
    DynamicArray<int> arr(&mem);
    
    for (int i = 1; i <= 5; ++i) {
        arr.push_back(i * 10);
    }
    
    int expected = 10;
    for (auto it = arr.begin(); it != arr.end(); ++it) {
        EXPECT_EQ(*it, expected);
        expected += 10;
    }
    
    expected = 10;
    for (const auto& val : arr) {
        EXPECT_EQ(val, expected);
        expected += 10;
    }
}

TEST(DynamicArrayIterators, ConstIterators) {
    CountingMemoryResource mem;
    DynamicArray<int> arr(&mem);
    
    arr.push_back(100);
    arr.push_back(200);
    
    const DynamicArray<int>& carr = arr;
    
    int sum = 0;
    for (auto it = carr.cbegin(); it != carr.cend(); ++it) {
        sum += *it;
    }
    EXPECT_EQ(sum, 300);
}


TEST(DynamicArrayMemory, EraseShiftsElements) {
    CountingMemoryResource mem;
    DynamicArray<int> arr(&mem);
    
    for (int i = 0; i < 5; ++i) {
        arr.push_back(i);
    }
    
    arr.erase(2);
    
    EXPECT_EQ(arr.size(), 4u);
    EXPECT_EQ(arr[0], 0);
    EXPECT_EQ(arr[1], 1);
    EXPECT_EQ(arr[2], 3);
    EXPECT_EQ(arr[3], 4);
    
    arr.erase(0);
    EXPECT_EQ(arr.size(), 3u);
    EXPECT_EQ(arr[0], 1);
    EXPECT_EQ(arr[1], 3);
    EXPECT_EQ(arr[2], 4);
}

TEST(DynamicArrayMemory, EraseLastElement) {
    CountingMemoryResource mem;
    DynamicArray<std::string> arr(&mem);
    
    arr.push_back("first");
    arr.push_back("last");
    
    arr.erase(1);
    
    EXPECT_EQ(arr.size(), 1u);
    EXPECT_EQ(arr[0], "first");
}

TEST(DynamicArrayMemory, EraseBoundsCheck) {
    CountingMemoryResource mem;
    DynamicArray<int> arr(&mem);
    
    arr.push_back(1);
    
    EXPECT_THROW(arr.erase(1), std::out_of_range);
    EXPECT_THROW(arr.erase(100), std::out_of_range);
}

TEST(DynamicArrayMove, MoveConstructor) {
    CountingMemoryResource mem;
    
    DynamicArray<int> original(&mem);
    original.push_back(1);
    original.push_back(2);
    original.push_back(3);
    
    size_t original_size = original.size();
    size_t original_capacity = original.capacity();
    
    DynamicArray<int> moved(std::move(original));
    
    EXPECT_EQ(moved.size(), original_size);
    EXPECT_EQ(moved.capacity(), original_capacity);
    EXPECT_EQ(moved[0], 1);
    EXPECT_EQ(moved[1], 2);
    EXPECT_EQ(moved[2], 3);
    
    EXPECT_EQ(original.size(), 0u);
    EXPECT_EQ(original.capacity(), 0u);
}

TEST(DynamicArrayMove, MoveAssignment) {
    CountingMemoryResource mem;
    
    DynamicArray<int> source(&mem);
    source.push_back(10);
    source.push_back(20);
    
    DynamicArray<int> target(&mem);
    target.push_back(5);
    
    target = std::move(source);
    
    EXPECT_EQ(target.size(), 2u);
    EXPECT_EQ(target[0], 10);
    EXPECT_EQ(target[1], 20);
    
    EXPECT_EQ(source.size(), 0u);
}

TEST(DynamicArrayReserve, ReserveIncreasesCapacity) {
    CountingMemoryResource mem;
    DynamicArray<int> arr(&mem);
    
    arr.reserve(10);
    
    EXPECT_GE(arr.capacity(), 10u);
    EXPECT_EQ(arr.size(), 0u);
    
    size_t capacity_before = arr.capacity();
    for (int i = 0; i < 10; ++i) {
        arr.push_back(i);
    }
    EXPECT_EQ(arr.capacity(), capacity_before);
}

TEST(MemoryResource, CustomAllocatorUsed) {
    CountingMemoryResource mem;
    
    DynamicArray<int> arr(&mem);
    
    EXPECT_EQ(mem.allocCount, 0u);
    EXPECT_EQ(mem.deallocCount, 0u);
    
    arr.push_back(1);
    arr.push_back(2);
    
    EXPECT_GT(mem.allocCount, 0u);
    
    arr.clear();
    
    EXPECT_EQ(mem.deallocCount, 0u);
}

TEST(MemoryResource, MemoryReuse) {
    CountingMemoryResource mem;
    
    {
        DynamicArray<int> arr(&mem);
        for (int i = 0; i < 100; ++i) {
            arr.push_back(i);
        }
        
        size_t allocs_during_fill = mem.allocCount;
        
        while (!arr.empty()) {
            arr.erase(0);
        }
        
        for (int i = 0; i < 100; ++i) {
            arr.push_back(i * 2);
        }
        
        EXPECT_LE(mem.allocCount, allocs_during_fill + 2);
    }
}

TEST(DynamicArrayEdgeCases, EmptyArrayOperations) {
    CountingMemoryResource mem;
    DynamicArray<int> arr(&mem);
    
    EXPECT_NO_THROW(arr.clear());
    EXPECT_THROW(arr.erase(0), std::out_of_range);
    EXPECT_THROW(arr.at(0), std::out_of_range);
}

TEST(DynamicArrayEdgeCases, SingleElement) {
    CountingMemoryResource mem;
    DynamicArray<std::string> arr(&mem);
    
    arr.push_back("alone");
    
    EXPECT_EQ(arr.size(), 1u);
    EXPECT_EQ(arr[0], "alone");
    
    arr.erase(0);
    EXPECT_TRUE(arr.empty());
    
    EXPECT_THROW(arr.erase(0), std::out_of_range);
}

TEST(DynamicArrayComplex, CombinedOperations) {
    CountingMemoryResource mem;
    DynamicArray<Person> arr(&mem);
    
    arr.push_back(Person("Alice", 25));
    arr.emplace_back("Bob", 30);
    arr.push_back(Person("Charlie", 35));
    
    EXPECT_EQ(arr.size(), 3u);
    
    arr.erase(1);
    EXPECT_EQ(arr.size(), 2u);
    EXPECT_EQ(arr[0].name, "Alice");
    EXPECT_EQ(arr[1].name, "Charlie");
    
    arr.emplace_back("David", 40);
    arr.push_back(Person("Eve", 45));
    
    EXPECT_EQ(arr.size(), 4u);
    
    int total_age = 0;
    for (const auto& person : arr) {
        total_age += person.age;
    }
    EXPECT_EQ(total_age, 25 + 35 + 40 + 45);
    
    arr.clear();
    EXPECT_TRUE(arr.empty());
    EXPECT_EQ(arr.size(), 0u);
}

int main(int argc, char **argv) {
    MR_LOG_ENABLED = false; 
    ::testing::InitGoogleTest(&argc, argv);
    
    std::cout << "Запуск расширенных тестов DynamicArray..." << std::endl;
    std::cout << "Эти тесты проверяют различные аспекты работы контейнера:" << std::endl;
    std::cout << "- Базовые операции добавления и доступа" << std::endl;
    std::cout << "- Управление памятью и аллокаторы" << std::endl;
    std::cout << "- Итераторы и диапазонные for-циклы" << std::endl;
    std::cout << "- Граничные случаи и обработка ошибок" << std::endl;
    std::cout << "- Перемещающая семантика" << std::endl;
    std::cout << std::endl;
    
    return RUN_ALL_TESTS();
}