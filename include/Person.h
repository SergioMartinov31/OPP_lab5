#ifndef PERSON_H
#define PERSON_H

#include <string>
#include <iostream>

struct Person {
    std::string name;
    int age;

    Person() = default;
    Person(std::string n, int a) : name(std::move(n)), age(a) {}
};

inline std::ostream& operator<<(std::ostream& os, const Person& p) {
    return os << p.name << " (" << p.age << ")";
}

#endif
