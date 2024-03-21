#ifndef SEZZ_STL_TUPLE_HPP_
#define SEZZ_STL_TUPLE_HPP_

#include <sezz/detail/sezz_decl.hpp>
#include <cassert>
#include <tuple>

namespace sezz {
template <typename... Args>
struct Serializer<std::tuple<Args...>> {

    using Type = std::tuple<Args...>;
    static constexpr size_t kTupleSize = std::tuple_size_v<Type>;

    template<typename OutputArchive>
    constexpr void Serialize(OutputArchive& ar, const Type& val) const {
        ar.Save(kTupleSize);

        [&]<size_t... I>(std::index_sequence<I...>) {
            (ar.Save(std::get<I>(val)), ...);
        }(std::make_index_sequence<kTupleSize>{});
    }

    template<typename InputArchive>
    constexpr void Deserialize(InputArchive& ar, Type* out) const {
        auto size = ar.Load<size_t>();
        assert(size == kTupleSize);

        [&]<size_t... I>(std::index_sequence<I...>) {
            ((std::get<I>(*out) = ar.Load<Args>()), ...);
        }(std::make_index_sequence<kTupleSize>{});
    }
};
} // namespace sezz


#endif // SEZZ_STL_TUPLE_HPP_