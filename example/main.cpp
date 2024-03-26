// packer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <packer/packer.hpp>

struct MyStruct
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
};

int main() {
    std::ofstream out{"test.txt"};
    std::ostreambuf_iterator iter{out};
    packer::SerializeTo(iter, MyStruct{ .a = 'a', .b = 'b', .c = 81, .d = 11.45f, .e = {2, 34, 56, -123}, .f = "12343", .g = { {1, "12234"}} });
    out.close();

    std::ifstream in{"test.txt"};
    std::istreambuf_iterator in_iter{in};
    MyStruct kk;
    packer::DeserializeTo(in_iter, &kk);
    return 0;
}
//#include <iostream>
//#include <chrono>
//#include <vector>
//
//int main() {
//    // ��ȡ��ǰʱ��
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
//    // ������������Ҫ���ԵĴ����
//    // ���磬������һЩ��ʱ�Ĳ���
//    for (size_t i = 0; i < 10000; i++) {
//        packer::SerializeTo(iter, total);
//    }
//
//    // ��ȡ����ʱ��
//    auto end = std::chrono::high_resolution_clock::now();
//
//    // ����ʱ��ת��Ϊ����
//    std::chrono::duration<double, std::milli> elapsed = end - start;
//
//    out.close();
//    // ��ӡ���
//    std::cout << "��������ʱ��: " << elapsed.count() << " ����\n";
//
//    return 0;
//}