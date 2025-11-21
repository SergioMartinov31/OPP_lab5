#include <iostream>
#include "include/MemoryResource.h"
#include "include/DynamicArray.h"
#include "include/Person.h"

template <typename C>
void print(const C& c) {
    for (auto& x : c) std::cout << x << "\n";
}

int main() {
    CustomMemoryResource mem;

    std::cout << "\n=== Demo int ===\n";
    DynamicArray<int> a(&mem);
    a.push_back(10);
    a.push_back(20);
    a.push_back(30);
    print(a);

    std::cout << "\nIterate with iterator explicitly:\n";
    for (auto it = a.begin(); it != a.end(); ++it)
        std::cout << *it << "\n";

    std::cout << "\n=== Demo Person ===\n";
    DynamicArray<Person> b(&mem);
    b.push_back(Person("Alice", 20));
    b.push_back(Person("Bob", 30));
    print(b);

    std::cout << "\n=== Test erase ===\n";
    a.erase(1);
    print(a);

    std::cout << "\n=== Program end ===\n";
    return 0;
}
