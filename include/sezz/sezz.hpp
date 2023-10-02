#ifndef SEZZ_SEZZ_HPP_
#define SEZZ_SEZZ_HPP_

#include <iostream>
#include <typeinfo>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <string>
#include <vector>
#include <optional>


namespace sezz {

namespace internal {
#pragma region 内部定义
template<template<class> class, class, class = void>
struct is_detected : std::false_type {};
template<template<class T> class OpT, class T>
struct is_detected<OpT, T, std::void_t<OpT<T>>> : std::true_type {};

template<template<class, class> class, class, class, class = void>
struct is_detected2 : std::false_type {};
template<template<class T1, class T2> class OpT, class T1, class T2>
struct is_detected2<OpT, T1, T2, std::void_t<OpT<T1, T2>>> : std::true_type {};

template<template<class T> class OpT, class T>
constexpr bool is_detected_v = is_detected<OpT, T>::value;
template<template<class T1, class T2> class OpT, class T1, class T2>
constexpr bool is_detected2_v = is_detected2<OpT, T1, T2>::value;

template <template <class...> class T>
struct TemplateType {};

template <class T>
struct ExtractTemplate {
    static constexpr bool IsTemplate = false;
};

template <template <class...> class T, class... Args>
struct ExtractTemplate<T<Args...>> {
    static constexpr bool IsTemplate = true;
    using Type = TemplateType<T>;
};

template <class T, class U>
constexpr bool is_same_template() {
    if constexpr (ExtractTemplate<T>::IsTemplate != ExtractTemplate<U>::IsTemplate) {
        return false;
    }
    else {
        return std::is_same_v<typename ExtractTemplate<T>::Type, typename ExtractTemplate<U>::Type>;
    }
}

template <class T, class U>
constexpr bool is_same_template_v = is_same_template<T, U>();
#pragma endregion
// Type

template<typename T>
using is_iterable_t = std::void_t<decltype(std::begin(std::declval<T>())), decltype(std::end(std::declval<T>()))>;

template<typename T>
using is_unique_ptr_t = std::void_t<decltype(&T::operator->), decltype(&T::operator*)>;

template<typename Archive, typename T>
using is_user_class_serializable_t = std::void_t<decltype(std::declval<T>().Serialize(std::declval<Archive&>()))>;

template<typename Archive, typename T>
using is_user_class_deserializable_t = std::void_t<decltype(std::declval<T>().Deserialize(std::declval<Archive&>()))>;

using place_t = char;

// Value

// pair
template <class T>
constexpr bool is_pair_v = is_same_template_v<T, std::pair<place_t, place_t>>;

// optional
template <class T>
constexpr bool is_optional_v = is_same_template_v<T, std::optional<place_t>>;

// unordered_map
template <class T>
constexpr bool is_unordered_map_v = is_same_template_v<T, std::unordered_map<place_t, place_t>>;

// unordered_set
template <class T>
constexpr bool is_unordered_set_v = is_same_template_v<T, std::unordered_set<place_t>>;

// map
template <class T>
constexpr bool is_map_v = is_same_template_v<T, std::map<place_t, place_t>>;

// set
template <class T>
constexpr bool is_set_v = is_same_template_v<T, std::set<place_t>>;

// string
template <class T>
constexpr bool is_string_v = is_same_template_v<T, std::basic_string<place_t>>;

// vector
template <class T>
constexpr bool is_vector_v = is_same_template_v<T, std::vector<place_t>>;

// unique_ptr
template <class T>
constexpr bool is_unique_ptr_v = is_detected_v<is_unique_ptr_t, T>;

// memory copyable
template <class T>
constexpr bool is_memcopyable_v = std::is_trivially_copyable_v<T>;

// iterative
template <class T>
constexpr bool is_iterable_v = is_detected_v<is_iterable_t, T>;

// serializable class
template <typename Archive, class T>
constexpr bool is_user_class_serializable_v = is_detected2_v<is_user_class_serializable_t, Archive, T>;
template <typename Archive, class T>
constexpr bool is_user_class_deserializable_v = is_detected2_v<is_user_class_deserializable_t, Archive, T>;

template <class>
constexpr bool always_false = false;


} // namespace internal

template <class Archive, class T, class... Types>
void Serialize(Archive& os, T& val, Types&... args) {
    using DecayT = std::decay_t<T>;
    if constexpr (internal::is_user_class_serializable_v<Archive, T>) {
        val.Serialize(os);
    }
    else if constexpr (internal::is_optional_v<T>) {
        bool has_value = val.has_value();
        Serialize(os, has_value);
        if (has_value) {
            Serialize(os, val.value());
        }
    }
    else if constexpr (std::is_pointer_v<DecayT>) {
        // Serialize(os, *val, args...);
        static_assert(internal::always_false<T>, "Serializing raw pointers is not supported!");
    }
    else if constexpr (internal::is_unique_ptr_v<DecayT>) {
        Serialize(os, *val, args...);
    }
    else if constexpr (internal::is_pair_v<DecayT>) {
        Serialize(os, val.first);
        Serialize(os, val.second);
    }
    else if constexpr (internal::is_iterable_v<DecayT>) {
        uint32_t size = val.size();
        os.write((const char*)&size, sizeof(size));
        if constexpr (internal::is_memcopyable_v<DecayT>) {
            os.write((const char*)&val, size * sizeof(typename T::value_type));
        }
        else {
            for (auto& v : val) { 
                Serialize(os, v); 
            }
        }
    }
    else if constexpr (internal::is_memcopyable_v<T>) {
        os.write((const char*)&val, sizeof(T));
    }
    else {
        //printf("types that cannot be serialized: %s\n", typeid(T).name()); throw;
        static_assert(internal::always_false<T>, "types that cannot be serialized.");
    }
    (Serialize(os, args), ...);
}

template <class T, class Archive>
T Deserialize(Archive& is) {
    using DecayT = std::decay_t<T>;
    uint32_t size = 0;
    DecayT res{};
    if constexpr (internal::is_user_class_deserializable_v<Archive, T>) {
        res.Deserialize(is);
    }
    else if constexpr (internal::is_optional_v<T>) {
        bool has_value = Deserialize<bool>(is);
        if (has_value) {
            ::new(&res) T(Deserialize<typename T::value_type>(is));
        }
    }
    else if constexpr (std::is_pointer_v<DecayT>) {
        // res = new std::remove_pointer_t<DecayT>{ Deserialize<std::remove_pointer_t<DecayT>>(is) };
        // 不支持原始指针的原因是，需要通过new构造一个对象
        // 但在现代cpp中，原始指针代表的是引用一个在其生命周期内的对象，若使用者未注意就会造成内存泄漏
        static_assert(internal::always_false<T>, "Deserializing raw pointers is not supported!");
    }
    else if constexpr (internal::is_unique_ptr_v<DecayT>) {
        using DecayRawT = std::decay_t<DecayT::element_type>;
        res = std::make_unique<DecayRawT>(Deserialize<DecayRawT>(is));
    }
    // pair
    else if constexpr (internal::is_pair_v<DecayT>) {
        auto first = Deserialize<typename T::first_type>(is);
        auto second = Deserialize<typename T::second_type>(is);
        ::new(&res) T(first, second);
    }
    // 可迭代的容器类型
    else if constexpr (internal::is_iterable_v<DecayT>) {
        // 可直接内存复制的容器类型, 比如vector<char>
        if constexpr (internal::is_memcopyable_v<DecayT>) {
            is.read((char*)&size, sizeof(uint32_t));
            res.resize(size);
            is.read((char*)res.data(), size * sizeof(typename T::value_type));
        }
        /*
        * 不可直接内存复制的容器类型
        * 比如std::vector<std::string>
        * string不可直接内存复制
        */
        else {
            is.read((char*)&size, sizeof(uint32_t));
            if constexpr (internal::is_vector_v<DecayT>) {
                res.resize(size);
            }
            for (uint32_t i = 0; i < size; i++) {
                auto tmp = Deserialize<typename T::value_type>(is);     // 反序列化T的元素, 比如std::string
                // basic_string
                if constexpr (internal::is_string_v<DecayT>) {
                    res.insert(i, 1, std::move(tmp));
                }
                // unordered_map || map || unordered_set || set
                else if constexpr (internal::is_unordered_map_v<DecayT> || internal::is_map_v<DecayT> || internal::is_unordered_set_v<DecayT> || internal::is_set_v<DecayT>) {
                    res.insert(std::move(tmp));
                }
                else if constexpr (internal::is_vector_v<DecayT>) {
                    res[i] = std::move(tmp);
                }
                else {
                    //printf("unrealized container: %s\n", typeid(DecayT).name()); throw;       // 运行时查看未实现的类型
                    static_assert(internal::always_false<T>, "unrealized container.");
                }
            }
        }
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

template <class Archive, class T, class... Types>
void Deserialize(Archive& is, T& val, Types&... args) {
    val = Deserialize<T>(is);
    (Deserialize(is, args), ...);
}

} // namespace sezz


#endif // SEZZ_SEZZ_HPP_