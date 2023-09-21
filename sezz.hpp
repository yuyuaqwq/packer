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



namespace sezz {

namespace details {
/*
* 对原始指针和其他智能指针的支持构思
* 需要引入运行时哈希表
* 在序列化时如果发现是原始指针/std::shared_ptr/weak_ptr，则判断在之前是否以及序列化过了(std::unique_ptr/std::shared_ptr)，是则序列化一个编号(不是实际值)
* 在反序列化时，如果是原始指针/std::shared_ptr，则基于编号索引先前反序列化时再次构造的表(在反序列化std::unique_ptr/std::shared_ptr是会构建)，使其指向同一份数据
*/


// std::pair
template<typename T>
struct is_pair_t : std::false_type {};
template<typename T1, typename T2>
struct is_pair_t<std::pair<T1, T2>> : std::true_type {};

// std::map
template<typename T>
struct is_map_t : std::false_type {};
template<typename T1, typename T2>
struct is_map_t<std::map<T1, T2>> : std::true_type {};

// std::unordered_map
template<typename T>
struct is_unordered_map_t : std::false_type {};
template<typename T1, typename T2>
struct is_unordered_map_t<std::unordered_map<T1, T2>> : std::true_type {};

// std::set
template<typename T>
struct is_set_t : std::false_type {};
template<typename T1, typename T2>
struct is_set_t<std::set<T1, T2>> : std::true_type {};

// std::unordered_set
template<typename T>
struct is_unordered_set_t : std::false_type {};
template<typename T1, typename T2>
struct is_unordered_set_t<std::unordered_set<T1, T2>> : std::true_type {};

// std::basic_string
template<typename T>
struct is_string_t : std::false_type {};
template<typename T>
struct is_string_t<std::basic_string<T>> : std::true_type {};

// std::vector
template<typename T>
struct is_vector_t : std::false_type {};
template<typename T>
struct is_vector_t<std::vector<T>> : std::true_type {};

// 可迭代
template<typename T, typename = void>
struct is_iterable_t : std::false_type {};
template<typename T>
struct is_iterable_t<T, std::void_t<decltype(std::begin(std::declval<T>())), decltype(std::end(std::declval<T>()))>> : std::true_type {};

// 智能指针
template<typename T, typename = void>
struct is_unique_ptr_t : std::false_type {};
template<typename T>
struct is_unique_ptr_t<T, std::void_t<decltype(&T::operator->), decltype(&T::operator*)>> : std::true_type {};

// 用户自定义类
template<typename T, typename = void>
struct is_user_serializable_t : std::false_type {};
template<typename T>
struct is_user_serializable_t<T, std::void_t<decltype(&T::Serialize)>> : std::true_type {};

template<typename T, typename = void>
struct is_user_deserializable_t : std::false_type {};
template<typename T>
struct is_user_deserializable_t<T, std::void_t<decltype(&T::Deserialize)>> : std::true_type {};


// 可memcopy
template <class T>
constexpr bool is_memcopyable_v = std::is_trivially_copyable_v<T>;
// 可迭代
template <class T>
constexpr bool is_iterable_v = details::is_iterable_t<T>::value;
// std::unordered_map
template <class T>
constexpr bool is_unordered_map_v = details::is_unordered_map_t<T>::value;
// std::map
template <class T>
constexpr bool is_map_v = details::is_map_t<T>::value;
// std::unordered_set
template <class T>
constexpr bool is_unordered_set_v = details::is_unordered_set_t<T>::value;
// std::set
template <class T>
constexpr bool is_set_v = details::is_set_t<T>::value;
// std::string
template <class T>
constexpr bool is_string_v = details::is_string_t<T>::value;
// std::pair
template <class T>
constexpr bool is_pair_v = details::is_pair_t<T>::value;
// std::vector
template <class T>
constexpr bool is_vector_v = details::is_vector_t<T>::value;
// std::unique_ptr
template <class T>
constexpr bool is_unique_ptr_v = details::is_unique_ptr_t<T>::value;
// 用户自定义类
template <class T>
constexpr bool is_user_serializable_v = details::is_user_serializable_t<T>::value;
template <class T>
constexpr bool is_user_deserializable_v = details::is_user_deserializable_t<T>::value;

template <class>
constexpr bool always_false = false;


} // namespace details


template <class T, class... Types>
void Serialize(std::ostream& os, T& val, Types&... args) {
    using DecayT = std::decay_t<T>;
    if constexpr (details::is_user_serializable_v<T>) {
        val.Serialize(os);
    }
    else if constexpr (std::is_pointer_v<DecayT>) {
        // Serialize(os, *val, args...);
        static_assert(details::always_false<T>, "Serializing raw pointers is not supported!");
    }
    else if constexpr (details::is_unique_ptr_v<DecayT>) {
        Serialize(os, *val, args...);
    }
    else if constexpr (details::is_pair_v<DecayT>) {
        Serialize(os, val.first);
        Serialize(os, val.second);
    }
    else if constexpr (details::is_iterable_v<DecayT>) {
        unsigned int size = val.size();
        os.write((const char*)&size, sizeof(size));
        if constexpr (details::is_memcopyable_v<DecayT>) {
            os.write((const char*)&val, size * sizeof(typename T::value_type));
        }
        else {
            for (auto& v : val) { 
                Serialize(os, v); 
            }
        }
    }
    else if constexpr (details::is_memcopyable_v<T>) {
        os.write((const char*)&val, sizeof(T));
    }
    else {
        //printf("types that cannot be serialized: %s\n", typeid(T).name()); throw;
        static_assert(details::always_false<T>, "types that cannot be serialized.");
    }
    if constexpr (sizeof...(args) > 0) {
        Serialize(os, args...);
    }
}

template <class T>
T Deserialize(std::istream& is) {
    using DecayT = std::decay_t<T>;

    unsigned int size = 0;
    DecayT res{};
    if constexpr (details::is_user_deserializable_v<T>) {
        res.Deserialize(is);
    }
    else if constexpr (std::is_pointer_v<DecayT>) {
        //res = new std::remove_pointer_t<DecayT>{ Deserialize<std::remove_pointer_t<DecayT>>(is) };
        // 不支持原始指针的原因是，需要通过new构造一个对象
        // 但在现代cpp中，原始指针代表的是引用一个在其生命周期内的对象，若使用者未注意就会造成内存泄漏
        static_assert(details::always_false<T>, "Deserializing raw pointers is not supported!");
    }
    else if constexpr (details::is_unique_ptr_v<DecayT>) {
        using DecayRawT = std::decay_t<DecayT::element_type>;
        res = std::make_unique<DecayRawT>(Deserialize<DecayRawT>(is));
    }
    // std::pair
    else if constexpr (details::is_pair_v<DecayT>) {
        auto first = Deserialize<typename T::first_type>(is);        // 反序列化first
        auto second = Deserialize<typename T::second_type>(is);      // 反序列化second
        ::new(&res) T(first, second);                                // 重构std::pair
    }
    // 可迭代的容器类型
    else if constexpr (details::is_iterable_v<DecayT>) {
        // 可直接内存复制的容器类型, 比如std::vector<char>
        if constexpr (details::is_memcopyable_v<DecayT>) {
            is.read((char*)&size, sizeof(unsigned int));
            res.resize(size);
            is.read((char*)res.data(), size * sizeof(typename T::value_type));
        }
        /*
        不可直接内存复制的容器类型
        比如std::vector<std::string>
        std::string不可直接内存复制
        */
        else {
            is.read((char*)&size, sizeof(unsigned int));
            if constexpr (details::is_vector_v<DecayT>) {
                res.resize(size);
            }
            for (unsigned int i = 0; i < size; i++) {
                auto tmp = Deserialize<typename T::value_type>(is);     // 反序列化T的元素, 比如std::string
                // std::basic_string
                if constexpr (details::is_string_v<DecayT>) {
                    res.insert(i, 1, std::move(tmp));
                }
                // std::unordered_map || std::map || std::unordered_set || std::set
                else if constexpr (details::is_unordered_map_v<DecayT> || details::is_map_v<DecayT> || details::is_unordered_set_v<DecayT> || details::is_set_v<DecayT>) {
                    res.insert(std::move(tmp));
                }
                else if constexpr (details::is_vector_v<DecayT>) {
                    res[i] = std::move(tmp);
                }
                else {
                    //printf("unrealized container: %s\n", typeid(DecayT).name()); throw;       // 运行时查看未实现的类型
                    static_assert(details::always_false<T>, "unrealized container.");
                }
            }
        }
    }
    // 可直接内存复制的类型
    else if constexpr (details::is_memcopyable_v<T>) {
        is.read((char*)&res, sizeof(T));
    }
    else {
        //printf("types that cannot be deserialized: %s\n", typeid(T).name()); throw;
        static_assert(details::always_false<T>, "types that cannot be deserialized.");
    }
    return res;
}

template <class T, class... Types>
void Deserialize(std::istream& is, T& val, Types&... args) {
    val = Deserialize<T>(is);
    if constexpr (sizeof...(args) > 0) {
        Deserialize(is, args...);
    }
}

} // namespace sezz


#endif // SEZZ_SEZZ_HPP_