#ifndef CONTAINERS_HASH_MAP_HPP
#define CONTAINERS_HASH_MAP_HPP

#include "hash_set.hpp"
#include <vector>

namespace containers {

template <typename K, typename V>
class HashMap {
private:
    struct Node { K key; V value; };
    std::vector<std::vector<Node>> buckets_;
    size_t capacity_;
    size_t num_elements_;
    static constexpr double kMaxLoadFactor = 1.0;

    void Rehash() {
        size_t new_capacity = capacity_ * 2 + 1;
        std::vector<std::vector<Node>> new_buckets(new_capacity);
        Hasher<K> hasher;
        for (auto& bucket : buckets_) {
            for (auto& node : bucket) {
                new_buckets[hasher(node.key) % new_capacity].push_back(std::move(node));
            }
        }
        buckets_ = std::move(new_buckets);
        capacity_ = new_capacity;
    }

public:
    HashMap(size_t cap = 1009) : capacity_(cap), num_elements_(0) {
        buckets_.resize(capacity_);
    }

    HashMap(const HashMap& other) : buckets_(other.buckets_), capacity_(other.capacity_), num_elements_(other.num_elements_) {}
    
    HashMap& operator=(const HashMap& other) {
        if (this != &other) {
            buckets_ = other.buckets_;
            capacity_ = other.capacity_;
            num_elements_ = other.num_elements_;
        }
        return *this;
    }

    V& operator[](const K& key) {
        Hasher<K> hasher;
        size_t idx = hasher(key) % capacity_;
        for (auto& node : buckets_[idx]) {
            if (node.key == key) return node.value;
        }
        if ((double)num_elements_ / capacity_ > kMaxLoadFactor) {
            Rehash();
            idx = hasher(key) % capacity_; 
        }
        buckets_[idx].push_back({key, V()});
        num_elements_++;
        return buckets_[idx].back().value;
    }

    size_t Size() const { return num_elements_; }

    struct Iterator {
        const HashMap& map;
        size_t b_idx;
        size_t i_idx;
        void Advance() {
            while (b_idx < map.capacity_ && i_idx >= map.buckets_[b_idx].size()) {
                b_idx++; i_idx = 0;
            }
        }
        bool operator!=(const Iterator& other) const { return b_idx != other.b_idx || i_idx != other.i_idx; }
        const Node& operator*() const { return map.buckets_[b_idx][i_idx]; }
        Iterator& operator++() { i_idx++; Advance(); return *this; }
    };

    Iterator begin() const { Iterator it{*this, 0, 0}; it.Advance(); return it; }
    Iterator end() const { return {*this, capacity_, 0}; }
};

} // namespace containers

#endif // CONTAINERS_HASH_MAP_HPP

