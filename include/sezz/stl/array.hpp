#ifndef SEZZ_STL_ARRAY_HPP_
#define SEZZ_STL_ARRAY_HPP_

#include <sezz/detail/sezz_decl.hpp>
#include <array>
#include <cassert>

namespace sezz {
template<typename T, size_t kSize>
struct Serializer<std::array<T, kSize>> {

    using Type = std::array<T, kSize>;

    template<typename OutputArchive>
    constexpr void Serialize(OutputArchive& ar, const Type& val) const {
        assert(val.size() == kSize);
        ar.Save(kSize);
        for (auto& v : val) {
            ar.Save(v);
        }
    }

    template<typename InputArchive>
    constexpr void Deserialize(InputArchive& ar, Type* out) const {
        auto size = ar.Load<size_t>();
        assert(size == kSize);
        for (size_t i = 0; i < size; i++) {
            out->at(i) = ar.Load<T>();
        }
    }
};
} // namespace sezz


#endif // SEZZ_STL_ARRAY_HPP_