#pragma once

#include <exception>
#include <tuple>

namespace mp {

struct nullopt_t {};

struct inplace_t {};

class bad_optional_access : public std::exception {
public:
    using std::exception::exception;
};

template <typename... Ts>
class optional {
    template <size_t Index>
    using element_type = typename std::tuple_element<Index, std::tuple<Ts...>>::type;

public:
    optional() : has_value_(false) {}
    optional(nullopt_t) : has_value_(false) {}
    optional(const optional &other) : has_value_(false) {
        if (other.has_value_) {
            emplace(other.value_);
        }
    }
    optional(optional &&other) : has_value_(false) {
        if (other.has_value_) {
            emplace(std::move(other.value_));
            other.reset();
        }
    }
    template <typename... Us>
    optional(inplace_t, Us &&...us) : has_value_(false) {
        emplace(std::forward<Us>(us)...);
    }

    ~optional() {
        reset();
    }

    optional &operator=(optional &&other) {
        reset();
        if (other.has_value_) {
            emplace(std::move(other.value_));
            other.reset();
        }
    }
    optional &operator=(const optional &other) {
        reset();
        if (other.has_value_) {
            emplace(other.value_);
        }
    }

    bool operator==(const optional &other) const {
        if (has_value_ != other.has_value_) {
            return false;
        }

        return !has_value_ || value_ == other.value_;
    }

    bool has_value() const {
        return has_value_;
    }

    template <size_t Index>
    const element_type<Index> &value() const {
        if (!has_value_) {
            throw bad_optional_access();
        }
        return std::get<Index>(value_);
    }

    template <size_t Index>
    element_type<Index> &value() {
        if (!has_value_) {
            throw bad_optional_access();
        }
        return std::get<Index>(value_);
    }

    template <size_t Index>
    element_type<Index> value_or(element_type<Index> fallback) const {
        if (has_value_) {
            return std::get<Index>(value_);
        } else {
            return fallback;
        }
    }

    void reset() {
        if (has_value_) {
            using T = std::tuple<Ts...>;
            value_.~T();
        }
    }

    void set(std::tuple<Ts...> &&input) {
        reset();
        has_value_ = true;
        using T = std::tuple<Ts...>;
        ::new (&value_) T(std::move(input));
    }

    template <typename... Us>
    void emplace(Us &&...us) {
        reset();
        has_value_ = true;
        using T = std::tuple<Ts...>;
        ::new (&value_) T(std::forward<Us>(us)...);
    }

private:
    bool has_value_;
    union {
        std::tuple<Ts...> value_;
    };
};

template <typename T>
class optional<T> {
public:
    optional() : has_value_(false) {}
    optional(nullopt_t) : has_value_(false) {}
    optional(const optional &other) : has_value_(false) {
        if (other.has_value_) {
            emplace(other.value_);
        }
    }
    optional(optional &&other) : has_value_(false) {
        if (other.has_value_) {
            emplace(std::move(other.value_));
            other.reset();
        }
    }
    template <typename... Us>
    optional(inplace_t, Us &&...us) : has_value_(false) {
        emplace(std::forward<Us>(us)...);
    }

    ~optional() {
        reset();
    }

    optional &operator=(optional &&other) {
        reset();
        if (other.has_value_) {
            emplace(std::move(other.value_));
            other.reset();
        }
    }
    optional &operator=(const optional &other) {
        reset();
        if (other.has_value_) {
            emplace(other.value_);
        }
    }

    bool operator==(const optional &other) const {
        if (has_value_ != other.has_value_) {
            return false;
        }

        return !has_value_ || value_ == other.value_;
    }

    bool has_value() const {
        return has_value_;
    }

    const T *operator->() const {
        if (!has_value_) {
            throw bad_optional_access();
        }
        return &value_;
    }

    T *operator->() {
        if (!has_value_) {
            throw bad_optional_access();
        }
        return &value_;
    }

    template <size_t Index = 0>
    const T &value() const {
        static_assert(Index == 0, "index out of range");
        if (!has_value_) {
            throw bad_optional_access();
        }
        return value_;
    }

    template <size_t Index = 0>
    T &value() {
        if (!has_value_) {
        static_assert(Index == 0, "index out of range");
            throw bad_optional_access();
        }
        return value_;
    }

    template <size_t Index = 0>
    T value_or(T fallback) const {
        static_assert(Index == 0, "index out of range");
        if (has_value_) {
            return value_;
        } else {
            return fallback;
        }
    }

    void reset() {
        if (has_value_) {
            value_.~T();
        }
    }

    void set(std::tuple<T> &&input) {
        reset();
        has_value_ = true;
        ::new (&value_) T(std::move(std::get<0>(input)));
    }

    template <typename... Us>
    void emplace(Us &&...us) {
        reset();
        has_value_ = true;
        ::new (&value_) T(std::forward<Us>(us)...);
    }

private:
    bool has_value_;
    union {
        T value_;
    };
};

}