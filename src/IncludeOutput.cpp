#include "IncludeOutput.h"
#include "IncludeAnalyze.h"
#include "IncludeConfig.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <cstdlib>

//图内部用完整路径，输出时只显示短一点的名字。
string GetDisplayPath(const fs::path& filePath, const vector<fs::path>& projectFiles)
{
    string fileNameWithExtension = filePath.filename().string();

    int sameFileNameCount = 0;
    for (const fs::path& aPath : projectFiles) {
        if (aPath.filename().string() == fileNameWithExtension) {
            sameFileNameCount++;
        }
    }

    if (sameFileNameCount == 1) {
        return fileNameWithExtension;
    }

    string parentName = filePath.parent_path().filename().string();
    return parentName + "/" + fileNameWithExtension;
}

//清屏，让每个功能页看起来干净一点。
void ClearScreen()
{
    system("cls");
}

//功能执行完以后停一下，用户按回车再回主菜单。
void WaitForEnter()
{
    cin.clear();
    cout << endl;
    cout << "按回车返回主菜单...";
    cin.ignore(10000, '\n');
    cin.get();
}

//打印主菜单。
void PrintMainMenu()
{
    cout << "IncludeLens - C++ include依赖图分析工具" << endl;
    cout << "----------------------------------------" << endl;
    cout << "1. 查看项目概况" << endl;
    cout << "2. 查看include项目内文件数量排序" << endl;
    cout << "3. 查看被项目内文件include次数排序" << endl;
    cout << "4. 检查循环include" << endl;
    cout << "5. 查看文件影响范围和why路径" << endl;
    cout << "6. 生成Mermaid依赖图" << endl;
    cout << "7. 查看和修改配置文件" << endl;
    cout << "0. 退出" << endl;
    cout << "----------------------------------------" << endl;
    cout << "请输入功能编号：";
}

//打印项目概况和当前配置。
void PrintProjectSummary(const IncludeGraph& graph, const vector<fs::path>& projectFiles, int externalIncludeCount, const IncludeConfig& config)
{
    cout << "项目概况：" << endl;
    cout << "文件顶点数：" << graph.nodes.size() << endl;
    cout << "项目内 include 边数：" << graph.edges.size() << endl;
    cout << "外部/未找到 include：" << externalIncludeCount << " 条" << endl;
    cout << "当前扫描后缀：" << JoinText(config.scanExtensions, ' ') << endl;
    cout << "当前排除目录：" << JoinText(config.excludeFolders, ' ') << endl;
    cout << "排序最多显示：" << config.maxShowCount << " 个" << endl;

    if (projectFiles.empty()) {
        cout << "没有找到符合条件的文件。" << endl;
    }
}

//根据argv[0]找到工具所在目录。
fs::path GetToolFolder(const string& exePathText)
{
    fs::path exePath = fs::absolute(fs::path(exePathText));
    return exePath.parent_path();
}

//把Mermaid节点文字里的双引号替换掉，避免生成的Mermaid语法出错。
string GetMermaidLabel(const string& text)
{
    string label = text;

    for (int i = 0; i < label.size(); i++) {
        if (label[i] == '"') {
            label[i] = '\'';
        }
    }

    return label;
}

//生成图文件时避免覆盖旧文件。
//如果include_graph.md已经存在，就依次尝试include_graph_1.md、include_graph_2.md。
fs::path GetNewMermaidFilePath(const fs::path& outputFolder)
{
    fs::path outputFilePath = outputFolder / "include_graph.md";

    if (fs::exists(outputFilePath) == false) {
        return outputFilePath;
    }

    int fileNumber = 1;
    while (true) {
        fs::path numberedFilePath = outputFolder / ("include_graph_" + to_string(fileNumber) + ".md");

        if (fs::exists(numberedFilePath) == false) {
            return numberedFilePath;
        }

        fileNumber++;
    }
}

//判断某个点是不是在一个循环依赖组里。
//nodeIndex存的是graph.nodes的下标，cycleGroup里面也存的是graph.nodes的下标。
bool IsNodeInCycleGroup(int nodeIndex, const vector<int>& cycleGroup)
{
    for (int aNodeIndex : cycleGroup) {
        if (aNodeIndex == nodeIndex) {
            return true;
        }
    }

    return false;
}

