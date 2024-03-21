#ifndef SEZZ_STL_PAIR_HPP_
#define SEZZ_STL_PAIR_HPP_

#include <sezz/detail/sezz_decl.hpp>
#include <utility>

namespace sezz {
template <typename Key, typename Val>
struct Serializer<std::pair<Key, Val>> {

    using Pair = std::pair<Key, Val>;

    template<typename OutputArchive>
    constexpr void Serialize(OutputArchive& ar, const Pair& val) const {
        ar.Save(val.first);
        ar.Save(val.second);
    }

    template<typename InputArchive>
    constexpr void Deserialize(InputArchive& ar, Pair* out) const {
        auto fst = ar.Load<std::remove_const_t<Key>>();
        auto sec = ar.Load<Val>();
        std::construct_at(out, std::move(fst), std::move(sec));
    }
};
} // namespace sezz


#endif // SEZZ_STL_PAIR_HPP_