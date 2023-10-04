#ifndef SEZZ_VARINT_HPP_
#define SEZZ_VARINT_HPP_

#include <stdint.h>

namespace sezz {
namespace varint {

uint8_t* VarintEncoded(int64_t val, uint8_t* buf) {
    while (val >= 0x80) {
        *buf++ = static_cast<uint8_t>(val) | 0x80;
        val >>= 7;
    }
    *buf++ = static_cast<uint8_t>(val);
    return buf;
}

uint8_t* VarintDecode(int64_t* val_ptr, uint8_t* buf) {
    uint8_t offset = 0;
    uint64_t result = 0;
    do {
        result |= static_cast<int64_t>((*buf) & ~0x80) << offset;
        offset += 7;
    } while (((*buf++) & 0x80) != 0);
    *val_ptr = result;
    return buf;
}

template <typename IoStream> 
size_t VarintDecode(int64_t* val_ptr, IoStream* stream) {
    uint8_t offset = 0;
    uint64_t result = 0;
    uint8_t byte;
    size_t len = 0;
    do {
        stream->read((char*)&byte, 1);
        result |= static_cast<int64_t>((byte) & ~0x80) << offset;
        offset += 7;
        ++len;
    } while ((byte & 0x80) != 0);
    *val_ptr = result;
    return len;
}

uint8_t* ZigzagEncoded(int64_t val, uint8_t* buf) {
    return VarintEncoded(static_cast<int64_t>((val << 1) ^ (val >> 63)), buf);
}

uint8_t* ZigzagDecode(int64_t* val_ptr, uint8_t* buf) {
    uint64_t val;
    buf = VarintDecode(reinterpret_cast<int64_t*>(&val), buf);
    *val_ptr = static_cast<int64_t>((val >> 1) ^ -static_cast<int64_t>(val & 1));
    return buf;
}

template <typename IoStream>
size_t ZigzagDecode(int64_t* val_ptr, IoStream* stream) {
    uint64_t val;
    auto size = VarintDecode(reinterpret_cast<int64_t*>(&val), stream);
    *val_ptr = static_cast<int64_t>((val >> 1) ^ -static_cast<int64_t>(val & 1));
    return size;
}

} // namespace varint
} // namespace sezz

#endif // SEZZ_VARINT_HPP_