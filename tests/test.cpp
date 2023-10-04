#include <sezz/stl/string.hpp>
#include <sezz/sezz.hpp>
#include <sezz/varint.hpp>

#include <sezz/stl/map.hpp>
#include <sezz/stl/vector.hpp>


#include <thread>
#include <iostream>
#include <fstream>

#include <vector>
class Invasive {
public:
    Invasive() :str_() {        // Default constructor required
        int_ = 0;
    }

    Invasive(std::string_view str, int _int, std::set<int> set) :str_(str), int_(_int), set_{ set } {

    }

    ~Invasive() {

    }

    template <class Archive>
    void Serialize(Archive& os) {
        sezz::Serialize(os, set_, str_, int_);
    }

    template <class Archive>
    void Deserialize(Archive& is) {
        sezz::Deserialize(is, str_, int_);
    }

private:
    std::string str_;
    int int_;
    std::set<int> set_;
};




class NonIntrusive {
public:     // Non intrusive, requiring external access to data members
    std::string str;
    int aaa;
};


namespace sezz {
// specialization of function templates
template <class Archive>
void Serialize/*<std::fstream, NonIntrusive>*/(Archive& os, NonIntrusive& val) {
    Serialize(os, val.str, val.aaa);
}

// example of overloaded return values
template <class T, class Archive>
    requires std::is_same_v<T, NonIntrusive>
T Deserialize(Archive& is) {
    NonIntrusive val;
    Deserialize(is, val.str, val.aaa);
    return val;
}

// example of parameter overloading
template <class Archive>
void Deserialize(Archive& is, NonIntrusive& val) {
    Deserialize(is, val.str, val.aaa);
}

}


int main() {
    uint8_t buf[10] = { 0 };
    sezz::varint::ZigzagEncoded(5000, buf);
    int64_t val;
    sezz::varint::ZigzagDecode(&val, buf);

    std::optional<int> aa;
    std::optional<int> v;
    aa = v;

    std::fstream fs;

    //sezz::internal::is_user_class_serializable_v<std::fstream, Invasive>;

    fs.open("test.bin", std::ios::binary | std::ios::out | std::ios::in | std::ios::trunc);
    
    //std::string test_str = "abc";
    //sezz::Serialize(fs, test_str);

    //std::map<std::string, std::string>

    std::map<std::string, std::string> test_map {
        { "pair_key_1", "pair_value_1" },
        { "pair_key_2", "pair_value_2" }
    };
    sezz::Serialize(fs, test_map);
    
    

    std::vector<std::string> test_vector{ 
        "vector_1", 
        "vector_2", 
        "vector_3"
    };
    sezz::Serialize(fs, test_vector);

    std::vector<std::vector<std::string>> test_vector2 {
        { "vector_1_1", "vector_2_2" },
        { "vector_2_1", "vector_2_2" },
        { "vector_3_1", "vector_3_2" },
    };
    sezz::Serialize(fs, test_vector2);

    Invasive test_invasive{ "str1", 2, {1,2,3} };
    sezz::Serialize(fs, test_invasive);

    NonIntrusive test_non_intrusive{ "str1", 2 };
    sezz::Serialize(fs, test_non_intrusive);

    fs.seekg(0);

    // match based on return value or parameters

    auto test_map_de = sezz::Deserialize<std::map<std::string, std::string>>(fs);

    //std::unordered_map<std::string, std::string> test_map_de;
    //sezz::Deserialize(fs, test_map_de);

    auto test_vector_de = sezz::Deserialize<std::vector<std::string>>(fs);

    auto test_vector2_de = sezz::Deserialize<std::vector<std::vector<std::string>>>(fs);

    auto test_invasive_de = sezz::Deserialize<Invasive>(fs);

    auto test_non_intrusive_de = sezz::Deserialize<NonIntrusive>(fs);

    // NonIntrusive test_non_intrusive_de;
    // sezz::Deserialize(fs, test_non_intrusive_de);

    std::cout << "ok\n";
}
