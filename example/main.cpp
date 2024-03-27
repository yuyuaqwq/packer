// packer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <unordered_set>
#include <tuple>
#include <packer/packer.hpp>



// 非侵入式序列化测试类
class MyClass
{
public:
    MyClass() {}
    ~MyClass() {}

    struct
    {
        struct {
            char ch;
        } a;
        char b;
        int c;
        float d;
        std::vector<int> e;
        std::string f;
        std::map<int, std::string> g;
    } ppp;
    std::unordered_set<std::string> jjj;
};

// 自定义序列化(非侵入式)
template<>
struct packer::Packer<MyClass> {
    void Serialize(const MyClass& val, auto& ctx) {
        packer::SerializeTo(ctx.iter(), val.ppp);
        packer::SerializeTo(ctx.iter(), val.jjj);
    }

    void Deserialize(MyClass* val, auto& ctx) {
        packer::DeserializeTo(ctx.iter(), &val->ppp);
        packer::DeserializeTo(ctx.iter(), &val->jjj);
    }
};

// 侵入式序列化测试类
class MyClass2
{
public:
    MyClass2() {}
    MyClass2(std::string p, std::tuple<int, float, uint64_t> a) : ppp{ p }, aaaasd{ a } {}
    ~MyClass2() {}

    void Serialize(auto& ctx) const {
        packer::SerializeTo(ctx.iter(), ppp);
        packer::SerializeTo(ctx.iter(), aaaasd);
    }

    void Deserialize(auto& ctx) {
        packer::DeserializeTo(ctx.iter(), &ppp);
        packer::DeserializeTo(ctx.iter(), &aaaasd);
    }

private:
    std::string ppp;
    std::tuple<int, float, uint64_t> aaaasd;
};

int main() {
    std::ofstream out{"test.txt"};
    MyClass ccc;
    ccc.ppp = { .a = 'a', .b = 'b', .c = 81, .d = 11.45f, .e = {2, 34, 56, -123}, .f = "12343", .g = { {1, "12234"}} };
    ccc.jjj = { "1231", "Sdfsd" };
    packer::SerializeTo(std::ostreambuf_iterator{out}, ccc);
    packer::SerializeTo(std::ostreambuf_iterator{out}, MyClass2{ "12312", {1122, 222.9f, 1145141919810ull} });
    out.close();

    std::ifstream in{"test.txt"};
    std::istreambuf_iterator in_iter{in};
    MyClass cccppp;
    packer::DeserializeTo(std::istreambuf_iterator{in}, &cccppp);
    MyClass2 cc222;
    packer::DeserializeTo(std::istreambuf_iterator{in}, &cc222);
    in.close();
    return 0;
}
//#include <iostream>
//#include <chrono>
//#include <vector>
//
//int main() {
//    // 获取当前时间
//    std::string total;
//    for (size_t i = 0; i < 10000; i++)
//    {
//        total.push_back('x');
//    }
//    std::ofstream out{"test2.txt"};
//    std::ostreambuf_iterator iter{out};
//
//    auto start = std::chrono::high_resolution_clock::now();
//    out.seekp(0);
//
//    // 在这里运行你要测试的代码块
//    // 例如，可以是一些耗时的操作
//    for (size_t i = 0; i < 10000; i++) {
//        packer::SerializeTo(iter, total);
//    }
//
//    // 获取结束时间
//    auto end = std::chrono::high_resolution_clock::now();
//
//    // 计算时间差并转换为毫秒
//    std::chrono::duration<double, std::milli> elapsed = end - start;
//
//    out.close();
//    // 打印结果
//    std::cout << "程序运行时间: " << elapsed.count() << " 毫秒\n";
//
//    return 0;
//}