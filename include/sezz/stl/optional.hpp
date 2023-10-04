#ifndef SEZZ_STL_OPTIONAL_HPP_
#define SEZZ_STL_OPTIONAL_HPP_

#include <sezz/type_traits.hpp>
#include <optional>

namespace sezz {

template <class Archive, class T>
    requires type_traits::is_same_template_v<std::decay_t<T>, std::optional<type_traits::place_t>>
void Serialize(Archive& os, T& val) {
    bool has_value = val.has_value();
    Serialize(os, has_value);
    if (has_value) {
        Serialize(os, val.value());
    }
}

template <class T, class Archive>
    requires type_traits::is_same_template_v<std::decay_t<T>, std::optional<type_traits::place_t>>
T Deserialize(Archive& is) {
    bool has_value = Deserialize<bool>(is);
    if (has_value) {
        return Deserialize<typename T::value_type>(is);
    } else {
        return std::nullopt;
    }
}

} // namespace sezz


#endif // SEZZ_STL_OPTIONAL_HPP_