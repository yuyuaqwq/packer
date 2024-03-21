#ifndef SEZZ_STL_ARRAY_HPP_
#define SEZZ_STL_ARRAY_HPP_

#include <array>
#include <sezz/detail/sezz_decl.hpp>

namespace sezz {
template<typename T, size_t kSize>
struct Serializer<std::array<T, kSize>> {

    using Array = std::array<T, kSize>;

    template<typename OutputArchive>
    constexpr void Serialize(OutputArchive& ar, const Array& val) const {
        ar.Save(val.size());
        for (auto& v : val) {
            ar.Save(v);
        }
    }

    template<typename InputArchive>
    constexpr void Deserialize(InputArchive& ar, Array* out) const {
        auto size = ar.Load<size_t>();
        for (size_t i = 0; i < size; i++) {
            out->at(i) = ar.Load<T>();
        }
    }
};
} // namespace sezz


#endif // SEZZ_STL_ARRAY_HPP_