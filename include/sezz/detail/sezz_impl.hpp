#pragma once
#include <concepts>
#include <vector>

namespace sezz {
template <class>
constexpr bool always_false = false;

template <class T>
struct Serializer {
	static_assert(always_false<T>, "You haven't specialized the \"Serializer\" for this type T yet!");
	template<typename OutputArchive>
	constexpr void Serialize(OutputArchive& ar, const T& val) const {}
	template<typename InputArchive>
	constexpr void Deserialize(InputArchive& ar, T* out) const {}
};

template <class OutputArchive, class T>
void SerializeTo(OutputArchive& ar, const T& val) {
	Serializer<T>{}.Serialize(ar, val);
}

template <class InputArchive, class T>
void DeserializeTo(InputArchive& ar, T* val) {
	Serializer<T>{}.Deserialize(ar, val);
}


class MemoryIoBase {
public:
    MemoryIoBase() {
        fail_ = false;
    }

    bool fail() {
        return fail_;
    }
protected:
    bool fail_;
};

class MemoryInputStream : virtual public MemoryIoBase {
public:
    MemoryInputStream(std::vector<uint8_t>* buf) : in_buf_{ buf } {
        pos_ = 0;
    }

    void read(char* buf, size_t size) {
        if (pos_ + size > in_buf_->size()) {
            fail_ = true;
            return;
        }
        memcpy(buf, &(*in_buf_)[pos_], size);
        pos_ += size;
    }

    size_t tellg() {
        return pos_;
    }

    void seekg(size_t pos) {
        pos_ = pos;
    }

private:
    std::vector<uint8_t>* in_buf_;
    size_t pos_;
};

class MemoryOutputStream : virtual public MemoryIoBase {
public:
    MemoryOutputStream(size_t size) : out_buf_(size) {
        pos_ = 0;
    }

    void write(const char* buf, size_t size) {
        size_t cur_size = out_buf_.size();
        while (pos_ + size > cur_size) {
            cur_size *= 2;
        }
        if (cur_size != out_buf_.size()) {
            out_buf_.resize(cur_size);
        }
        memcpy(&out_buf_[pos_], buf, size);
        pos_ += size;
    }

    size_t tellp() {
        return pos_;
    }

    void seekp(size_t pos) {
        pos_ = pos;
    }

    uint8_t* data() {
        return out_buf_.data();
    }

protected:
    std::vector<uint8_t> out_buf_;
    size_t pos_;
};

class MemoryIoStream : public MemoryInputStream, public MemoryOutputStream {
public:
    MemoryIoStream(size_t size) : MemoryOutputStream{ size }, MemoryInputStream{ &this->out_buf_ } {

    }
};
}