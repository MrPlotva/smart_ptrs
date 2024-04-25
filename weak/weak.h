#pragma once

#include <cstddef>
#include "sw_fwd.h"  // Forward declaration
#include "weak/shared.h"

// https://en.cppreference.com/w/cpp/memory/weak_ptr

class ControlBlock;

template <typename T>
class WeakPtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors
    template <typename Y>
    friend class SharedPtr;

    WeakPtr() {
    }

    WeakPtr(const WeakPtr& other) {
        block_ = other.block_;
        observed_ = other.observed_;
        if (block_ != nullptr) {
            block_->IncWeak();
        }
    }
    WeakPtr(WeakPtr&& other) {
        block_ = other.block_;
        observed_ = other.observed_;
        other.observed_ = nullptr;
        other.block_ = nullptr;
    }
    // Demote `SharedPtr`
    // #2 from https://en.cppreference.com/w/cpp/memory/weak_ptr/weak_ptr
    WeakPtr(const SharedPtr<T>& other) {
        block_ = other.block_;
        observed_ = other.observed_;
        if (block_ != nullptr) {
            block_->IncWeak();
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    WeakPtr& operator=(const WeakPtr& other) {
        if (observed_ == other.observed_) {
            return *this;
        }
        Clear();
        block_ = other.block_;
        observed_ = other.observed_;
        if (block_ != nullptr) {
            block_->IncWeak();
        }
        return *this;
    }

    WeakPtr& operator=(WeakPtr&& other) {
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

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~WeakPtr() {
        Clear();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        Clear();
        block_ = nullptr;
        observed_ = nullptr;
    }
    void Swap(WeakPtr& other) {
        std::swap(block_, other.block_);
        std::swap(observed_, other.observed_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    size_t UseCount() const {
        if (!block_) {
            return 0;
        }
        return block_->GetStrongCount();
    }
    bool Expired() const {
        if (block_ == nullptr) {
            return true;
        }
        return (block_->GetStrongCount() == 0);
    }
    SharedPtr<T> Lock() const {
        if (!Expired()) {
            return SharedPtr<T>(*this);
        } else {
            return SharedPtr<T>();
        }
    }

private:
    ControlBlock* block_ = nullptr;
    T* observed_ = nullptr;
    void Clear() {
        if (block_ != nullptr) {
            block_->DecWeak();
        }
    }
    template <typename Y>
    friend class WeakPtr;

    // template <typename U>
    // friend inline bool operator==(const SharedPtr<T>& left, const SharedPtr<U>& right);

    // template <typename U, typename... Args>
    // friend SharedPtr<U> MakeShared(Args&&... args);
};
