## sezz
现代C++的对象序列化与反序列化魔法

## 特性
- header-only
- C++20
- 支持STL常用容器
- 支持自定义类对象(侵入式与非侵入式)
- 支持智能指针与原始指针
    - 原始指针遵循现代C++实践，不持有对象所有权
    - 当前设计仅允许原始指针指向`std::unique_ptr`持有的对象
- 支持`std::optional`, `std::tuple`
- 字节序无关
    - 基于`varint`/`zigzag`编码＆解码`u/i 16/32/64`
    - 浮点数则在序列化时将其转换为小端序，反序列化时转换为本机字节序，仅支持本机为小端/大端

## 基准

### 环境

-   CPU: Intel(R) Core(TM) i7-8700K CPU @ 3.70GHz   3.70 GHz

-   RAM: 32.0 GB

-   Compiler: MSVC - v1929,  /O2

### 测试数据

-   ntdll.dll - pdber object

-   RawSize: `417,507 bytes` \* `4000 count` = `1670028000 bytes` ≈ `1.5 GB`

-   IoStream: `sezz::MemoryIoStream`

-   ObjectCount:
    | std::string | total integral(bool、uint8_t、size_t) |
    | ----------- | -------- |
    | 46,900,000  | 51,484,000 |


### 结果

#### Serialize
| Init IoStream Buffer Size | Serialized size       | Time       |
| ------------------------- | --------------------- | ---------- |
| `1,024 bytes`             | `1,028,876,000 bytes` | `1,557 ms` |
| `1,028,876,000 bytes`     | `1,028,876,000 bytes` | `1,166 ms` |

#### Deserialize
time: `2,685 ms`
- 构造`std::string`时的`new`，占用了许多反序列化的时间。


#### 性能
| Type                 | Performance(RawSize) |
| -------------------- | -------------------- |
| Serialize.init_small | ≈1,022MB/s           |
| Serialize.init_big   | ≈1,365MB/s           |
| Deserialize          | ≈593MB/s             |



#### 不用看
~~鄙视一下cpp-serializers这个项目，yas测试用的自己的MemoryIoStream，cereal用的std::stringstream，实际性能差距都在这里。~~


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

## 引用&鸣谢
1. [C++序列化对象 ](https://www.cnblogs.com/mmc1206x/p/11053826.html)
2. [计都](https://github.com/fuyouawa)
    - *为此项目的建设提供了许多指导。*
3. [cereal](https://github.com/USCiLab/cereal)
    - *参考了一些设计。*
4. [如何实现对多个字节的数据序列化+压缩](https://www.eet-china.com/mp/a202331.html)