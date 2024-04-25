#pragma once

#pragma once

#include <bits/iterator_concepts.h>
#include <memory>
#include <type_traits>
#include <iostream>
#include <utility>

template <typename V>
inline constexpr bool comp_is_compressed_v = std::is_empty_v<V> && !std::is_final_v<V>;  // NOLINT

template <typename F, typename S, typename COMP = void>
class CompressedPair {
    CompressedPair() {
    }
};
template <typename F, typename S>
class CompressedPair<
    F, S,
    std::enable_if_t<comp_is_compressed_v<F> && comp_is_compressed_v<S> && !(std::is_same_v<F, S>)>>
    : F, S {
public:
    CompressedPair() {
    }
    CompressedPair(const F& first, const S& second) {
    }

    F& GetFirst() {
        return *this;
    }

    const F& GetFirst() const {
        return *this;
    }

    S& GetSecond() {
        return *this;
    };

    const S& GetSecond() const {
        return *this;
    };

    CompressedPair& operator=(CompressedPair&& other) {
        return *this;
    }

    CompressedPair& operator=(CompressedPair& other) {
        return *this;
    }
};

template <typename F, typename S>
class CompressedPair<F, S, std::enable_if_t<!comp_is_compressed_v<F> && comp_is_compressed_v<S>>>
    : S {
public:
    CompressedPair() {
    }
    CompressedPair(const F& first, const S& second) : first_(first) {
    }

    CompressedPair(F&& first, const S&& second) : first_(std::move(first)) {
    }

    CompressedPair(F&& first, const S& second) : first_(std::move(first)) {
    }

    CompressedPair(const F& first) : first_(first) {
    }

    CompressedPair(F&& first) : first_(std::move(first)) {
    }

    F& GetFirst() {
        return first_;
    }

    const F& GetFirst() const {
        return first_;
    }

    S& GetSecond() {
        return *this;
    };

    const S& GetSecond() const {
        return *this;
    };

    CompressedPair& operator=(CompressedPair&& other) {
        first_ = std::move(other.first_);
        return *this;
    }

    CompressedPair& operator=(CompressedPair& other) {
        first_ = other.first_;
        return *this;
    }

private:
    F first_;
};

template <typename F, typename S>
class CompressedPair<F, S, std::enable_if_t<comp_is_compressed_v<F> && !comp_is_compressed_v<S>>>
    : F {
public:
    CompressedPair() {
    }
    CompressedPair(const F& first, const S& second) : second_(second) {
    }

    CompressedPair(F&& first, const S& second) : second_(second) {
    }

    CompressedPair(const F& first, S&& second) : second_(std::move(second)) {
    }

    CompressedPair(const S& second) : second_(second) {
    }

    CompressedPair(S&& second) : second_(std::move(second)) {
    }

    F& GetFirst() {
        return *this;
    }

    const F& GetFirst() const {
        return *this;
    }

    S& GetSecond() {
        return second_;
    };

    const S& GetSecond() const {
        return second_;
    };

    CompressedPair& operator=(CompressedPair&& other) {
        second_ = std::move(other.second_);
        return *this;
    }

    CompressedPair& operator=(CompressedPair& other) {
        second_ = other.second_;
        return *this;
    }

private:
    S second_;
};

template <typename F, typename S>
class CompressedPair<F, S,
                     std::enable_if_t<(!comp_is_compressed_v<F> && !comp_is_compressed_v<S>) ||
                                      (std::is_same_v<F, S>)>> {
public:
    CompressedPair() {
        first_ = F();
        second_ = S();
    }
    CompressedPair(const F& first, const S& second) : first_(first), second_(second) {
    }

    CompressedPair(F&& first, S& second) : first_(std::move(first)), second_(second) {
    }

    CompressedPair(F& first, S&& second) : first_(first), second_(std::move(second)) {
    }

    ~CompressedPair() {
    }

    F& GetFirst() {
        return first_;
    }

    const F& GetFirst() const {
        return first_;
    }

    S& GetSecond() {
        return second_;
    };

    const S& GetSecond() const {
        return second_;
    };

    CompressedPair& operator=(CompressedPair&& other) {
        first_ = std::move(other.first_);
        second_ = std::move(other.second_);
        return *this;
    }

    CompressedPair& operator=(CompressedPair& other) {
        first_ = other.first_;
        second_ = std::move(other.second_);
        return *this;
    }

private:
    F first_;
    S second_;
};
