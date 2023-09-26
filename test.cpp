#include <sezz/sezz.hpp>


#include <thread>
#include <Geek/ring_queue.hpp>
#include <fstream>

#include <vector>
class Invasive {
public:
    Invasive() :str_() {        // Default constructor required
        int_ = 0;
    }

    Invasive(std::string_view str, int _int) :str_(str), int_(_int) {

    }

    ~Invasive() {

    }

    void Serialize(std::ostream& os) {
        sezz::Serialize(os, str_, int_);
    }

    void Deserialize(std::istream& is) {
        sezz::Deserialize(is, str_, int_);
    }

private:
    std::string str_;
    int int_;
};




class NonIntrusive {
public:     // Non intrusive, requiring external access to data members
    std::string str;
    int aaa;
};


namespace sezz {
// specialization of function templates
template<>
void Serialize/*<NonIntrusive>*/(std::ostream& os, NonIntrusive& val) {
    Serialize(os, val.str, val.aaa);
}
template<>
NonIntrusive Deserialize<NonIntrusive>(std::istream& is) {
    NonIntrusive val;
    Deserialize(is, val.str, val.aaa);
    return val;
}

}


int main()
{
    std::fstream fs;

    fs.open("test.bin", std::ios::binary | std::ios::out | std::ios::in | std::ios::trunc);


    std::unordered_map<std::string, std::string> test_map {
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

    Invasive test_invasive{ "str1", 2};
    sezz::Serialize(fs, test_invasive);

    NonIntrusive test_non_intrusive{ "str1", 2 };
    sezz::Serialize(fs, test_non_intrusive);

    fs.seekg(0);

    auto test_map_de = sezz::Deserialize<std::unordered_map<std::string, std::string>>(fs);

    auto test_vector_de = sezz::Deserialize<std::vector<std::string>>(fs);

    auto test_vector2_de = sezz::Deserialize<std::vector<std::vector<std::string>>>(fs);

    auto test_invasive_de = sezz::Deserialize<Invasive>(fs);

    auto test_non_intrusive_de = sezz::Deserialize<NonIntrusive>(fs);

    std::cout << "ok\n";
}
