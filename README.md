# IncludeLens

IncludeLens 是一个基于 C++17 的 Windows 控制台工具，用来分析 C++ 项目里的 `#include` 依赖关系。

它的核心思路是：把每一个源码文件或头文件看成图里的一个节点，把项目内的 include 关系看成一条有向边。这样，循环 include、修改影响范围、依赖路径解释等问题，都可以转化成图问题来处理。

## 主要功能

- 递归扫描 C/C++ 项目目录
- 解析常见的 `#include <...>` 和 `#include "..."`
- 将项目内头文件匹配到真实文件路径
- 构建 include 有向依赖图
- 统计文件 include 项目内文件的数量
- 统计文件被项目内文件 include 的次数
- 使用 Tarjan 强连通分量算法检测循环 include
- 分析某个文件被修改后可能影响到哪些文件
- 输出一条 why 路径，解释文件为什么会受到影响
- 生成 Mermaid Markdown 依赖图
- 支持配置扫描文件后缀和排除目录

## 技术点

- C++17
- STL 容器
- `std::filesystem`
- 数组版邻接表
- DFS
- Tarjan 强连通分量算法
- Mermaid 图生成
- 简单文本配置文件

## 项目结构

```text
src
├─ main.cpp              程序入口和主菜单流程
├─ IncludeModel.h        数据结构定义
├─ IncludeConfig.h       配置文件相关声明
├─ IncludeConfig.cpp     配置文件读写和配置菜单
├─ IncludeScanner.h      文件扫描和 include 解析声明
├─ IncludeScanner.cpp    文件扫描、include 解析和依赖图构建
├─ IncludeAnalyze.h      图分析相关声明
├─ IncludeAnalyze.cpp    循环依赖检测和影响范围分析
├─ IncludeOutput.h       控制台输出和 Mermaid 生成声明
└─ IncludeOutput.cpp     菜单输出、统计输出和 Mermaid 文件生成
```

## 运行方式

### 方式一：直接运行 exe

双击或在命令行运行：

```text
IncludeLens.exe
```

程序启动后，输入一个 C/C++ 项目目录，例如：

```text
C:\Users\24019\Desktop\MyCppProject
```

然后根据菜单选择功能。

### 方式二：用 Visual Studio 打开源码

1. 双击 `IncludeLens.sln`
2. 选择 `Release | x64` 或 `Debug | x64`
3. 点击生成解决方案
4. 运行程序

## 菜单功能

```text
1. 查看项目概况
2. 查看include项目内文件数量排序
3. 查看被项目内文件include次数排序
4. 检查循环include
5. 查看文件影响范围和why路径
6. 生成Mermaid依赖图
7. 查看和修改配置文件
0. 退出
```

## 配置文件

程序第一次运行时，会在工具所在目录自动生成：

```text
IncludeLens_config.txt
```

默认内容类似：

```text
scan_extensions=.cpp;.h;.hpp
exclude_folders=build;x64;Debug;Release;.git
max_show_count=10
```

含义：

- `scan_extensions`：要扫描的文件后缀
- `exclude_folders`：递归扫描时要跳过的文件夹名称
- `max_show_count`：排序结果最多显示多少条

也可以在程序菜单第 7 项里直接查看和修改配置。

## Mermaid 依赖图

选择菜单第 6 项后，程序会在工具所在目录下生成：

```text
IncludeLens_outputs
```

里面会保存 Markdown 图文件，例如：

```text
include_graph.md
include_graph_1.md
include_graph_2.md
```

这些文件可以用 Obsidian、VS Code Markdown 预览插件或其他支持 Mermaid 的 Markdown 阅读器打开。

为了避免不同 Markdown 阅读器在中文编码上出现显示问题，生成的 Markdown 文件中的固定标题使用英文；控制台菜单和程序交互仍然使用中文。

## 测试样例

项目里提供了一个测试样例：

```text
samples\include_lens_mermaid_test
```

这个样例包含：

- 普通 include 链
- 同名 `config.h`
- 二文件循环 include
- 自 include
- 孤立文件
- 外部 include
- 找不到的 include

适合用来测试扫描、构图、循环检测、影响范围分析和 Mermaid 生图功能。

## 当前不支持的内容

为了保持项目简单、可完成、可解释，当前版本不做：

- 宏展开
- 条件编译求值
- 完整 C/C++ 预处理器
- `compile_commands.json`
- GUI
- 复杂 include 语义分析

IncludeLens 更接近一个轻量级 include 依赖图分析工具，而不是完整编译器前端。
