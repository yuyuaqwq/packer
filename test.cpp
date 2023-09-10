#include <sezz/sezz.hpp>


class User {
public:
    void Serialize(std::ostream& os) {
        sezz::Serialize(os, str);
    }

public:
    std::string str;
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

    // std::vector<std::string> fake2 = { "adawwd", "dawwdwa", "csac" };

    User user;
    sezz::is_user_v<User>;

    sezz::Serialize(f, user);


    f.close();

    f.open("qqq.txt");


    // auto v = sezz::Deserialize<std::unordered_map<std::string, std::string>>(f);
    auto v = sezz::Deserialize<std::vector<std::string>>(f);
    std::cout << "Hello World!\n";
}
