#ifndef SEARCH_BOOLEAN_SEARCH_HPP
#define SEARCH_BOOLEAN_SEARCH_HPP

#include "containers/hash_map.hpp"
#include "containers/hash_set.hpp"
#include "search/set_operations.hpp"
#include <string>

namespace search {

using InvertedIndex = containers::HashMap<std::wstring, containers::HashSet<DocID>>;

containers::HashSet<DocID> BooleanSearchRu(const std::string& query, InvertedIndex& index);

} // namespace search

#endif // SEARCH_BOOLEAN_SEARCH_HPP

