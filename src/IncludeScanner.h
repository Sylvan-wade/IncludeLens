#pragma once

#include "IncludeModel.h"

//判断一个文件是不是目前要扫描的C/C++文件。
bool IsProjectFile(const fs::path& filePath, const IncludeConfig& config);

//判断一个字符是不是空格或tab。
bool IsSpaceOrTab(char ch);

//去掉一行代码最左边的空格或tab，返回处理完的字符串。
string RemoveLeftSpaces(const string& text);

//从一行代码里解析include，返回保留<>或""的includeRaw。
string GetIncludeFromLine(const string& line);

//根据"Student.h"这种includeRaw，尝试找到项目里的真实文件。
fs::path FindHeaderFile(const fs::path& includeFilePath, const string& includeRaw, const vector<fs::path>& projectFiles);

//把所有文件里的include关系整理成一张记录表。
vector<IncludeRecord> CollectIncludeRecords(const vector<fs::path>& projectFiles);

//判断一个文件夹是不是配置里要求排除的文件夹。
bool IsExcludedFolder(const fs::path& folderPath, const IncludeConfig& config);

//从用户输入的目录开始，递归查找需要的C++文件。
vector<fs::path> CollectProjectFiles(const fs::path& projectFolder, const IncludeConfig& config);

//在顶点数组里找某个文件，返回graph.nodes的下标。
int FindNodeByPath(const IncludeGraph& graph, const fs::path& filePath);

//根据include记录表，构建include依赖图。
IncludeGraph BuildIncludeGraph(const vector<fs::path>& projectFiles, const vector<IncludeRecord>& includeRecords);

//统计外部或未找到的include数量。
int CountExternalIncludes(const vector<IncludeRecord>& includeRecords);

//重新扫描文件、重新解析include、重新构图。
void RebuildProjectData(const fs::path& projectFolder, const IncludeConfig& config, vector<fs::path>& projectFiles, vector<IncludeRecord>& includeRecords, IncludeGraph& graph, int& externalIncludeCount);
