#ifndef SEZZ_SEZZ_HPP_
#define SEZZ_SEZZ_HPP_

#include <vector>
#include <bit>

#include <sezz/type_traits.hpp>
#include <sezz/algorithm.hpp>

namespace sezz {

namespace detail {

template<typename Archive, typename T>
concept serialize_accept = requires(Archive ar, T t) { t.Serialize(ar); };
template<typename Archive, typename T>
concept deserialize_accept = requires(Archive ar, T t) { t.Deserialize(ar); };

class MemoryRuntime;

} // namespace detail


class MemoryIoStream {
public:
    MemoryIoStream(size_t size) : buf_(size){
        pos_ = 0;
    }

    void write(const char* buf, size_t size) {
        if (pos_ + size > buf_.size()) {
            buf_.resize((buf_.size() + size) * 2);
        }
        auto data = &buf_[pos_];
        memcpy(data, buf, size);
        pos_ += size;
    }

    void read(char* buf, size_t size) {
        if (pos_ + size > buf_.size()) {
            throw std::runtime_error("Stream data has reached the end.");
        }
        auto data = &buf_[pos_];
        memcpy(buf, data, size);
        pos_ += size;
    }

    size_t tellp() {
        return pos_;
    }

    void seekp(size_t pos) {
        pos_ = pos;
    }
    
private:
    std::vector<uint8_t> buf_;
    size_t pos_;
};

template <class IoStream = MemoryIoStream>
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
                size_t len = detail::ZigzagEncoded(val, buf) - buf;
                io_stream_.write(reinterpret_cast<char*>(buf), len);
            }
            else {
                uint8_t buf[10];
                size_t len = detail::VarintEncoded(val, buf) - buf;
                io_stream_.write(reinterpret_cast<char*>(buf), len);
            }
        }
        else if constexpr (std::is_floating_point_v<DecayT>) {
            DecayT temp = val;
            
            if constexpr (std::endian::native == std::endian::little) {
                temp = detail::RevereseByte(temp);
            }
            else if constexpr (std::endian::native != std::endian::big) {
                static_assert(detail::always_false<T>, "Unsupported byte order!");
            }
            io_stream_.write(reinterpret_cast<char*>(&temp), sizeof(DecayT));
            //static_assert(detail::always_false<T>, "This type of floating-point number cannot be serialized!");
        }
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
                size_t len = detail::ZigzagDecode(&res, &io_stream_);
                return res;
            }
            else {
                size_t len = detail::VarintDecode(&res, &io_stream_);
                return static_cast<uint64_t>(res);
            }
        }
        else if constexpr (std::is_floating_point_v<DecayT>) {
            DecayT res;
            io_stream_.read(reinterpret_cast<char*>(&res), sizeof(DecayT));
            if constexpr (std::endian::native == std::endian::little) {
                res = detail::RevereseByte(res);
            }
            else if constexpr (std::endian::native != std::endian::big) {
                static_assert(detail::always_false<T>, "Unsupported byte order!");
            }
            return res;
            //static_assert(detail::always_false<T>, "This type of floating-point number cannot be deserialized!");
        }
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