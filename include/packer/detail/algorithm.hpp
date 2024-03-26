#pragma once
#include <cstdint>

namespace packer {
namespace detail {
template <typename OutIt>
constexpr OutIt VarintEncoded(int64_t val, OutIt it) {
    while (val >= 0x80) {
        *it++ = static_cast<uint8_t>(val) | 0x80;
        val >>= 7;
    }
    *it++ = static_cast<uint8_t>(val);
    return it;
}

template <typename InIt>
InIt VarintDecode(int64_t* res, InIt it) {
    uint8_t offset = 0;
    uint64_t tmp = 0;
    uint8_t byte;
    do {
        byte = static_cast<uint8_t>(*it++);
        tmp |= static_cast<int64_t>(byte & ~0x80) << offset;
        offset += 7;
    } while ((byte & 0x80) != 0);
    *res = tmp;
    return it;
}

template <typename OutIt>
OutIt ZigzagEncoded(int64_t val, OutIt it) {
    return VarintEncoded(static_cast<int64_t>((val << 1) ^ (val >> 63)), it);
}

template <typename InIt>
InIt ZigzagDecode(int64_t* res, InIt it) {
    uint64_t val;
    it = VarintDecode(reinterpret_cast<int64_t*>(&val), it);
    *res = static_cast<int64_t>((val >> 1) ^ -static_cast<int64_t>(val & 1));
    return it;
}

constexpr uint32_t Reverse4Byte(uint32_t val) {
    return (val << 24) | ((val & 0xff00) << 8) | ((val & 0xff0000) >> 8) | (val >> 24);
}

constexpr uint64_t Reverse8Byte(uint64_t val) {
    return (val << 56) | ((val & 0xff00) << 40) | ((val & 0xff0000) << 24) | ((val & 0xff000000) << 8) | ((val & 0xff00000000) >> 8) | ((val & 0xff0000000000) >> 24) | ((val & 0xff000000000000) >> 40) | (val >> 56);
}

template <typename T>
constexpr T RevereseByte(T val) {
    static_assert(sizeof(T) <= 8);

    if constexpr (sizeof(T) == 4) {
        uint32_t res = Reverse4Byte(*reinterpret_cast<uint32_t*>(&val));
        return *reinterpret_cast<T*>(&res);
    }
    else if constexpr (sizeof(T) == 8) {
        uint64_t res = Reverse8Byte(*reinterpret_cast<uint64_t*>(&val));
        return *reinterpret_cast<T*>(&res);
    }
}
}	// namespace detail
}	// namespace packer