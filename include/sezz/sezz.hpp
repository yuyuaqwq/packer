#ifndef SEZZ_SEZZ_HPP_
#define SEZZ_SEZZ_HPP_

#include <sezz/type_traits.hpp>
#include <sezz/algorithm.hpp>

namespace sezz {

namespace detail {

template<typename Archive, typename T>
concept serialize_accept = requires(Archive ar, T t) { t.Serialize(ar); };
template<typename Archive, typename T>
concept deserialize_accept = requires(Archive ar, T t) { t.Deserialize(ar); };

template <class>
constexpr bool always_false = false;

class MemoryRuntime;

} // namespace detail


template <class IoStream>
class BinaryArchive {
public:
    BinaryArchive(IoStream& io_stream) : io_stream_{ io_stream }, memory_runtime_{ nullptr } { }

    ~BinaryArchive() {
        if (memory_runtime_) {
            delete memory_runtime_;
            memory_runtime_ = nullptr;
        }
    }

    template <class T>
    void Save(T&& val) {
        using DecayT = std::decay_t<T>;
        if constexpr (detail::serialize_accept<BinaryArchive, DecayT>) {
            val.Serialize(*this);
        }
        else if constexpr (sizeof(val) == 1) {
            io_stream_.write(reinterpret_cast<char*>(&val), sizeof(val));
        }
        else if constexpr (std::is_integral_v<DecayT>) {
            if constexpr (std::is_signed_v<DecayT>) {
                uint8_t buf[10];
                size_t len = algorithm::ZigzagEncoded(val, buf) - buf;
                io_stream_.write(reinterpret_cast<char*>(buf), len);
            }
            else {
                uint8_t buf[10];
                size_t len = algorithm::VarintEncoded(val, buf) - buf;
                io_stream_.write(reinterpret_cast<char*>(buf), len);
            }
        }
        //else if constexpr (std::is_floating_point_v<DecayT>) {
        //    static_assert(detail::always_false<T>, "This type of floating-point number cannot be serialized!");
        //}
        //else if constexpr (std::is_trivially_copyable_v<DecayT>) {
        //    io_stream_.write(reinterpret_cast<char*>(&val), sizeof(DecayT));
        //}
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
            DecayT res{};
            res.Deserialize(*this);
            return res;
        }
        else if constexpr (sizeof(DecayT) == 1) {
            DecayT res{};
            io_stream_.read(reinterpret_cast<char*>(&res), sizeof(DecayT));
            return res;
        }
        else if constexpr (std::is_integral_v<DecayT>) {
            int64_t res;
            if constexpr (std::is_signed_v<DecayT>) {
                size_t len = algorithm::ZigzagDecode(&res, &io_stream_);
                return res;
            }
            else {
                size_t len = algorithm::VarintDecode(&res, &io_stream_);
                return static_cast<uint64_t>(res);
            }
        }
        //else if constexpr (std::is_floating_point_v<DecayT>) {
        //    static_assert(detail::always_false<T>, "This type of floating-point number cannot be serialized!");
        //}
        // 可直接内存复制的类型
        //else if constexpr (std::is_trivially_copyable_v<DecayT>) {
        //    DecayT res{};
        //    io_stream_.read((char*)&res, sizeof(DecayT));
        //    return res;
        //}
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

    detail::MemoryRuntime*& GetMemoryRuntime() {
        return memory_runtime_;
    }

    void ClearMemoryRuntimeContext() {
        if (memory_runtime_) {
            delete memory_runtime_;
            memory_runtime_ = nullptr;
        }
    }

private:
    IoStream& io_stream_;
    detail::MemoryRuntime* memory_runtime_;
};

} // namespace sezz


#endif // SEZZ_SEZZ_HPP_