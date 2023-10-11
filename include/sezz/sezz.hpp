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

template<typename T>
concept statistical_size_accept = requires(T t) { t.StatisticalSize(); };

class MemoryRuntime;

} // namespace detail


class MemoryInputStream {
public:
    MemoryInputStream(uint8_t* buf, size_t size) : buf_{ buf }, size_{ size }{
        pos_ = 0;
        fail_ = false;
    }

    void read(char* buf, size_t size) {
        if (pos_ + size > size_) {
            fail_ = true;
            return;
        }
        memcpy(buf, &buf_[pos_], size);
        pos_ += size;
    }

    size_t tellg() {
        return pos_;
    }

    void seekg(size_t pos) {
        pos_ = pos;
    }
    
    uint8_t* data() {
        return buf_;
    }

    bool fail() {
        return fail_;
    }

private:
    uint8_t* buf_;
    size_t size_;
    size_t pos_;
    bool fail_;
};

class MemoryOutputStream {
public:
    MemoryOutputStream(size_t size) : buf_(size) {
        pos_ = 0;
        fail_ = false;
    }

    void write(const char* buf, size_t size) {
        size_t cur_size = buf_.size();
        while (pos_ + size > cur_size) {
            cur_size *= 2;
        }
        if (cur_size != buf_.size()) {
            buf_.resize(cur_size);
        }
        memcpy(&buf_[pos_], buf, size);
        pos_ += size;
    }

    size_t tellp() {
        return pos_;
    }

    void seekp(size_t pos) {
        pos_ = pos;
    }

    uint8_t* data() {
        return buf_.data();
    }

    bool fail() {
        return fail_;
    }

private:
    std::vector<uint8_t> buf_;
    size_t pos_;
    bool fail_;
};


enum class ArchiveMode {
    kCompact,
    kRaw,
};

template <class InputStream = MemoryInputStream, ArchiveMode mode = ArchiveMode::kCompact>
class BinaryInputArchive {
public:
    BinaryInputArchive(InputStream& istream) : istream_{ istream }, memory_runtime_{ nullptr }, version_{ 0 } { }

    ~BinaryInputArchive() {
        if (memory_runtime_) {
            delete memory_runtime_;
            memory_runtime_ = nullptr;
        }
    }

    template <class T, class DecayT = std::decay_t<T>>
    T Load() {
        if constexpr (detail::deserialize_accept<BinaryInputArchive, DecayT>) {
            DecayT res{};
            res.Deserialize(*this);
            return res;
        }
        // 可直接内存复制的类型
        else if constexpr (mode == ArchiveMode::kRaw && std::is_trivially_copyable_v<DecayT>) {
            DecayT res{};
            istream_.read((char*)&res, sizeof(DecayT));
            if (istream_.fail()) {
                throw std::runtime_error("input stream read fail.");
            }
            return res;
        }
        else if constexpr (sizeof(DecayT) == 1) {
            DecayT res{};
            istream_.read(reinterpret_cast<char*>(&res), sizeof(DecayT));
            if (istream_.fail()) {
                throw std::runtime_error("input stream read fail.");
            }
            return res;
        }
        else if constexpr (std::is_integral_v<DecayT>) {
            int64_t res;
            if constexpr (std::is_signed_v<DecayT>) {
                size_t len = detail::ZigzagDecode(&res, &istream_);
                return res;
            }
            else {
                size_t len = detail::VarintDecode(&res, &istream_);
                return static_cast<uint64_t>(res);
            }
        }
        else if constexpr (std::is_floating_point_v<DecayT>) {
            DecayT res;
            if (istream_.fail()) {
                throw std::runtime_error("input stream read fail.");
            }
            istream_.read(reinterpret_cast<char*>(&res), sizeof(DecayT));
            if constexpr (std::endian::native == std::endian::big) {
                // Small endings are more common, so we will convert large endings to small endings
                res = detail::RevereseByte(res);
            }
            else if constexpr (std::endian::native != std::endian::little) {
                static_assert(detail::always_false<T>, "Unsupported byte order!");
            }
            return res;
            //static_assert(detail::always_false<T>, "This type of floating-point number cannot be deserialized!");
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

    InputStream& GetInputStream() {
        return istream_;
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

    void LoadVersion() {
        version_ = Load<uint64_t>();
    }

    uint64_t GetVersion() {
        return version_;
    }

private:
    InputStream& istream_;
    detail::MemoryRuntime* memory_runtime_;
    uint64_t version_;
};

template <class OutputStream = MemoryOutputStream, ArchiveMode mode = ArchiveMode::kCompact>
class BinaryOutputArchive {
public:
    BinaryOutputArchive(OutputStream& ostream) : ostream_{ ostream }, memory_runtime_{ nullptr }, version_{ 0 } {  }

    ~BinaryOutputArchive() {
        if (memory_runtime_) {
            delete memory_runtime_;
            memory_runtime_ = nullptr;
        }
    }



    template <class T>
    void Save(T&& val) {
        using DecayT = std::decay_t<T>;
        if constexpr (detail::serialize_accept<BinaryOutputArchive, DecayT>) {
            val.Serialize(*this);
        }
        else if constexpr (mode == ArchiveMode::kRaw && std::is_trivially_copyable_v<DecayT>) {
            ostream_.write(reinterpret_cast<char*>(&val), sizeof(DecayT));
        }
        else if constexpr (sizeof(val) == 1) {
            ostream_.write(reinterpret_cast<char*>(&val), sizeof(val));
        }
        else if constexpr (std::is_integral_v<DecayT>) {
            if constexpr (std::is_signed_v<DecayT>) {
                uint8_t buf[10];
                size_t len = detail::ZigzagEncoded(val, buf) - buf;
                ostream_.write(reinterpret_cast<char*>(buf), len);
            }
            else {
                uint8_t buf[10];
                size_t len = detail::VarintEncoded(val, buf) - buf;
                ostream_.write(reinterpret_cast<char*>(buf), len);
            }
        }
        else if constexpr (std::is_floating_point_v<DecayT>) {
            DecayT temp = val;

            if constexpr (std::endian::native == std::endian::big) {
                // Small endings are more common, so we will convert large endings to small endings
                temp = detail::RevereseByte(temp);
            }
            else if constexpr (std::endian::native != std::endian::little) {
                static_assert(detail::always_false<T>, "Unsupported byte order!");
            }
            ostream_.write(reinterpret_cast<char*>(&temp), sizeof(DecayT));
            //static_assert(detail::always_false<T>, "This type of floating-point number cannot be serialized!");
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

    OutputStream& GetOutputStream() {
        return ostream_;
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

    void SaveVersion(uint64_t version) {
        version_ = version;
        Save(version_);
    }

    uint64_t GetVersion() {
        return version_;
    }

private:
    OutputStream& ostream_;
    detail::MemoryRuntime* memory_runtime_;
    uint64_t version_;
};


} // namespace sezz


#endif // SEZZ_SEZZ_HPP_