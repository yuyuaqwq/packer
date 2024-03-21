#ifndef SEZZ_STL_SET_HPP_
#define SEZZ_STL_SET_HPP_

#include <sezz/detail/sezz_decl.hpp>
#include <set>
#include <ranges>

namespace sezz {
template <typename Key, typename Pr, typename Alloc>
struct Serializer<std::set<Key, Pr, Alloc>> {

    using Type = std::set<Key, Pr, Alloc>;

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


#endif // SEZZ_STL_SET_HPP_