#ifndef SEZZ_STL_UNORDERED_MAP_HPP_
#define SEZZ_STL_UNORDERED_MAP_HPP_

#include <sezz/stl/pair.hpp>
#include <unordered_map>
#include <ranges>

namespace sezz {
template <typename Key, typename Val, typename Hasher, typename KeyEq, typename Alloc>
struct Serializer<std::unordered_map<Key, Val, Hasher, KeyEq, Alloc>> {

    using Type = std::unordered_map<Key, Val, Hasher, KeyEq, Alloc>;

    template<typename OutputArchive>
    constexpr void Serialize(OutputArchive& ar, const Type& val) const {
        ar.Save(val.size());
        for (auto& v : val) {
            ar.Save(v);
        }
    }

    template<typename InputArchive>
    constexpr void Deserialize(InputArchive& ar, Type* out) const {
        auto size = ar.Load<size_t>();
        for (size_t i = 0; i < size; i++) {
            out->emplace(ar.Load<std::ranges::range_value_t<Type>>());
        }
    }
};
} // namespace sezz


#endif // SEZZ_STL_UNORDERED_MAP_HPP_