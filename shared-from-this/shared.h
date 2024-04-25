#pragma once

#include "catch.hpp"
// #include "shared-from-this/weak.h"
#include "sw_fwd.h"  // Forward declaration

#include <cstddef>  // std::nullptr_t
#include <iostream>
#include <type_traits>
#include <utility>

// https://en.cppreference.com/w/cpp/memory/shared_ptr
template <typename T>
class EnableSharedFromThis;

class ESFTBase;

class ControlBlock {
public:
    // ControlBlock() {}
    virtual void IncStrong() {
    }

    virtual void DecStrong() {
    }

    virtual int GetStrongCount() {
        return 0;
    }

    virtual void IncWeak(){};

    virtual void DecWeak(){};

    virtual int GetWeakCount() {
        return 0;
    };

    // virtual auto GetPtr() {}
    virtual ~ControlBlock() noexcept {
    }

private:
    int strong_counter_ = 0;
    int weak_counter_ = 0;
    template <typename T>
    friend class ControlBlockPtr;
    template <typename T>
    friend class ControlBlockObj;
    template <typename T>
    friend class SharedPtr;
    // friend class ControlBlockPtr;
    // friend class ControlBlockObj;
};

template <typename T>
class ControlBlockPtr : ControlBlock {
public:
    ControlBlockPtr(T* ptr = nullptr) : ptr_(ptr) {
        this->IncStrong();
    }

    void IncStrong() override {
        this->strong_counter_++;
    }
    void DecStrong() override {
        if (this->strong_counter_ == 1) {
            if (this->weak_counter_ == 0) {
                delete this;
                return;
            } else {
                delete ptr_;
                ptr_ = nullptr;
            }
        }
        this->strong_counter_--;
        if (strong_counter_ == 0 && weak_counter_ == 0) {
            delete this;
        }
    }

    int GetStrongCount() override {
        return this->strong_counter_;
    }

    void IncWeak() override {
        this->weak_counter_++;
    }
    void DecWeak() override {
        if (this->weak_counter_ == 1 && this->strong_counter_ == 0) {
            delete this;
            return;
        }
        this->weak_counter_--;
    }

    int GetWeakCount() override {
        return this->weak_counter_;
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
    // ControlBlockObj(T& obj) : obj_(obj) {
    //     this->IncStrong();
    // }
    // template <typename Y>

    template <typename U, typename... Args>
    friend SharedPtr<U> MakeShared(Args&&... args);

    template <typename... Args>
    ControlBlockObj(Args&&... args) {
        new (GetPtr()) T(std::forward<Args>(args)...);
        this->IncStrong();
    }

    operator ControlBlock*() {
        return this;
    }

    ~ControlBlockObj() override {
        // reinterpret_cast<T*>(std::addressof(storage_)) -> ~T();
    }

    void IncStrong() override {
        this->strong_counter_++;
    }
    void DecStrong() override {
        if (this->strong_counter_ == 1) {
            reinterpret_cast<T*>(std::addressof(storage_))->~T();
            if (this->weak_counter_ == 0) {
                delete this;
                return;
            }
        }
        this->strong_counter_--;
    }

    int GetStrongCount() override {
        return this->strong_counter_;
    }

    void IncWeak() override {
        this->weak_counter_++;
    }
    void DecWeak() override {
        if (this->weak_counter_ == 1 && this->strong_counter_ == 0) {
            delete this;
            return;
        }
        this->weak_counter_--;
    }

    int GetWeakCount() override {
        return this->weak_counter_;
    }

    T* GetPtr() {
        return reinterpret_cast<T*>(std::addressof(storage_));
    }

private:
    std::aligned_storage_t<sizeof(T), alignof(T)> storage_;
};

template <typename T>
class SharedPtr {
public:
    template <typename Y>
    friend class SharedPtr;

    template <typename... Args>
    friend SharedPtr<T> MakeShared(Args&&... args);

    SharedPtr() {
        // PutWeakThis();
    }
    SharedPtr(std::nullptr_t) {
        // PutWeakThis();
    }

    explicit SharedPtr(T* ptr) {
        block_ = *(new ControlBlockPtr<T>(ptr));
        observed_ = ptr;
        PutWeakThis();
    }

    template <typename U>
    explicit SharedPtr(U* ptr) {
        block_ = *(new ControlBlockPtr<U>(ptr));
        observed_ = ptr;
        PutWeakThis();
    }

    explicit SharedPtr(const WeakPtr<T>& other) {
        if (other.Expired()) {
            throw BadWeakPtr();
        }
        block_ = other.block_;
        observed_ = other.observed_;
        if (block_ != nullptr) {
            block_->IncStrong();
        }
    }

    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other) {
        block_ = other.block_;
        observed_ = other.observed_;
        if (block_ != nullptr) {
            block_->IncStrong();
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
            block_->IncStrong();
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
        block_->IncStrong();
    }

    // explicit SharedPtr(const WeakPtr<T>& other);

    template <typename Y>
    SharedPtr& operator=(const SharedPtr<Y>& other) {
        if (observed_ == other.observed_) {
            return *this;
        }
        Clear();
        block_ = other.block_;
        observed_ = other.observed_;
        if (block_ != nullptr) {
            block_->IncStrong();
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
            block_->IncStrong();
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
        PutWeakThis();
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
        PutWeakThis();
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
        return block_->GetStrongCount();
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
            block_->DecStrong();
        }
    }

    template <typename Y>
    void InitWeakThis(EnableSharedFromThis<Y>* e) {
        e->weak_this_ = *this;
    }

    void PutWeakThis() {
        if constexpr (std::is_convertible_v<T*, ESFTBase*>) {
            if (Get()) {
                InitWeakThis(Get());
            }
        }
    }
    template <typename Y>
    friend class SharedPtr;

    template <typename Y>
    friend class WeakPtr;

    template <typename U, typename... Args>
    friend SharedPtr<U> MakeShared(Args&&... args);
};

// template <>
// inline bool operator==<T>(const SharedPtr& left, const SharedPtr& right) {
//     return left.observed_ == right.observed_;
// }

template <typename T, typename U>
inline bool operator==(const SharedPtr<T>& left, const SharedPtr<U>& right) {
    return left.Get() == right.Get();
}

// template <typename T>
// inline bool operator==(const SharedPtr<T>& left, const SharedPtr<T>& right) {
//     return left.observed_ == right.observed_;
// }

template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
    SharedPtr<T> shr;
    auto cur = (new ControlBlockObj<T>(std::forward<Args>(args)...));
    shr.block_ = cur;
    shr.observed_ = (cur->GetPtr());
    shr.PutWeakThis();
    return shr;
}

// Look for usage examples in tests
class ESFTBase {};

template <typename T>
class EnableSharedFromThis : public ESFTBase {
public:
    SharedPtr<T> SharedFromThis() {
        return SharedPtr<T>(weak_this_);
    }
    SharedPtr<const T> SharedFromThis() const {
        return SharedPtr<const T>(weak_this_);
    }

    WeakPtr<T> WeakFromThis() noexcept {
        return WeakPtr<T>(weak_this_);
    }
    WeakPtr<const T> WeakFromThis() const noexcept {
        return WeakPtr<const T>(weak_this_);
    }

    ~EnableSharedFromThis() {
    }

private:
    WeakPtr<T> weak_this_;
    template <typename Y>
    friend class SharedPtr;
};
