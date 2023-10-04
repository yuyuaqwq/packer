#ifndef SEZZ_SEZZ_HPP_
#define SEZZ_SEZZ_HPP_

#include <sezz/type_traits.hpp>
#include <sezz/varint.hpp>

namespace sezz {

namespace detail {

template<typename Archive, typename T>
concept serialize_accept = requires(Archive ar, T t) { t.Serialize(ar); };
template<typename Archive, typename T>
concept deserialize_accept = requires(Archive ar, T t) { t.Deserialize(ar); };

template <class>
constexpr bool always_false = false;

} // namespace detail


template <class IoStream>
class BinaryArchive {
public:
    BinaryArchive(IoStream& io_stream) : io_stream_{ io_stream } { }

    template <class T>
    void Save(T&& val) {
        using DecayT = std::decay_t<T>;
        if constexpr (detail::serialize_accept<BinaryArchive, DecayT>) {
            val.Serialize(*this);
        }
        else if constexpr (std::is_pointer_v<DecayT>) {
            static_assert(detail::always_false<T>, "Serializing raw pointers is not supported!");
        }
        else if constexpr (
            std::is_same_v<int16_t, DecayT> ||
            std::is_same_v<int32_t, DecayT> ||
            std::is_same_v<int64_t, DecayT>
            ) {
            uint8_t buf[10];
            size_t len = varint::ZigzagEncoded(val, buf) - buf;
            io_stream_.write(reinterpret_cast<char*>(buf), len);
        }
        else if constexpr (
            std::is_same_v<uint16_t, DecayT> ||
            std::is_same_v<uint32_t, DecayT> ||
            std::is_same_v<uint64_t, DecayT>
            ) {
            uint8_t buf[10];
            size_t len = varint::VarintEncoded(val, buf) - buf;
            io_stream_.write(reinterpret_cast<char*>(buf), len);
        }
        else if constexpr (std::is_trivially_copyable_v<DecayT>) {
            io_stream_.write(reinterpret_cast<char*>(&val), sizeof(DecayT));
        }
        else {
            Serialize(*this, std::forward<T>(val));
            //printf("types that cannot be serialized: %s\n", typeid(T).name()); throw;
            //static_assert(detail::always_false<T>, "types that cannot be serialized.");
        }
    }

    template <class... Types>
    void Save(Types&&... vals) {
        (Save(std::forward<Types>(vals)), ...);
    }

    template <class T, class DecayT = std::decay_t<T>>
    T Load() {
        if constexpr (detail::deserialize_accept<BinaryArchive, DecayT>) {
            T res{};
            res.Deserialize(*this);
            return res;
        }
        else if constexpr (std::is_pointer_v<DecayT>) {
            // res = new std::remove_pointer_t<DecayT>{ Deserialize<std::remove_pointer_t<DecayT>>(is) };
            // 不支持原始指针的原因是，需要通过new构造一个对象
            // 但在现代cpp中，原始指针代表的是引用一个在其生命周期内的对象，若使用者未注意就会造成内存泄漏
            static_assert(detail::always_false<T>, "Deserializing raw pointers is not supported!");
        }
        else if constexpr (
            std::is_same_v<int16_t, DecayT> ||
            std::is_same_v<int32_t, DecayT> ||
            std::is_same_v<int64_t, DecayT>
            ) {
            int64_t res;
            size_t len = varint::ZigzagDecode(&res, &io_stream_);
            return res;
        }
        else if constexpr (
            std::is_same_v<uint16_t, DecayT> ||
            std::is_same_v<uint32_t, DecayT> ||
            std::is_same_v<uint64_t, DecayT>
            ) {
            int64_t res;
            size_t len = varint::VarintDecode(&res, &io_stream_);
            return (uint64_t)res;
        }
        // 可直接内存复制的类型
        else if constexpr (std::is_trivially_copyable_v<DecayT>) {
            DecayT res{};
            io_stream_.read((char*)&res, sizeof(DecayT));
            return res;
        }
        else {
            return Deserialize<T>(*this);
            //printf("types that cannot be deserialized: %s\n", typeid(T).name()); throw;
            //static_assert(detail::always_false<T>, "types that cannot be deserialized.");
        }
    }

    template <class T, class... Types, class DecayT = std::decay_t<T>>
    void Load(T& buf, Types&... bufs) {
        buf = Load<DecayT>();
        (Load(bufs), ...);
    }

    IoStream& GetIoStream() {
        return io_stream_;
    }

private:
    IoStream& io_stream_;
};

} // namespace sezz


#endif // SEZZ_SEZZ_HPP_