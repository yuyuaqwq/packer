#ifndef SEZZ_STL_ARRAY_HPP_
#define SEZZ_STL_ARRAY_HPP_

#include <sezz/type_traits.hpp>
#include <array>

namespace sezz {
namespace detail {
template <typename T>
struct is_std_array : std::false_type {};

template <typename T, std::size_t N>
struct is_std_array<std::array<T, N>> : std::true_type {};

template <typename T>
constexpr bool is_std_array_v = is_std_array<T>::value;
} // namespace detail


template <class Archive, class T, class DecayT = std::decay_t<T>>
    requires detail::is_std_array_v<DecayT>
void Serialize(Archive& ar, T& val) {
    size_t size = val.size();
    ar.Save(size);
    for (auto& v : val) {
        ar.Save(v);
    }
}

template <class T, class Archive, class DecayT = std::decay_t<T>>
    requires detail::is_std_array_v<DecayT>
T Deserialize(Archive& ar) {
    auto size = ar.Load<size_t>();
    DecayT res{ 0 };
    for (int64_t i = 0; i < size; i++) {
        res[i] = ar.Load<typename DecayT::value_type>();
    }
    return res;
}

} // namespace sezz


#endif // SEZZ_STL_ARRAY_HPP_