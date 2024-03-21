#ifndef SEZZ_STL_STRING_HPP_
#define SEZZ_STL_STRING_HPP_

#include <sezz/detail/sezz_decl.hpp>
#include <string>

namespace sezz {
template <typename Elem, typename Traits, typename Alloc>
struct Serializer<std::basic_string<Elem, Traits, Alloc>> {

    using Type = std::basic_string<Elem, Traits, Alloc>;

    template<typename OutputArchive>
    constexpr void Serialize(OutputArchive& ar, const Type& val) const {
        size_t size = val.size() * sizeof(Elem);
        ar.Save(size);
        ar.ostream().write(val.data(), size);
    }

    template<typename InputArchive>
    constexpr void Deserialize(InputArchive& ar, Type* out) const {
        auto size = ar.Load<size_t>();
        out->resize(size / sizeof(Elem), '\0');
        ar.istream().read(out->data(), size);
        if (ar.istream().fail()) {
            throw std::runtime_error("input stream read fail.");
        }
    }
};
} // namespace sezz


#endif // SEZZ_STL_STRING_HPP_