//把一个循环依赖组写成单独的Mermaid小图。
//cycleGroup里面存的是graph.nodes的下标。
void WriteCycleGroupMermaid(ofstream& outputFile, const IncludeGraph& graph, const vector<fs::path>& projectFiles, const vector<int>& cycleGroup, int groupIndex)
{
    outputFile << "### Cycle group " << groupIndex + 1 << endl;
    outputFile << endl;
    outputFile << "```mermaid" << endl;
    outputFile << "graph LR" << endl;

    for (int nodeIndex : cycleGroup) {
        //nodeIndex存的是graph.nodes的下标
        string label = GetMermaidLabel(GetDisplayPath(graph.nodes[nodeIndex].filePath, projectFiles));
        outputFile << "    C" << groupIndex << "N" << nodeIndex << "[\"" << label << "\"]" << endl;
    }

    for (const IncludeEdge& aEdge : graph.edges) {
        if (IsNodeInCycleGroup(aEdge.includeFile, cycleGroup) && IsNodeInCycleGroup(aEdge.includedFile, cycleGroup)) {
            outputFile << "    C" << groupIndex << "N" << aEdge.includeFile
                << " --> C" << groupIndex << "N" << aEdge.includedFile << endl;
        }
    }

    outputFile << "```" << endl;
    outputFile << endl;
}

//生成一个Markdown文件，里面放Mermaid依赖图。
void GenerateMermaidGraphFile(const IncludeGraph& graph, const vector<fs::path>& projectFiles, const fs::path& toolFolder, bool shouldWriteCycleGroups)
{
    fs::path outputFolder = toolFolder / "IncludeLens_outputs";
    fs::create_directories(outputFolder);

    fs::path outputFilePath = GetNewMermaidFilePath(outputFolder);
    ofstream outputFile(outputFilePath, ios::binary);

    if (outputFile.is_open() == false) {
        cout << "生成失败，无法写入文件：" << outputFilePath.string() << endl;
        return;
    }

    //生成的Markdown图文件里固定标题使用英文，避免不同阅读器打开时出现中文编码问题。
    //控制台菜单仍然使用中文，Markdown里真正重要的是Mermaid图本身。
    outputFile << "\xEF\xBB\xBF";
    outputFile << "# IncludeLens include graph" << endl;
    outputFile << endl;
    outputFile << "## Project graph" << endl;
    outputFile << endl;
    outputFile << "```mermaid" << endl;
    outputFile << "graph LR" << endl;

    //先把所有文件都写成Mermaid节点，这样没有include关系的文件也能显示出来。
    for (int i = 0; i < graph.nodes.size(); i++) {
        string label = GetMermaidLabel(GetDisplayPath(graph.nodes[i].filePath, projectFiles));
        outputFile << "    N" << i << "[\"" << label << "\"]" << endl;
    }

    //再把include关系写成有向边：写include的文件 --> 被include的文件。
    for (const IncludeEdge& aEdge : graph.edges) {
        outputFile << "    N" << aEdge.includeFile << " --> N" << aEdge.includedFile << endl;
    }

    outputFile << "```" << endl;

    if (shouldWriteCycleGroups) {
        vector<vector<int>> cycleGroups = FindIncludeCycles(graph);    //每个组里面存的是graph.nodes的下标
        outputFile << endl;
        outputFile << "## Cycle groups" << endl;
        outputFile << endl;

        if (cycleGroups.empty()) {
            outputFile << "No include cycles found." << endl;
        }
        else {
            for (int i = 0; i < cycleGroups.size(); i++) {
                WriteCycleGroupMermaid(outputFile, graph, projectFiles, cycleGroups[i], i);
            }
        }
    }

    outputFile.close();

    cout << "Mermaid依赖图已生成：" << endl;
    cout << outputFilePath.string() << endl;
}

