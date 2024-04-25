#pragma once

#include "compressed_pair.h"

#include <cstddef>  // std::nullptr_t
#include <type_traits>

template <class T, class U>
concept check = std::is_base_of_v<T, U> || std::is_same_v<U, T>;

template <class T>
class Slug {
public:
    Slug() {
    }
    template <class U>
    Slug(Slug<U>&& other) requires check<T, U> {
    }
    template <class U>
    Slug(const Slug<U>& other) requires check<T, U> {
    }

    template <class U>
    void operator()(U*& a) requires check<T, U> {
        if (a == nullptr) {
            return;
        }
        delete a;
    }
    template <class U>
    Slug& operator=(const Slug<U>& other) requires check<T, U> {
        return *this;
    }
    template <class U>
    Slug& operator=(Slug<U[]>&& other) requires check<T, U> {
        return *this;
    }
};
template <class T>
struct Slug<T[]> {

    Slug() {
    }
    template <class U>
    Slug(Slug<U>&& other) requires check<T, U> {
    }
    template <class U>
    Slug(const Slug<U>& other) requires check<T, U> {
    }

    template <class U>
    Slug(Slug<U[]>&& other) requires check<T, U> {
    }

    template <class U>
    Slug(const Slug<U[]>& other) requires check<T, U> {
    }

    template <class U>
    void operator()(U*& a) requires check<T, U> {
        if (a == nullptr) {
            return;
        }
        delete[] a;
    }

    template <class U>
    Slug& operator=(const Slug<U[]>& other) requires check<T, U> {
        return *this;
    }
    template <class U>
    Slug& operator=(Slug<U[]>&& other) requires check<T, U> {
        return *this;
    }
};

// Primary template
template <typename T, typename Deleter = Slug<T>>
class UniquePtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(T* ptr = nullptr) : object_(ptr, Deleter()) {
    }
    UniquePtr(T* ptr, Deleter deleter) : object_(ptr, std::move(deleter)) {
    }

    template <class U, class DeleterU>
    UniquePtr(UniquePtr<U, DeleterU>&& other) noexcept {
        object_.GetFirst() = std::move(other.object_.GetFirst());
        object_.GetSecond() = std::move(other.object_.GetSecond());
        other.object_.GetFirst() = nullptr;
    }

    template <class U, class DeleterU>
    UniquePtr& operator=(UniquePtr<U, DeleterU>&& other) noexcept {
        if (object_.GetFirst() == other.object_.GetFirst()) {
            return *this;
        }
        Clear();
        object_.GetFirst() = std::move(other.object_.GetFirst());
        object_.GetSecond() = std::move(other.object_.GetSecond());
        other.object_.GetFirst() = nullptr;
        return *this;
    }

    UniquePtr& operator=(std::nullptr_t) {
        Clear();
        return *this;
    }

    ~UniquePtr() {
        Clear();
    }

    T* Release() {
        auto first = object_.GetFirst();
        object_.GetFirst() = nullptr;
        return first;
    }

    void Reset(T* ptr = nullptr) {
        auto prev = object_.GetFirst();
        object_.GetFirst() = ptr;
        object_.GetSecond()(prev);
    }

    void Swap(UniquePtr& other) {
        std::swap(object_.GetFirst(), other.object_.GetFirst());
        std::swap(object_.GetSecond(), other.object_.GetSecond());
    }

    T* Get() const {
        return object_.GetFirst();
    }

    Deleter& GetDeleter() {
        return object_.GetSecond();
    }

    const Deleter& GetDeleter() const {
        return object_.GetSecond();
    }

    explicit operator bool() const {
        return (object_.GetFirst() != nullptr);
    }

    std::add_lvalue_reference_t<T> operator*() const {
        return *object_.GetFirst();
    }

    T* operator->() const {
        return object_.GetFirst();
    }

    CompressedPair<T*, Deleter> object_;

private:
    void Clear() {
        object_.GetSecond()(object_.GetFirst());
        object_.GetFirst() = nullptr;
    }
};

// Specialization for arrays
template <typename T, typename Deleter>
class UniquePtr<T[], Deleter> {
public:
    explicit UniquePtr(T* ptr = nullptr) : object_(ptr, Deleter()) {
    }
    UniquePtr(T* ptr, Deleter deleter) : object_(ptr, deleter) {
    }

    UniquePtr(UniquePtr&& other) noexcept {
        object_.GetFirst() = dynamic_cast<T*>(std::move(other.object_.GetFirst()));
        object_.GetSecond() = std::move(other.object_.GetSecond());
        other.object_.GetFirst() = nullptr;
    }

    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (object_.GetFirst() == other.object_.GetFirst()) {
            return *this;
        }
        Clear();
        object_ = std::move(other.object_);
        other.object_.GetFirst() = nullptr;
        return *this;
    }
    UniquePtr& operator=(std::nullptr_t) {
        Clear();
        return *this;
    }

    ~UniquePtr() {
        Clear();
    }

    T* Release() {
        auto first = object_.GetFirst();
        object_.GetFirst() = nullptr;
        return first;
    }

    void Reset(T* ptr = nullptr) {
        auto prev = object_.GetFirst();
        object_.GetFirst() = ptr;
        object_.GetSecond()(prev);
    }

    void Swap(UniquePtr& other) {
        std::swap(object_.GetFirst(), other.object_.GetFirst());
        std::swap(object_.GetSecond(), other.object_.GetSecond());
    }

    T* Get() const {
        return object_.GetFirst();
    }

    Deleter& GetDeleter() {
        return object_.GetSecond();
    }

    const Deleter& GetDeleter() const {
        return object_.GetSecond();
    }

    explicit operator bool() const {
        return (object_.GetFirst() != nullptr);
    }

    T& operator[](size_t i) {
        return object_.GetFirst()[i];
    }

    std::add_lvalue_reference_t<T> operator*() const {
        return *object_.GetFirst();
    }
    T* operator->() const {
        return object_.GetFirst();
    }

private:
    CompressedPair<T*, Deleter> object_;
    void Clear() {
        object_.GetSecond()(object_.GetFirst());
    }
};