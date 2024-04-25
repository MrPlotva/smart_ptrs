#pragma once

#include "sw_fwd.h"  // Forward declaration

#include <cstddef>  // std::nullptr_t
#include <iostream>
#include <type_traits>
#include <utility>

// https://en.cppreference.com/w/cpp/memory/shared_ptr

class ControlBlock {
public:
    void Add() {
        counter_++;
    }
    void Del() {
        counter_--;
        if (counter_ == 0) {
            this->~ControlBlock();
        }
    }

    int GetCount() {
        return counter_;
    }

    virtual ~ControlBlock() noexcept {
    }

private:
    int counter_ = 0;
};

template <typename T>
class ControlBlockPtr : ControlBlock {
public:
    ControlBlockPtr(T* ptr = nullptr) : ptr_(ptr) {
        this->Add();
    }

    operator ControlBlock*() {
        return this;
    }

    ~ControlBlockPtr() override {
        delete ptr_;
    }

private:
    T* ptr_;
};

template <typename T>
class ControlBlockObj : ControlBlock {
public:
    ControlBlockObj(T& obj) : obj_(obj) {
        this->Add();
    }

    template <typename U, typename... Args>
    friend SharedPtr<U> MakeShared(Args&&... args);

    template <typename... Args>
    ControlBlockObj(Args&&... args) : obj_(std::forward<Args>(args)...) {
        this->Add();
    }

    operator ControlBlock*() {
        return this;
    }

    ~ControlBlockObj() override {
    }

private:
    T obj_;
};

template <typename T>
class SharedPtr {
public:
    template <typename Y>
    friend class SharedPtr;

    template <typename... Args>
    friend SharedPtr<T> MakeShared(Args&&... args);

    SharedPtr() {
    }
    SharedPtr(std::nullptr_t) {
    }

    explicit SharedPtr(T* ptr) {
        block_ = *(new ControlBlockPtr<T>(ptr));
        observed_ = ptr;
    }

    template <typename U>
    explicit SharedPtr(U* ptr) {
        block_ = *(new ControlBlockPtr<U>(ptr));
        observed_ = ptr;
    }

    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other) {
        block_ = other.block_;
        observed_ = other.observed_;
        if (block_ != nullptr) {
            block_->Add();
        }
    }

    template <typename Y>
    SharedPtr(SharedPtr<Y>&& other) {
        block_ = other.block_;
        observed_ = other.observed_;
        other.observed_ = nullptr;
        other.block_ = nullptr;
    }

    SharedPtr(const SharedPtr& other) {
        block_ = other.block_;
        observed_ = other.observed_;
        if (block_ != nullptr) {
            block_->Add();
        }
    }

    SharedPtr(SharedPtr&& other) {
        block_ = other.block_;
        observed_ = other.observed_;
        other.observed_ = nullptr;
        other.block_ = nullptr;
    }

    template <typename Y>
    SharedPtr(SharedPtr<Y>& other, T* ptr) {
        block_ = other.block_;
        observed_ = ptr;
        block_->Add();
    }

    explicit SharedPtr(const WeakPtr<T>& other);

    template <typename Y>
    SharedPtr& operator=(const SharedPtr<Y>& other) {
        if (observed_ == other.observed_) {
            return *this;
        }
        Clear();
        block_ = other.block_;
        observed_ = other.observed_;
        if (block_ != nullptr) {
            block_->Add();
        }
        return *this;
    }

    template <typename Y>
    SharedPtr& operator=(SharedPtr<Y>&& other) {
        if (observed_ == other.observed_) {
            other.observed_ = nullptr;
            other.block_ = nullptr;
            return *this;
        }
        Clear();
        block_ = other.block_;
        observed_ = other.observed_;
        other.observed_ = nullptr;
        other.block_ = nullptr;
        return *this;
    }

    SharedPtr& operator=(const SharedPtr& other) {
        if (observed_ == other.observed_) {
            return *this;
        }
        Clear();
        block_ = other.block_;
        observed_ = other.observed_;
        if (block_ != nullptr) {
            block_->Add();
        }
        return *this;
    }

    SharedPtr& operator=(SharedPtr&& other) {
        if (observed_ == other.observed_) {
            other.observed_ = nullptr;
            other.block_ = nullptr;
            return *this;
        }
        Clear();
        block_ = other.block_;
        observed_ = other.observed_;
        other.observed_ = nullptr;
        other.block_ = nullptr;
        return *this;
    }

    ~SharedPtr() {
        Clear();
    }

    void Reset() {
        Clear();
        block_ = nullptr;
        observed_ = nullptr;
    }

    void Reset(T* ptr) {

        if (observed_ == ptr) {
            return;
        }
        if (observed_ != ptr) {
            Clear();
        }
        block_ = *(new ControlBlockPtr<T>(ptr));
        observed_ = ptr;
    }

    template <typename U>
    void Reset(U* ptr) {

        if (observed_ == ptr) {
            return;
        }
        if (observed_ != ptr) {
            Clear();
        }
        block_ = *(new ControlBlockPtr<U>(ptr));
        observed_ = ptr;
    }

    void Swap(SharedPtr& other) {
        std::swap(block_, other.block_);
        std::swap(observed_, other.observed_);
    }

    T* Get() const {
        return observed_;
    }
    T& operator*() const {
        return *observed_;
    }
    T* operator->() const {
        return observed_;
    }
    size_t UseCount() const {
        if (!block_) {
            return 0;
        }
        return block_->GetCount();
    }
    explicit operator bool() const {
        return (observed_ != nullptr);
    }

private:
    ControlBlock* block_ = nullptr;
    T* observed_ = nullptr;

    ControlBlock*& GetBlock() {
        return block_;
    }
    void Clear() {
        if (block_ != nullptr) {
            if (block_->GetCount() == 1) {
                delete block_;
            } else {
                block_->Del();
            }
        }
    }
    template <typename Y>
    friend class SharedPtr;

    template <typename U>
    friend inline bool operator==(const SharedPtr<T>& left, const SharedPtr<U>& right);

    template <typename U, typename... Args>
    friend SharedPtr<U> MakeShared(Args&&... args);
};

template <typename T, typename U>
inline bool operator==(const SharedPtr<T>& left, const SharedPtr<U>& right) {
    return left.observed_ == right.observed_;
}

template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
    SharedPtr<T> shr;
    auto cur = (new ControlBlockObj<T>(std::forward<Args>(args)...));
    shr.block_ = cur;
    shr.observed_ = &(cur->obj_);
    return shr;
}

// Look for usage examples in tests
template <typename T>
class EnableSharedFromThis {
public:
    SharedPtr<T> SharedFromThis();
    SharedPtr<const T> SharedFromThis() const;

    WeakPtr<T> WeakFromThis() noexcept;
    WeakPtr<const T> WeakFromThis() const noexcept;
};
