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
// Type

// pair
template<typename T>
struct is_pair_t : std::false_type {};
template<typename T1, typename T2>
struct is_pair_t<std::pair<T1, T2>> : std::true_type {};


// optional
template<typename T>
struct is_optional_t : std::false_type {};
template<typename T>
struct is_optional_t<std::optional<T>> : std::true_type {};

// map
template<typename T>
struct is_map_t : std::false_type {};
template<typename T1, typename T2>
struct is_map_t<std::map<T1, T2>> : std::true_type {};
template<typename T1, typename T2, typename T3>
struct is_map_t<std::map<T1, T2, T3>> : std::true_type {};

// unordered_map
template<typename T>
struct is_unordered_map_t : std::false_type {};
template<typename T1, typename T2>
struct is_unordered_map_t<std::unordered_map<T1, T2>> : std::true_type {};

// set
template<typename T>
struct is_set_t : std::false_type {};
template<typename T1, typename T2>
struct is_set_t<std::set<T1, T2>> : std::true_type {};

// unordered_set
template<typename T>
struct is_unordered_set_t : std::false_type {};
template<typename T1, typename T2>
struct is_unordered_set_t<std::unordered_set<T1, T2>> : std::true_type {};

// basic_string
template<typename T>
struct is_string_t : std::false_type {};
template<typename T>
struct is_string_t<std::basic_string<T>> : std::true_type {};

// vector
template<typename T>
struct is_vector_t : std::false_type {};
template<typename T>
struct is_vector_t<std::vector<T>> : std::true_type {};

// iterative
template<typename T, typename = void>
struct is_iterable_t : std::false_type {};
template<typename T>
struct is_iterable_t<T, std::void_t<decltype(std::begin(std::declval<T>())), decltype(std::end(std::declval<T>()))>> : std::true_type {};

// unique_ptr
template<typename T, typename = void>
struct is_unique_ptr_t : std::false_type {};
template<typename T>
struct is_unique_ptr_t<T, std::void_t<decltype(&T::operator->), decltype(&T::operator*)>> : std::true_type {};

// serializable class
template<typename Archive, typename T, typename = void>
struct is_user_class_serializable_t : std::false_type {};
template<typename Archive, typename T>
struct is_user_class_serializable_t<Archive, T, std::void_t<decltype(std::declval<T>().Serialize(std::declval<Archive&>()))>> : std::true_type {};

template<typename Archive, typename T, typename = void>
struct is_user_class_deserializable_t : std::false_type {};
template<typename Archive, typename T>
struct is_user_class_deserializable_t<Archive, T, std::void_t<decltype(std::declval<T>().Serialize(std::declval<Archive&>()))>> : std::true_type {};


// Value

// pair
template <class T>
constexpr bool is_pair_v = internal::is_pair_t<T>::value;

// optional
template <class T>
constexpr bool is_optional_v = internal::is_optional_t<T>::value;

// memory copyable
template <class T>
constexpr bool is_memcopyable_v = std::is_trivially_copyable_v<T>;

// iterative
template <class T>
constexpr bool is_iterable_v = internal::is_iterable_t<T>::value;

// unordered_map
template <class T>
constexpr bool is_unordered_map_v = internal::is_unordered_map_t<T>::value;

// map
template <class T>
constexpr bool is_map_v = internal::is_map_t<T>::value;

// unordered_set
template <class T>
constexpr bool is_unordered_set_v = internal::is_unordered_set_t<T>::value;

// set
template <class T>
constexpr bool is_set_v = internal::is_set_t<T>::value;

// string
template <class T>
constexpr bool is_string_v = internal::is_string_t<T>::value;

// vector
template <class T>
constexpr bool is_vector_v = internal::is_vector_t<T>::value;

// unique_ptr
template <class T>
constexpr bool is_unique_ptr_v = internal::is_unique_ptr_t<T>::value;

// serializable class
template <typename Archive, class T>
constexpr bool is_user_class_serializable_v = internal::is_user_class_serializable_t<Archive, T>::value;
template <typename Archive, class T>
constexpr bool is_user_class_deserializable_v = internal::is_user_class_deserializable_t<Archive, T>::value;

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