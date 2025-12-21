#ifndef CONTAINERS_HASH_SET_HPP
#define CONTAINERS_HASH_SET_HPP

#include <vector>

namespace containers {

template <typename T>
struct Hasher {
    size_t operator()(const T& key) const {
        size_t hash = 2166136261U;
        for (auto c : key) {
            hash ^= static_cast<size_t>(c);
            hash *= 16777619U;
        }
        return hash;
    }
};

template <typename T>
class HashSet {
private:
    std::vector<std::vector<T>> buckets_;
    size_t num_elements_;
    size_t capacity_;
    static constexpr double kMaxLoadFactor = 1.0;

    void Rehash() {
        size_t new_capacity = capacity_ * 2 + 1;
        std::vector<std::vector<T>> new_buckets(new_capacity);
        Hasher<T> hasher;
        for (auto& bucket : buckets_) {
            for (auto& item : bucket) {
                new_buckets[hasher(item) % new_capacity].push_back(std::move(item));
            }
        }
        buckets_ = std::move(new_buckets);
        capacity_ = new_capacity;
    }

public:
    HashSet(size_t cap = 101) : capacity_(cap), num_elements_(0) {
        buckets_.resize(capacity_);
    }

    HashSet(const HashSet& other) {
        buckets_ = other.buckets_;
        num_elements_ = other.num_elements_;
        capacity_ = other.capacity_;
    }

    HashSet& operator=(const HashSet& other) {
        if (this != &other) {
            buckets_ = other.buckets_;
            num_elements_ = other.num_elements_;
            capacity_ = other.capacity_;
        }
        return *this;
    }

    void Insert(const T& key) {
        if (Contains(key)) return;
        if ((double)num_elements_ / capacity_ > kMaxLoadFactor) Rehash();
        Hasher<T> hasher;
        buckets_[hasher(key) % capacity_].push_back(key);
        num_elements_++;
    }

    bool Contains(const T& key) const {
        Hasher<T> hasher;
        const auto& bucket = buckets_[hasher(key) % capacity_];
        for (const auto& item : bucket) {
            if (item == key) return true;
        }
        return false;
    }

    void Erase(const T& key) {
        Hasher<T> hasher;
        auto& bucket = buckets_[hasher(key) % capacity_];
        for (auto it = bucket.begin(); it != bucket.end(); ++it) {
            if (*it == key) {
                bucket.erase(it);
                num_elements_--;
                return;
            }
        }
    }

    size_t Size() const { return num_elements_; }

    struct Iterator {
        const HashSet& set;
        size_t b_idx;
        size_t i_idx;
        void Advance() {
            while (b_idx < set.capacity_ && i_idx >= set.buckets_[b_idx].size()) {
                b_idx++; i_idx = 0;
            }
        }
        bool operator!=(const Iterator& other) const { return b_idx != other.b_idx || i_idx != other.i_idx; }
        const T& operator*() const { return set.buckets_[b_idx][i_idx]; }
        Iterator& operator++() { i_idx++; Advance(); return *this; }
    };

    Iterator begin() const { Iterator it{*this, 0, 0}; it.Advance(); return it; }
    Iterator end() const { return {*this, capacity_, 0}; }
};

} // namespace containers

#endif // CONTAINERS_HASH_SET_HPP

