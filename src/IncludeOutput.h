#pragma once

#include "IncludeModel.h"

//图内部用完整路径，输出时只显示短一点的名字。
string GetDisplayPath(const fs::path& filePath, const vector<fs::path>& projectFiles);

//清屏，让每个功能页看起来干净一点。
void ClearScreen();

//功能执行完以后停一下，用户按回车再回主菜单。
void WaitForEnter();

//打印主菜单。
void PrintMainMenu();

//打印项目概况和当前配置。
void PrintProjectSummary(const IncludeGraph& graph, const vector<fs::path>& projectFiles, int externalIncludeCount, const IncludeConfig& config);

//根据argv[0]找到工具所在目录。
fs::path GetToolFolder(const string& exePathText);

//生成Markdown依赖图文件。
void GenerateMermaidGraphFile(const IncludeGraph& graph, const vector<fs::path>& projectFiles, const fs::path& toolFolder, bool shouldWriteCycleGroups);

//打印include项目内文件比较多的文件。
void PrintMostIncludeFiles(const IncludeGraph& graph, const vector<fs::path>& projectFiles, int maxShowCount);

//打印被项目内文件include比较多的文件。
void PrintMostIncludedFiles(const IncludeGraph& graph, const vector<fs::path>& projectFiles, int maxShowCount);

//打印循环include检查结果。
void PrintIncludeCycles(const IncludeGraph& graph, const vector<fs::path>& projectFiles);

//把一条影响路径转成能给人看的文字。
string GetAffectedPathText(const IncludeGraph& graph, const vector<int>& aPath, const vector<fs::path>& projectFiles);

//打印一个文件的影响范围，并允许用户查看why路径。
void PrintAffectedFiles(const IncludeGraph& graph, int selectedNodeIndex, const vector<fs::path>& projectFiles);

//影响范围功能的二级菜单。
void ShowAffectedFilesMenu(const IncludeGraph& graph, const vector<fs::path>& projectFiles);

//Mermaid生图功能的二级菜单。
void ShowMermaidMenu(const IncludeGraph& graph, const vector<fs::path>& projectFiles, const fs::path& toolFolder);
