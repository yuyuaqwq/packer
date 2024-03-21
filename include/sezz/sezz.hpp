#ifndef SEZZ_SEZZ_HPP_
#define SEZZ_SEZZ_HPP_

#include <bit>

#include <sezz/detail/sezz_impl.hpp>
#include <sezz/detail/algorithm.hpp>

namespace sezz {

namespace detail {
class MemoryRuntime;

} // namespace detail


enum class ArchiveMode {
    kCompact,
    kRaw,
};

template <class InputStream = MemoryInputStream, 
    class Version = uint64_t, 
    ArchiveMode kMode = ArchiveMode::kCompact>
class BinaryInputArchive {
public:
    BinaryInputArchive(InputStream& istream) : istream_{ istream }, memory_runtime_{ nullptr }, version_{ 0 } { }

    ~BinaryInputArchive() {
        if (memory_runtime_) {
            delete memory_runtime_;
            memory_runtime_ = nullptr;
        }
    }

    template <class T>
    T Load() {
        T res{};
        // 可直接内存复制的类型
        if constexpr (
            (kMode == ArchiveMode::kRaw && std::is_trivially_copyable_v<T>) ||
            (sizeof(T) == 1))
        {
            istream_.read(reinterpret_cast<char*>(&res), sizeof(T));
            if (istream_.fail()) {
                throw std::runtime_error("input stream read fail.");
            }
        }
        else {
            DeserializeTo(*this, &res);
        }
        return res;
    }

    template <class... Types>
    void Load(Types&&... bufs) {
        ((bufs = Load<Types>()), ...);
    }

    InputStream& istream() {
        return istream_;
    }

    detail::MemoryRuntime*& memory_runtime() {
        return memory_runtime_;
    }

    void ClearMemoryRuntimeContext() {
        if (memory_runtime_) {
            delete memory_runtime_;
            memory_runtime_ = nullptr;
        }
    }

    void LoadVersion() {
        version_ = Load<Version>();
    }

    const Version& version() const noexcept {
        return version_;
    }

private:
    InputStream& istream_;
    detail::MemoryRuntime* memory_runtime_;
    Version version_;
};

template <class OutputStream = MemoryOutputStream,
    class Version = uint64_t,
    ArchiveMode kMode = ArchiveMode::kCompact>
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
    void Save(const T& val) {
        if constexpr (
            (kMode == ArchiveMode::kRaw && std::is_trivially_copyable_v<T>) ||
            (sizeof(T) == 1))
        {
            ostream_.write(reinterpret_cast<char*>(&val), sizeof(T));
        }
        else {
            SerializeTo(*this, val);
        }
    }

    template <class... Types>
    void Save(const Types&... vals) {
        (Save(vals), ...);
    }

    OutputStream& ostream() {
        return ostream_;
    }

    detail::MemoryRuntime*& memory_runtime() {
        return memory_runtime_;
    }

    void ClearMemoryRuntimeContext() {
        if (memory_runtime_) {
            delete memory_runtime_;
            memory_runtime_ = nullptr;
        }
    }

    void SaveVersion(Version&& version) {
        version_ = version;
        Save(version_);
    }

    const Version& version() const noexcept {
        return version_;
    }

private:
    OutputStream& ostream_;
    detail::MemoryRuntime* memory_runtime_;
    Version version_;
};


template <std::integral T>
struct Serializer<T> {
    template<typename OutputArchive>
    constexpr void Serialize(OutputArchive& ar, const T& val) const {
        if constexpr (std::is_signed_v<T>) {
            uint8_t buf[10];
            size_t len = detail::ZigzagEncoded(val, buf) - buf;
            ar.ostream().write(reinterpret_cast<char*>(buf), len);
        }
        else {
            uint8_t buf[10];
            size_t len = detail::VarintEncoded(val, buf) - buf;
            ar.ostream().write(reinterpret_cast<char*>(buf), len);
        }
    }

    template<typename InputArchive>
    constexpr void Deserialize(InputArchive& ar, T* out) const {
        int64_t tmp{};
        if constexpr (std::is_signed_v<T>) {
            detail::ZigzagDecode(&tmp, &ar.istream());
        }
        else {
            detail::VarintDecode(&tmp, &ar.istream());
        }
        *out = static_cast<T>(tmp);
    }
};

template <std::floating_point T>
struct Serializer<T> {
    template<typename OutputArchive>
    constexpr void Serialize(OutputArchive& ar, const T& val) const {
        auto tmp = val;

        if constexpr (std::endian::native == std::endian::big) {
            // Small endings are more common, so we will convert large endings to small endings
            tmp = detail::RevereseByte(tmp);
        }
        else if constexpr (std::endian::native != std::endian::little) {
            static_assert(always_false<T>, "Unsupported byte order!");
        }
        ar.ostream().write(reinterpret_cast<char*>(&tmp), sizeof(T));
    }

    template<typename InputArchive>
    constexpr void Deserialize(InputArchive& ar, T* out) const {
        ar.istream().read(reinterpret_cast<char*>(out), sizeof(T));
        if (ar.istream().fail()) {
            throw std::runtime_error("input stream read fail.");
        }
        if constexpr (std::endian::native == std::endian::big) {
            // Small endings are more common, so we will convert large endings to small endings
            out = detail::RevereseByte(out);
        }
        else if constexpr (std::endian::native != std::endian::little) {
            static_assert(always_false<T>, "Unsupported byte order!");
        }
    }
};
} // namespace sezz


#endif // SEZZ_SEZZ_HPP_