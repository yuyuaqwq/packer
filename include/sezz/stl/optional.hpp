#ifndef SEZZ_STL_OPTIONAL_HPP_
#define SEZZ_STL_OPTIONAL_HPP_

#include <sezz/detail/sezz_decl.hpp>
#include <optional>

namespace sezz {
template<typename T>
struct Serializer<std::optional<T>> {

    using Type = std::optional<T>;

    template<typename OutputArchive>
    constexpr void Serialize(OutputArchive& ar, const Type& val) const {
        bool has_value = val.has_value();
        ar.Save(has_value);
        if (has_value) {
            ar.Save(val.value());
        }
    }

    template<typename InputArchive>
    constexpr void Deserialize(InputArchive& ar, Type* out) const {
        if (ar.Load<bool>()) {
            *out = ar.Load<T>();
        }
        else {
            *out = std::nullopt;
        }
    }
};
} // namespace sezz


#endif // SEZZ_STL_OPTIONAL_HPP_