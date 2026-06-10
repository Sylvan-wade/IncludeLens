#pragma once

#include <string>
#include <vector>
#include <filesystem>

using namespace std;

namespace fs = filesystem;

//一个文件的一个include对应的一条记录。
struct IncludeRecord
{
    fs::path includeFilePath;    //写了这条include的文件
    string includeRaw;           //include里面原样写的内容，比如<iostream>或"Student.h"，保留<>和""
    fs::path includedFilePath;   //如果匹配到项目内真实文件，就放这里
    bool isInProject;            //这个include是否指向项目里自己写的文件
};

//include图里的一个顶点，对应一个.cpp/.h/.hpp文件。
struct FileNode
{
    fs::path filePath;              //文件的完整路径
    vector<int> outEdges;           //这个文件include了哪些项目内文件，里面存的是graph.edges的下标
    vector<int> inEdges;            //哪些项目内文件include了这个文件，里面存的是graph.edges的下标
};

//include图里的一条有向边。
//includeFile -> includedFile表示一个include关系。
struct IncludeEdge
{
    int includeFile;    //写了#include的文件，存的是graph.nodes的下标
    int includedFile;   //被include的项目内文件，存的是graph.nodes的下标
    string includeRaw;   //include原文，比如"Student.h"，保留双引号
};

//include依赖图，nodes是顶点，edges是边。
struct IncludeGraph
{
    vector<FileNode> nodes;
    vector<IncludeEdge> edges;
};

struct IncludeConfig
{
    vector<string> scanExtensions;      //要扫描的文件后缀，比如.cpp、.h、.hpp
    vector<string> excludeFolders;      //要排除的文件夹名称，比如build、x64
    int maxShowCount;                   //排序结果最多显示多少个
};
