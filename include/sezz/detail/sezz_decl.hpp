#pragma once

namespace sezz {
template <typename>
constexpr bool always_false = false;

template <typename T>
struct Serializer {
    static_assert(always_false<T>, "You haven't specialized the \"Serializer\" for this type T yet!");

    using Type = T;

    template<typename OutputArchive>
    constexpr void Serialize(OutputArchive& ar, const T& val) const {}

    template<typename InputArchive>
    constexpr void Deserialize(InputArchive& ar, T* out) const {}
};
}   // namespace sezz