//打印include项目内文件比较多的文件。
void PrintMostIncludeFiles(const IncludeGraph& graph, const vector<fs::path>& projectFiles, int maxShowCount)
{
    vector<int> nodeIndexes;    //里面存的是graph.nodes的index，排序时不改变graph.nodes本身

    for (int i = 0; i < graph.nodes.size(); i++) {
        nodeIndexes.push_back(i);
    }

    sort(nodeIndexes.begin(), nodeIndexes.end(), [&](int aIndex, int bIndex) {
        return graph.nodes[aIndex].outEdges.size() > graph.nodes[bIndex].outEdges.size();
    });

    cout << endl;
    cout << "按include项目内文件数量排序，最多显示" << maxShowCount << "个：" << endl;
    cout << "序号\t数量\t文件" << endl;
    int showCount = maxShowCount;
    if (showCount > nodeIndexes.size()) {
        showCount = nodeIndexes.size();
    }

    for (int i = 0; i < showCount; i++) {
        int nodeIndex = nodeIndexes[i];    //存的是graph.nodes的index
        cout << i + 1 << "\t"
            << graph.nodes[nodeIndex].outEdges.size() << "\t"
            << GetDisplayPath(graph.nodes[nodeIndex].filePath, projectFiles) << endl;
    }
}

//打印被项目内文件include比较多的文件。
void PrintMostIncludedFiles(const IncludeGraph& graph, const vector<fs::path>& projectFiles, int maxShowCount)
{
    vector<int> nodeIndexes;    //里面存的是graph.nodes的index，排序时不改变graph.nodes本身

    for (int i = 0; i < graph.nodes.size(); i++) {
        nodeIndexes.push_back(i);
    }

    sort(nodeIndexes.begin(), nodeIndexes.end(), [&](int aIndex, int bIndex) {
        return graph.nodes[aIndex].inEdges.size() > graph.nodes[bIndex].inEdges.size();
    });

    cout << endl;
    cout << "按被项目内文件include次数排序，最多显示" << maxShowCount << "个：" << endl;
    cout << "序号\t数量\t文件" << endl;
    int showCount = maxShowCount;
    if (showCount > nodeIndexes.size()) {
        showCount = nodeIndexes.size();
    }

    for (int i = 0; i < showCount; i++) {
        int nodeIndex = nodeIndexes[i];    //存的是graph.nodes的index
        cout << i + 1 << "\t"
            << graph.nodes[nodeIndex].inEdges.size() << "\t"
            << GetDisplayPath(graph.nodes[nodeIndex].filePath, projectFiles) << endl;
    }
}

//打印循环include检查结果。
void PrintIncludeCycles(const IncludeGraph& graph, const vector<fs::path>& projectFiles)
{
    vector<vector<int>> cycleGroups = FindIncludeCycles(graph);    //里面存的是graph.nodes的下标

    cout << endl;
    cout << "循环include检查：" << endl;

    if (cycleGroups.empty()) {
        cout << "未发现项目内循环include。" << endl;
        return;
    }

    for (int i = 0; i < cycleGroups.size(); i++) {
        cout << i + 1 << ". ";

        for (int j = 0; j < cycleGroups[i].size(); j++) {
            int nodeIndex = cycleGroups[i][j];    //存的是graph.nodes的下标
            cout << GetDisplayPath(graph.nodes[nodeIndex].filePath, projectFiles);

            if (j != cycleGroups[i].size() - 1) {
                cout << "、";
            }

        }

        cout << endl;
    }
}

//把一条影响路径转成能给人看的文字。
string GetAffectedPathText(const IncludeGraph& graph, const vector<int>& aPath, const vector<fs::path>& projectFiles)
{
    string pathText = "";

    for (int i = 0; i < aPath.size(); i++) {
        int nodeIndex = aPath[i];    //存的是graph.nodes的下标
        pathText += GetDisplayPath(graph.nodes[nodeIndex].filePath, projectFiles);

        if (i != aPath.size() - 1) {
            pathText += " -> ";
        }
    }

    return pathText;
}

