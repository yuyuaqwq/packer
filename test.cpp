#include <sezz/sezz.hpp>

#include <fstream>

class User {
public:
    void Serialize(std::ostream& os) {
        sezz::Serialize(os, str, aaa);
    }

    void Deserialize(std::istream& is) {
        sezz::Deserialize(is, str, aaa);
    }

public:
    std::string str;
    int aaa;
};

int main()
{
    std::fstream f;

    f.open("qqq.txt", std::ios::binary | std::ios::out | std::ios::in | std::ios::trunc);


    std::unordered_set<std::string> fake;
    //std::unordered_map<std::string, std::string> fake2 {
    //    {"a", "b"},
    //    { "cc", "dd" }
    //};

    /*std::string str = "???";
    sezz::Serialize(f, str);*/

    std::is_pointer_v<int*>;
    std::remove_pointer_t<int*>;

    std::vector<std::string> fake2 = { "adawwd", "dawwdwa", "csac" };
    std::vector<std::string> fake3 = { "daccs", "2e12", "zCc" };

    int intv = 0xaaaaaaaa;
    int* intp = &intv;

    auto aaa = std::make_unique<int>(100);
    
    sezz::Serialize(f, aaa);

    //User user{ "user", 0xaaa };
    //sezz::is_user_serializable_v<User>;
    //sezz::is_user_deserializable_v<User>;

    //sezz::Serialize(f, user);


    f.close();

    f.open("qqq.txt");


    // auto v = sezz::Deserialize<std::unordered_map<std::string, std::string>>(f);
    //int* intv2 = 0;
    //std::vector<std::string> dese1, dese2;
    //auto user2 = sezz::Deserialize<User>(f);
    

    auto ptr = sezz::Deserialize<std::unique_ptr<int>>(f);

    auto v = sezz::Deserialize<std::vector<std::string>>(f);
    auto v2 = sezz::Deserialize<std::vector<std::string>>(f);

    std::cout << "Hello World!\n";
}
