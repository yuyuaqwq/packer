

#include <sezz/sezz.hpp>
#include <sezz/varint.hpp>

#include <sezz/stl/set.hpp>
#include <sezz/stl/map.hpp>
#include <sezz/stl/vector.hpp>
#include <sezz/stl/string.hpp>

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
    void Serialize(Archive& ar) {
        ar.Save(set_, str_, int_);
    }

    template <class Archive>
    void Deserialize(Archive& ar) {
        ar.Load(set_, str_, int_);
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

template <class Archive, class T>
    requires std::is_same_v<std::decay_t<T>, NonIntrusive>
void Serialize(Archive& ar, T& val) {
    //printf("%s %d", val.str, val.aaa);
    ar.Save(val.str, val.aaa);
    auto& a = ar.GetIoStream();
}


// example of overloaded return values
template <class T, class Archive>
    requires std::is_same_v<T, NonIntrusive>
T Deserialize(Archive& ar) {
    NonIntrusive val;
    ar.Load(val.str, val.aaa);
    return val;
}

// example of parameter overloading
template <class Archive>
void Deserialize(Archive& ar, NonIntrusive& val) {
    ar.Load(val.str, val.aaa);
}

} // namespace sezz

int main() {

    uint8_t buf[10] = { 0 };
    sezz::varint::ZigzagEncoded(5000, buf);
    int64_t val;
    sezz::varint::ZigzagDecode(&val, buf);

    std::fstream fs;

    fs.open("test.bin", std::ios::binary | std::ios::out | std::ios::in | std::ios::trunc);
    
    sezz::BinaryArchive<std::iostream> ar(fs);

    std::string test_str = "abc";
    ar.Save(test_str);

    std::map<std::string, std::string> test_map {
        { "pair_key_1", "pair_value_1" },
        { "pair_key_2", "pair_value_2" }
    };
    ar.Save(test_map);

    
    std::vector<std::string> test_vector{ 
        "vector_1", 
        "vector_2", 
        "vector_3"
    };
    ar.Save(test_vector);

    std::vector<std::vector<std::string>> test_vector2 {
        { "vector_1_1", "vector_2_2" },
        { "vector_2_1", "vector_2_2" },
        { "vector_3_1", "vector_3_2" },
    };
    ar.Save(test_vector2);

    Invasive test_invasive{ "str1", 2, {1,2,3} };
    ar.Save(test_invasive);

    NonIntrusive test_non_intrusive{ "str1", 2};
    ar.Save(test_non_intrusive);

    fs.seekg(0);

    
    auto test_str_de = ar.Load<std::string>();
    // match based on return value or parameters
    auto test_map_de = ar.Load<std::map<std::string, std::string>>();

    auto test_vector_de = ar.Load<std::vector<std::string>>();

    auto test_vector_de2 = ar.Load<std::vector<std::vector<std::string>>>();

    auto test_invasive_de = ar.Load<Invasive>();

    auto test_non_intrusive_de = ar.Load<NonIntrusive>();

     //NonIntrusive test_non_intrusive_de;
     //ar.Load(fs, test_non_intrusive_de);

    std::cout << "ok\n";
}
