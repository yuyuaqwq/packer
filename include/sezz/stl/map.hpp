#ifndef SEZZ_STL_MAP_HPP_
#define SEZZ_STL_MAP_HPP_

#include <sezz/stl/pair.hpp>
#include <map>
#include <ranges>

namespace sezz {
template <typename Key, typename Val, typename Pr, typename Alloc>
struct Serializer<std::map<Key, Val, Pr, Alloc>> {

    using Map = std::map<Key, Val, Pr, Alloc>;

    template<typename OutputArchive>
    constexpr void Serialize(OutputArchive& ar, const Map& val) const {
        ar.Save(val.size());
        for (auto& v : val) {
            ar.Save(v);
        }
    }

    template<typename InputArchive>
    constexpr void Deserialize(InputArchive& ar, Map* out) const {
        auto size = ar.Load<size_t>();
        for (size_t i = 0; i < size; i++) {
            out->emplace(ar.Load<std::ranges::range_value_t<Map>>());
        }
    }
};
} // namespace sezz


#endif // SEZZ_STL_MAP_HPP_