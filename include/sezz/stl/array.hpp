#ifndef SEZZ_STL_ARRAY_HPP_
#define SEZZ_STL_ARRAY_HPP_

#include <array>

namespace sezz {
template<typename T, size_t kSize>
struct Serializer<std::array<T, kSize>> {
    using Array = std::array<T, kSize>;

    template<typename OutputArchive>
    constexpr void Serialize(OutputArchive& ar, const Array& val) const {
        size_t size = val.size();
        ar.Save(size);
        for (auto& v : val) {
            ar.Save(v);
        }
    }

    template<typename InputArchive>
    constexpr void Deserialize(InputArchive& ar, Array* out) const {
        auto size = ar.Load<size_t>();
        for (size_t i = 0; i < size; i++) {
            out->at(i) = ar.Load<std::ranges::range_value_t<Array>>();
        }
    }
};
} // namespace sezz


#endif // SEZZ_STL_ARRAY_HPP_