#ifndef SEZZ_VARINT_HPP_
#define SEZZ_VARINT_HPP_

/*
https://www.eet-china.com/mp/a202331.html
*/

#include <stdint.h>

namespace sezz {
namespace varint {

uint8_t* VarintEncoded(int64_t val, uint8_t* buf) {
    while (val >= 0x80) {
        *buf++ = (uint8_t)val | 0x80;
        val >>= 7;
    }
    *buf++ = (uint8_t)val;
    return buf;
}

uint8_t* VarintDecode(int64_t* val_ptr, uint8_t* buf) {
    uint8_t offset = 0;
    uint64_t result = 0;
    do {
        result |= (int64_t)((*buf) & ~0x80) << offset;
        offset += 7;
    } while (((*buf++) & 0x80) != 0);
    *val_ptr = result;
    return buf;
}

uint8_t* ZigzagEncoded(int64_t val, uint8_t* buf) {
    return VarintEncoded((int64_t)((val << 1) ^ (val >> 63)), buf);
}

uint8_t* ZigzagDecode(int64_t* val_ptr, uint8_t* buf) {
    uint64_t val;
    buf = VarintDecode((int64_t*)&val, buf);
    *val_ptr = (int64_t)((val >> 1) ^ -(int64_t)(val & 1));
    return buf;
}

} // namespace varint
} // namespace sezz

#endif // SEZZ_VARINT_HPP_