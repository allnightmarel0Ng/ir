#ifndef SEARCH_SET_OPERATIONS_HPP
#define SEARCH_SET_OPERATIONS_HPP

#include "containers/hash_set.hpp"
#include <string>

namespace search {

using DocID = std::string;

template <typename T>
containers::HashSet<T> SetAnd(
    const containers::HashSet<T>& a,
    const containers::HashSet<T>& b
) {
    if (a.Size() > b.Size()) {
        containers::HashSet<T> result;
        for (const auto& x : b) {
            if (a.Contains(x)) {
                result.Insert(x);
            }
        }
        return result;
    } else {
        containers::HashSet<T> result;
        for (const auto& x : a) {
            if (b.Contains(x)) {
                result.Insert(x);
            }
        }
        return result;
    }
}

template <typename T>
containers::HashSet<T> SetOr(
    const containers::HashSet<T>& a,
    const containers::HashSet<T>& b
) {
    auto result = a;
    for (const auto& id : b) {
        result.Insert(id);
    }
    return result;
}

} // namespace search

#endif // SEARCH_SET_OPERATIONS_HPP

