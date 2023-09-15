#include <sezz/sezz.hpp>

#include <thread>
#include <Geek/ring_queue.hpp>

#include <vector>
class User {
public:
    void Serialize(std::ostream& os) {
        sezz::Serialize(os, str);
    }

public:
    std::string str;
};

template<typename T, typename = void>
struct is_aaa_t : std::false_type {};
template<typename T>
struct is_aaa_t<T, std::void_t<decltype(&T::Serialize)>> : std::true_type {};



int main()
{
    //xxxxx<int>;
    uint32_t* buf = new uint32_t[1024];
    RingQueue<uint32_t> queue(buf, 1024);
    clock_t start, end;
    start = clock();
    

    std::thread enqueue([&]() {
        for (int i = 0; i < 10000000; i++) {
            queue.Enqueue(100);
        }
    });

    std::thread dequeue([&]() {
        uint32_t a;
        for (int i = 0; i < 10000000; i++) {
            queue.Dequeue(&a);
        }
    });

    enqueue.join();
    dequeue.join();

    end = clock();

    std::cout << "F1运行时间" << (double)(end - start) << "ms" << std::endl;

    std::fstream f;

    f.open("qqq.txt", std::ios::binary | std::ios::out | std::ios::in | std::ios::trunc);


    std::unordered_set<std::string> fake;
    //std::unordered_map<std::string, std::string> fake2 {
    //    {"a", "b"},
    //    { "cc", "dd" }
    //};

    /*std::string str = "???";
    sezz::Serialize(f, str);*/

    std::vector<std::string> fake2 = { "adawwd", "dawwdwa", "csac" };
    std::vector<std::string> fake3 = { "daccs", "2e12", "zCc" };

    User user;
    sezz::is_user_v<User>;

    sezz::Serialize(f, fake2, fake3);


    f.close();

    f.open("qqq.txt");


    // auto v = sezz::Deserialize<std::unordered_map<std::string, std::string>>(f);
    auto v = sezz::Deserialize<std::vector<std::string>>(f);
    auto v2 = sezz::Deserialize<std::vector<std::string>>(f);
    std::cout << "Hello World!\n";
}
