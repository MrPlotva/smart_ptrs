#pragma once

#include <cstddef>  // for std::nullptr_t
#include <type_traits>
#include <utility>  // for std::exchange / std::swap
#include <iostream>

class SimpleCounter {
public:
    size_t IncRef() {
        count_++;
        return count_;
    }
    size_t DecRef() {
        count_--;
        return count_;
    }
    size_t RefCount() const {
        return count_;
    }

private:
    size_t count_ = 0;
};

struct DefaultDelete {
    template <typename T>
    static void Destroy(T* object) {
        if (object) {
            delete object;
        }
    }
};

template <typename Derived, typename Counter, typename Deleter>
class RefCounted {
public:
    void IncRef() {
        counter_.IncRef();
    }

    void DecRef() {
        if (counter_.DecRef() == 0) {
            Deleter().Destroy(static_cast<Derived*>(this));
        }
    }

    size_t RefCount() const {
        return counter_.RefCount();
    }

private:
    Counter counter_;
};

template <typename Derived, typename D = DefaultDelete>
using SimpleRefCounted = RefCounted<Derived, SimpleCounter, D>;

template <typename T>
class IntrusivePtr {
    template <typename Y>
    friend class IntrusivePtr;

public:
    // Constructors
    IntrusivePtr() {
    }
    IntrusivePtr(std::nullptr_t) {
    }
    IntrusivePtr(T* ptr) {
        ptr_ = ptr;
        if (ptr_) {
            ptr_->IncRef();
        }
    }

    template <typename Y>
    IntrusivePtr(const IntrusivePtr<Y>& other) {
        ptr_ = other.ptr_;
        if (ptr_) {
            ptr_->IncRef();
        }
    }

    template <typename Y>
    IntrusivePtr(IntrusivePtr<Y>&& other) {
        ptr_ = other.ptr_;
        other.ptr_ = nullptr;
    }

    IntrusivePtr(const IntrusivePtr& other) {
        ptr_ = other.ptr_;
        if (ptr_) {
            ptr_->IncRef();
        }
    }
    IntrusivePtr(IntrusivePtr&& other) {
        ptr_ = other.ptr_;
        other.ptr_ = nullptr;
    }

    // `operator=`-s
    IntrusivePtr& operator=(const IntrusivePtr& other) {
        if (ptr_ == other.ptr_) {
            return *this;
        }
        if (ptr_) {
            ptr_->DecRef();
        }
        ptr_ = other.ptr_;
        if (ptr_) {
            ptr_->IncRef();
        }
        return *this;
    }
    IntrusivePtr& operator=(IntrusivePtr&& other) {
        if (ptr_ == other.ptr_) {
            // other.ptr_ = nullptr;
            return *this;
        }
        if (ptr_) {
            ptr_->DecRef();
        }
        ptr_ = other.ptr_;
        other.ptr_ = nullptr;
        return *this;
    }

    // Destructor
    ~IntrusivePtr() {
        if (ptr_) {
            if (UseCount() == 0) {
                delete ptr_;
                return;
            }
            ptr_->DecRef();
        }
    }

    // Modifiers
    void Reset() {
        if (ptr_) {
            ptr_->DecRef();
        }
        ptr_ = nullptr;
    }
    void Reset(T* ptr) {
        if (ptr_ == ptr) {
            return;
        }
        if (ptr_) {
            ptr_->DecRef();
        }
        ptr_ = ptr;
        if (ptr_) {
            ptr_->IncRef();
        }
    }
    void Swap(IntrusivePtr& other) {
        swap(ptr_, other.ptr_);
    }

    T* Get() const {
        return ptr_;
    }
    T& operator*() const {
        return *ptr_;
    }
    T* operator->() const {
        return ptr_;
    }
    size_t UseCount() const {
        if (!ptr_) {
            return 0;
        }
        return ptr_->RefCount();
    }
    explicit operator bool() const {
        return (ptr_ != nullptr);
    }

private:
    T* ptr_ = nullptr;
};

template <typename T, typename... Args>
IntrusivePtr<T> MakeIntrusive(Args&&... args) {
    IntrusivePtr<T> intr_ptr = IntrusivePtr<T>(new T(std::forward<Args>(args)...));
    return intr_ptr;
}