//打印一个文件的影响范围，并允许用户查看why路径。
void PrintAffectedFiles(const IncludeGraph& graph, int selectedNodeIndex, const vector<fs::path>& projectFiles)
{
    cout << endl;
    cout << "你选择的文件：" << GetDisplayPath(graph.nodes[selectedNodeIndex].filePath, projectFiles) << endl;
    cout << "影响范围（直接或间接include了它的项目内文件）：" << endl;

    if (graph.nodes[selectedNodeIndex].inEdges.empty()) {
        cout << "没有发现会受它影响的项目内文件。" << endl;
        return;
    }

    vector<string> nodeState;
    vector<int> currentPath;             //当前正在查找的一条影响路径，里面存的是graph.nodes的下标
    vector<vector<int>> affectedPaths;   //每一行是一条影响路径，里面存的是graph.nodes的下标

    for (int i = 0; i < graph.nodes.size(); i++) {
        nodeState.push_back("未记录");
    }

    nodeState[selectedNodeIndex] = "已记录";
    currentPath.push_back(selectedNodeIndex);
    FindAffectedPaths(graph, selectedNodeIndex, nodeState, currentPath, affectedPaths);

    if (affectedPaths.empty()) {
        cout << "没有发现会受它影响的项目内文件。" << endl;
        return;
    }

    cout << "序号\t文件\t影响类型" << endl;
    for (int i = 0; i < affectedPaths.size(); i++) {
        const vector<int>& aPath = affectedPaths[i];    //里面存的是graph.nodes的下标
        int affectedNodeIndex = aPath[aPath.size() - 1];    //存的是graph.nodes的下标

        cout << i + 1 << "\t"
            << GetDisplayPath(graph.nodes[affectedNodeIndex].filePath, projectFiles) << "\t";

        if (aPath.size() == 2) {
            cout << "直接";
        }
        else {
            cout << "间接";
        }

        cout << endl;
    }

    cout << endl;
    cout << "请输入要查看影响路径的文件编号，输入 0 跳过：" << endl;

    int pathChoice = 0;
    cin >> pathChoice;

    if (cin.fail()) {
        cout << "编号无效，已跳过影响路径查看。" << endl;
        return;
    }

    if (pathChoice > 0 && pathChoice <= affectedPaths.size()) {
        const vector<int>& selectedPath = affectedPaths[pathChoice - 1];    //里面存的是graph.nodes的下标
        int affectedNodeIndex = selectedPath[selectedPath.size() - 1];      //存的是graph.nodes的下标

        cout << GetDisplayPath(graph.nodes[affectedNodeIndex].filePath, projectFiles) << " 的影响路径：" << endl;
        cout << GetAffectedPathText(graph, selectedPath, projectFiles) << endl;
    }
    else if (pathChoice != 0) {
        cout << "编号无效，已跳过影响路径查看。" << endl;
    }
}

//影响范围功能的二级菜单。
void ShowAffectedFilesMenu(const IncludeGraph& graph, const vector<fs::path>& projectFiles)
{
    if (graph.nodes.empty()) {
        cout << "当前项目没有可分析的文件。" << endl;
        return;
    }

    cout << "请选择要分析的文件：" << endl;

    for (int i = 0; i < graph.nodes.size(); i++) {
        cout << i + 1 << ". " << GetDisplayPath(graph.nodes[i].filePath, projectFiles) << endl;
    }

    cout << "输入 0 返回主菜单。" << endl;
    cout << "请输入文件编号：";

    int choice = 0;
    cin >> choice;

    if (cin.fail()) {
        cout << "编号无效，已返回主菜单。" << endl;
        return;
    }

    if (choice > 0 && choice <= graph.nodes.size()) {
        PrintAffectedFiles(graph, choice - 1, projectFiles);
    }
    else if (choice != 0) {
        cout << "编号无效，已返回主菜单。" << endl;
    }
}

//Mermaid生图功能的二级菜单。
void ShowMermaidMenu(const IncludeGraph& graph, const vector<fs::path>& projectFiles, const fs::path& toolFolder)
{
    cout << "生成Mermaid依赖图：" << endl;
    cout << "1. 只生成项目主图" << endl;
    cout << "2. 生成项目主图，并附加循环依赖分组" << endl;
    cout << "0. 返回主菜单" << endl;
    cout << "请输入功能编号：" << endl;

    int choice = 0;
    cin >> choice;

    if (cin.fail()) {
        cout << "功能编号无效，已返回主菜单。" << endl;
        return;
    }

    if (choice == 1) {
        GenerateMermaidGraphFile(graph, projectFiles, toolFolder, false);
    }
    else if (choice == 2) {
        GenerateMermaidGraphFile(graph, projectFiles, toolFolder, true);
    }
    else if (choice != 0) {
        cout << "功能编号无效，已返回主菜单。" << endl;
    }
}
