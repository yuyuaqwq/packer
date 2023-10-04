#ifndef SEZZ_SEZZ_HPP_
#define SEZZ_SEZZ_HPP_

#include <sezz/type_traits.hpp>

#include <unordered_map>
#include <map>
#include <set>
#include <unordered_set>
#include <string>
#include <vector>
#include <memory>
#include <optional>


namespace sezz {

namespace internal {

// Type

template<typename Archive, typename T>
using is_user_class_serializable_t = std::void_t<decltype(std::declval<T>().Serialize(std::declval<Archive&>()))>;

template<typename Archive, typename T>
using is_user_class_deserializable_t = std::void_t<decltype(std::declval<T>().Deserialize(std::declval<Archive&>()))>;

// Value

// memory copyable
template <class T>
constexpr bool is_memcopyable_v = std::is_trivially_copyable_v<T>;


// serializable class
template <typename Archive, class T>
constexpr bool is_user_class_serializable_v = type_traits::is_detected2_v<is_user_class_serializable_t, Archive, T>;
template <typename Archive, class T>
constexpr bool is_user_class_deserializable_v = type_traits::is_detected2_v<is_user_class_deserializable_t, Archive, T>;

template <class>
constexpr bool always_false = false;

} // namespace internal

template <class Archive, class T>
    requires internal::is_user_class_serializable_v<Archive, std::decay_t<T>> ||
             std::is_pointer_v<std::decay_t<T>> ||
             internal::is_memcopyable_v<std::decay_t<T>>
void Serialize(Archive& os, T&& val) {
    if constexpr (internal::is_user_class_serializable_v<Archive, T>) {
        val.Serialize(os);
    }
    else if constexpr (std::is_pointer_v<T>) {
        // Serialize(os, *val, args...);
        static_assert(internal::always_false<T>, "Serializing raw pointers is not supported!");
    }
    else if constexpr (internal::is_memcopyable_v<T>) {
        os.write((const char*)&val, sizeof(T));
    }
    else {
        printf("types that cannot be serialized: %s\n", typeid(T).name()); throw;
        //static_assert(internal::always_false<T>, "types that cannot be serialized.");
    }
    
}

template <class Archive, class... Types>
    requires (sizeof...(Types) > 1)
void Serialize(Archive& os, Types&&... vals) {
    (Serialize(os, std::forward<Types>(vals)), ...);
}

template <class T, class Archive>
    requires internal::is_user_class_deserializable_v<Archive, T> ||
             std::is_pointer_v<T> ||
             internal::is_memcopyable_v<T>
T Deserialize(Archive& is) {
    T res{};
    if constexpr (internal::is_user_class_deserializable_v<Archive, T>) {
        res.Deserialize(is);
    }
    else if constexpr (std::is_pointer_v<T>) {
        // res = new std::remove_pointer_t<DecayT>{ Deserialize<std::remove_pointer_t<DecayT>>(is) };
        // 不支持原始指针的原因是，需要通过new构造一个对象
        // 但在现代cpp中，原始指针代表的是引用一个在其生命周期内的对象，若使用者未注意就会造成内存泄漏
        static_assert(internal::always_false<T>, "Deserializing raw pointers is not supported!");
    }
    // 可直接内存复制的类型
    else if constexpr (internal::is_memcopyable_v<T>) {
        is.read((char*)&res, sizeof(T));
    }
    else {
        //printf("types that cannot be deserialized: %s\n", typeid(T).name()); throw;
        static_assert(internal::always_false<T>, "types that cannot be deserialized.");
    }
    return res;
}

template <class Archive, class... Types>
void Deserialize(Archive& is, Types&... buffers) {
    (Deserialize(is, buffers), ...);
}

} // namespace sezz


#endif // SEZZ_SEZZ_HPP_