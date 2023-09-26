#ifndef SEZZ_SEZZ_HPP_
#define SEZZ_SEZZ_HPP_


#ifndef SEZZ_STD
#include <iostream>
#include <typeinfo>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <string>
#include <vector>
#include <optional>

#define SEZZ_STD std::
#endif

namespace sezz {

namespace exposer {
template <class MemberType> struct Exposer {
    static MemberType memberPtr;
};
template <class MemberType> MemberType Exposer<MemberType>::memberPtr;

template <class MemberType, MemberType MemberPtr> struct ExposerImpl {
    static struct ExposerFactory {
        ExposerFactory() { Exposer<MemberType>::memberPtr = MemberPtr; }
    } factory;
};
template <class MemberType, MemberType Ptr>
typename ExposerImpl<MemberType, Ptr>::ExposerFactory
    ExposerImpl<MemberType, Ptr>::factory;
} // namespace exposer

//#define SEZZ_ACCESS(ClassName, AttrName, AttrType)                               \
//    template struct exposer::ExposerImpl<decltype(&ClassName::AttrName), &ClassName::AttrName>; \
//    AttrType &get_##AttrName##_from_##ClassName(ClassName &T) {                  \
//        return T.*exposer::Exposer<AttrType ClassName::*>::memberPtr;            \
//    }

namespace internal {
/*
* 对原始指针和其他智能指针的支持构思
* 需要引入运行时哈希表
* 在序列化时如果发现是原始指针/shared_ptr/weak_ptr，则判断在之前是否以及序列化过了(unique_ptr/shared_ptr)，是则序列化一个编号(不是实际值)
* 在反序列化时，如果是原始指针/shared_ptr，则基于编号索引先前反序列化时再次构造的表(在反序列化unique_ptr/shared_ptr是会构建)，使其指向同一份数据
*/


// SEZZ_STD pair
template<typename T>
struct is_pair_t : SEZZ_STD false_type {};
template<typename T1, typename T2>
struct is_pair_t<SEZZ_STD pair<T1, T2>> : SEZZ_STD true_type {};


// SEZZ_STD optional
template<typename T>
struct is_optional_t : SEZZ_STD false_type {};
template<typename T>
struct is_optional_t<SEZZ_STD optional<T>> : SEZZ_STD true_type {};

// SEZZ_STD map
template<typename T>
struct is_map_t : SEZZ_STD false_type {};
template<typename T1, typename T2>
struct is_map_t<SEZZ_STD map<T1, T2>> : SEZZ_STD true_type {};
template<typename T1, typename T2, typename T3>
struct is_map_t<SEZZ_STD map<T1, T2, T3>> : SEZZ_STD true_type {};

// SEZZ_STD unordered_map
template<typename T>
struct is_unordered_map_t : SEZZ_STD false_type {};
template<typename T1, typename T2>
struct is_unordered_map_t<SEZZ_STD unordered_map<T1, T2>> : SEZZ_STD true_type {};

// SEZZ_STD set
template<typename T>
struct is_set_t : SEZZ_STD false_type {};
template<typename T1, typename T2>
struct is_set_t<SEZZ_STD set<T1, T2>> : SEZZ_STD true_type {};

// SEZZ_STD unordered_set
template<typename T>
struct is_unordered_set_t : SEZZ_STD false_type {};
template<typename T1, typename T2>
struct is_unordered_set_t<SEZZ_STD unordered_set<T1, T2>> : SEZZ_STD true_type {};

// SEZZ_STD basic_string
template<typename T>
struct is_string_t : SEZZ_STD false_type {};
template<typename T>
struct is_string_t<SEZZ_STD basic_string<T>> : SEZZ_STD true_type {};

// SEZZ_STD vector
template<typename T>
struct is_vector_t : SEZZ_STD false_type {};
template<typename T>
struct is_vector_t<SEZZ_STD vector<T>> : SEZZ_STD true_type {};

// 可迭代
template<typename T, typename = void>
struct is_iterable_t : SEZZ_STD false_type {};
template<typename T>
struct is_iterable_t<T, SEZZ_STD void_t<decltype(SEZZ_STD begin(SEZZ_STD declval<T>())), decltype(SEZZ_STD end(SEZZ_STD declval<T>()))>> : SEZZ_STD true_type {};

// 智能指针
template<typename T, typename = void>
struct is_unique_ptr_t : SEZZ_STD false_type {};
template<typename T>
struct is_unique_ptr_t<T, SEZZ_STD void_t<decltype(&T::operator->), decltype(&T::operator*)>> : SEZZ_STD true_type {};

// 用户自定义类
template<typename T, typename = void>
struct is_user_class_serializable_t : SEZZ_STD false_type {};
template<typename T>
struct is_user_class_serializable_t<T, SEZZ_STD void_t<decltype(&T::Serialize)>> : SEZZ_STD true_type {};

template<typename T, typename = void>
struct is_user_class_deserializable_t : SEZZ_STD false_type {};
template<typename T>
struct is_user_class_deserializable_t<T, SEZZ_STD void_t<decltype(&T::Deserialize)>> : SEZZ_STD true_type {};


// SEZZ_STD pair
template <class T>
constexpr bool is_pair_v = internal::is_pair_t<T>::value;
// SEZZ_STD optional
template <class T>
constexpr bool is_optional_v = internal::is_optional_t<T>::value;
// 可memcopy
template <class T>
constexpr bool is_memcopyable_v = SEZZ_STD is_trivially_copyable_v<T>;
// 可迭代
template <class T>
constexpr bool is_iterable_v = internal::is_iterable_t<T>::value;
// SEZZ_STD unordered_map
template <class T>
constexpr bool is_unordered_map_v = internal::is_unordered_map_t<T>::value;
// SEZZ_STD map
template <class T>
constexpr bool is_map_v = internal::is_map_t<T>::value;
// SEZZ_STD unordered_set
template <class T>
constexpr bool is_unordered_set_v = internal::is_unordered_set_t<T>::value;
// SEZZ_STD set
template <class T>
constexpr bool is_set_v = internal::is_set_t<T>::value;
// SEZZ_STD string
template <class T>
constexpr bool is_string_v = internal::is_string_t<T>::value;
// SEZZ_STD vector
template <class T>
constexpr bool is_vector_v = internal::is_vector_t<T>::value;
// SEZZ_STD unique_ptr
template <class T>
constexpr bool is_unique_ptr_v = internal::is_unique_ptr_t<T>::value;
// 用户自定义类
template <class T>
constexpr bool is_user_class_serializable_v = internal::is_user_class_serializable_t<T>::value;
template <class T>
constexpr bool is_user_class_deserializable_v = internal::is_user_class_deserializable_t<T>::value;

template <class>
constexpr bool always_false = false;


} // namespace internal


template <class T, class... Types>
void Serialize(SEZZ_STD ostream& os, T& val, Types&... args) {
    using DecayT = SEZZ_STD decay_t<T>;
    if constexpr (internal::is_user_class_serializable_v<T>) {
        val.Serialize(os);
    }
    else if constexpr (internal::is_optional_v<T>) {
        bool has_value = val.has_value();
        Serialize(os, has_value);
        if (has_value) {
            Serialize(os, val.value());
        }
    }
    else if constexpr (SEZZ_STD is_pointer_v<DecayT>) {
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
    //if constexpr (sizeof...(args) > 0) {
    //    Serialize(os, args...);
    //}
    (Serialize(os, args), ...);
}

template <class T>
T Deserialize(SEZZ_STD istream& is) {
    using DecayT = SEZZ_STD decay_t<T>;
    uint32_t size = 0;
    DecayT res{};
    if constexpr (internal::is_user_class_deserializable_v<T>) {
        res.Deserialize(is);
    }
    else if constexpr (internal::is_optional_v<T>) {
        bool has_value = Deserialize<bool>(is);
        if (has_value) {
            ::new(&res) T(Deserialize<typename T::value_type>(is));
        }
    }
    else if constexpr (SEZZ_STD is_pointer_v<DecayT>) {
        //res = new SEZZ_STD remove_pointer_t<DecayT>{ Deserialize<SEZZ_STD remove_pointer_t<DecayT>>(is) };
        // 不支持原始指针的原因是，需要通过new构造一个对象
        // 但在现代cpp中，原始指针代表的是引用一个在其生命周期内的对象，若使用者未注意就会造成内存泄漏
        static_assert(internal::always_false<T>, "Deserializing raw pointers is not supported!");
    }
    else if constexpr (internal::is_unique_ptr_v<DecayT>) {
        using DecayRawT = SEZZ_STD decay_t<DecayT::element_type>;
        res = SEZZ_STD make_unique<DecayRawT>(Deserialize<DecayRawT>(is));
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
        不可直接内存复制的容器类型
        比如vector<string>
        string不可直接内存复制
        */
        else {
            is.read((char*)&size, sizeof(uint32_t));
            if constexpr (internal::is_vector_v<DecayT>) {
                res.resize(size);
            }
            for (uint32_t i = 0; i < size; i++) {
                auto tmp = Deserialize<typename T::value_type>(is);     // 反序列化T的元素, 比如SEZZ_STD string
                // basic_string
                if constexpr (internal::is_string_v<DecayT>) {
                    res.insert(i, 1, SEZZ_STD move(tmp));
                }
                // unordered_map || map || unordered_set || set
                else if constexpr (internal::is_unordered_map_v<DecayT> || internal::is_map_v<DecayT> || internal::is_unordered_set_v<DecayT> || internal::is_set_v<DecayT>) {
                    res.insert(SEZZ_STD move(tmp));
                }
                else if constexpr (internal::is_vector_v<DecayT>) {
                    res[i] = SEZZ_STD move(tmp);
                }
                else {
                    printf("unrealized container: %s\n", typeid(DecayT).name()); throw;       // 运行时查看未实现的类型
                    //static_assert(internal::always_false<T>, "unrealized container.");
                }
            }
        }
    }
    // 可直接内存复制的类型
    else if constexpr (internal::is_memcopyable_v<T>) {
        is.read((char*)&res, sizeof(T));
    }
    else {
        printf("types that cannot be deserialized: %s\n", typeid(T).name()); throw;
        //static_assert(internal::always_false<T>, "types that cannot be deserialized.");
    }
    return res;
}

template <class T, class... Types>
void Deserialize(SEZZ_STD istream& is, T& val, Types&... args) {
    val = Deserialize<T>(is);
    //if constexpr (sizeof...(args) > 0) {
    //    Deserialize(is, args...);
    //}
    (Deserialize(is, args), ...);
}

} // namespace sezz


#endif // SEZZ_SEZZ_HPP_