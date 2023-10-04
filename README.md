## sezz
现代C++的对象序列化与反序列化魔法

## 特性
- header-only
- C++20
- 支持STL常用容器
- 支持自定义类对象(侵入式与非侵入式)
- 支持std::unique_ptr
- 基于zigzag/varint编码＆解码u/i 16/32/64
    - 若不使用浮点数，通常不需要考虑网络字节序问题。

## 快速入门
``` C++
#include <iostream>
#include <fstream>

#include <sezz/sezz.hpp>
#include <sezz/stl/string.hpp>
#include <sezz/stl/unordered_map.hpp>

int main() {
    std::fstream fs;
    fs.open("test.bin", std::ios::binary | std::ios::out | std::ios::in | std::ios::trunc);

    sezz::BinaryArchive<std::iostream> ar(fs);

    std::unordered_map<std::string, std::string> test_map {
        { "pair_key_1", "pair_value_1" },
        { "pair_key_2", "pair_value_2" }
    };
    ar.Save(test_map);

    fs.seekg(0);

    auto test_map_de = ar.Load<std::unordered_map<std::string, std::string>>();
}
```

## 鸣谢
1. [C++序列化对象 ](https://www.cnblogs.com/mmc1206x/p/11053826.html)
2. [计都](https://github.com/fuyouawa)
    - *为此项目的建设提供了许多指导。*
3. [cereal](https://github.com/USCiLab/cereal)
    - *参考了一些设计。*