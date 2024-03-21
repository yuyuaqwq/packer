#ifndef SEZZ_STL_VECTOR_HPP_
#define SEZZ_STL_VECTOR_HPP_

#include <sezz/detail/sezz_decl.hpp>
#include <vector>

namespace sezz {
template <typename T, typename Alloc>
struct Serializer<std::vector<T, Alloc>> {

    using Type = std::vector<T, Alloc>;

    template<typename OutputArchive>
    constexpr void Serialize(OutputArchive& ar, const Type& val) const {
        size_t size = val.size() * sizeof(T);
        ar.Save(size);
        for (auto& item : val) {
            ar.Save(item);
        }
        if (ar.ostream().fail()) {
            throw std::runtime_error("input stream read fail.");
        }
    }

    template<typename InputArchive>
    constexpr void Deserialize(InputArchive& ar, Type* out) const {
        auto size = ar.Load<size_t>();
        out->resize(size / sizeof(T));
        for (auto& o : *out) {
            std::construct_at(&o, ar.Load<T>());
        }
    }
};
} // namespace sezz


#endif // SEZZ_STL_VECTOR_HPP_