## sezz
现代C++的对象序列化与反序列化魔法

## 特性
- header-only
- C++17
- 支持STL常用容器
- 支持自定义类对象(侵入式与非侵入式)
- 支持std::unique_ptr

## 快速入门
``` C++
fs.open("test.bin", std::ios::binary | std::ios::out | std::ios::in | std::ios::trunc);

std::unordered_map<std::string, std::string> test_map {
    { "pair_key_1", "pair_value_1" },
    { "pair_key_2", "pair_value_2" }
};
sezz::Serialize(fs, test_map);

auto test_map_de = sezz::Deserialize<std::unordered_map<std::string, std::string>>(fs);
```

## 鸣谢
1. [C++序列化对象 ](https://www.cnblogs.com/mmc1206x/p/11053826.html)
2. [计都](https://github.com/fuyouawa)
；
