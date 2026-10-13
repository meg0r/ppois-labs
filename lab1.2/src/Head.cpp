#include "Head.hpp"

Head::Head(int64_t start_pos) noexcept : position_(start_pos) {}

int64_t Head::getPosition() const noexcept {
    return position_;
}

void Head::setPosition(int64_t pos) noexcept {
    position_ = pos;
}

Head& Head::operator++() noexcept {
    ++position_;
    return *this;
}

Head Head::operator++(int) noexcept {
    Head temp = *this;
    ++(*this);
    return temp;
}

Head& Head::operator--() noexcept {
    --position_;
    return *this;
}

Head Head::operator--(int) noexcept {
    Head temp = *this;
    --(*this);
    return temp;
}

bool Head::operator==(const Head& other) const noexcept {
    return position_ == other.position_;
}

bool Head::operator!=(const Head& other) const noexcept {
    return !(*this == other);
}