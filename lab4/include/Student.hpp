#pragma once
#include <string>
#include <iostream>

namespace lab4 {

class Student {
private:
    std::string name_;
    int id_;
    double gpa_;

public:
    Student() : name_("Unknown"), id_(0), gpa_(0.0) {}
    Student(std::string name, int id, double gpa)
        : name_(std::move(name)), id_(id), gpa_(gpa) {}

    const std::string& getName() const noexcept { return name_; }
    int getId() const noexcept { return id_; }
    double getGpa() const noexcept { return gpa_; }

    // Comparison operators (prioritize GPA, then ID, then name)
    bool operator==(const Student& other) const noexcept {
        return id_ == other.id_ && name_ == other.name_ && gpa_ == other.gpa_;
    }
    bool operator!=(const Student& other) const noexcept { return !(*this == other); }

    bool operator<(const Student& other) const noexcept {
        if (gpa_ != other.gpa_) return gpa_ < other.gpa_;
        if (id_ != other.id_) return id_ < other.id_;
        return name_ < other.name_;
    }
    bool operator>(const Student& other) const noexcept { return other < *this; }
    bool operator<=(const Student& other) const noexcept { return !(other < *this); }
    bool operator>=(const Student& other) const noexcept { return !(*this < other); }

    friend std::ostream& operator<<(std::ostream& os, const Student& s) {
        os << "{" << s.name_ << " #" << s.id_ << ", GPA: " << s.gpa_ << "}";
        return os;
    }
};

} // namespace lab4