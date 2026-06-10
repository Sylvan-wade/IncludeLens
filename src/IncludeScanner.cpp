#include "IncludeScanner.h"

#include <fstream>

//判断一个文件是不是目前要扫描的C/C++文件。
bool IsProjectFile(const fs::path& filePath, const IncludeConfig& config)
{
    string extensionName = filePath.extension().string();

    for (const string& aExtension : config.scanExtensions) {
        if (extensionName == aExtension) {
            return true;
        }
    }

    return false;
}

//判断一个字符是不是空格或tab。
bool IsSpaceOrTab(char ch)
{
    return ch == ' ' || ch == '\t';
}

//去掉一行代码最左边的空格或tab，返回处理完的字符串。
string RemoveLeftSpaces(const string& text)
{
    int start = 0;

    while (start < text.size() && IsSpaceOrTab(text[start])) {
        start++;
    }

    return text.substr(start);
}

//从一行代码里解析include，返回保留<>或""的includeRaw。
string GetIncludeFromLine(const string& line)
{
    string cleanLine = RemoveLeftSpaces(line);

    string key = "#include";

    if (cleanLine.find(key) != 0) {
        return "";
    }

    //跳过#include后面的空格，找到真正的<或"。
    int i = key.size(); //从"#include"后面开始
    while (i < cleanLine.size() && IsSpaceOrTab(cleanLine[i])) {
        i++;
    }

    if (i >= cleanLine.size()) {
        return "";
    }

    char left = cleanLine[i];

    //系统、外部头文件一般写成<iostream>，项目头文件一般写成"xxx.h"。
    if (left == '<') {
        int start = i;
        int end = cleanLine.find('>', start + 1);
        if (end == string::npos) {
            return "";
        }

        return cleanLine.substr(start, end - start + 1);
    }

    if (left == '"') {
        int start = i;
        int end = cleanLine.find('"', start + 1);
        if (end == string::npos) {
            return "";
        }

        return cleanLine.substr(start, end - start + 1);
    }

    return "";
}

//根据"Student.h"这种includeRaw，尝试找到项目里的真实文件。
//先按include所在文件的同目录查找，找不到再按文件名在项目里查找。
fs::path FindHeaderFile(const fs::path& includeFilePath, const string& includeRaw, const vector<fs::path>& projectFiles)
{
    fs::path emptyPath; //一个空路径，当我们什么都没找到的时候，就返回这个空路径。

    if (includeRaw.empty() || includeRaw[0] != '"') {   //只找项目内自己定义的头文件,系统或者外部的不做分析。
        return emptyPath;
    }

    string includeFileName = includeRaw.substr(1, includeRaw.size() - 2);   //通过子串截取得到文件名。
    fs::path sameFolderPath = includeFilePath.parent_path() / includeFileName;

    for (const fs::path& aPath : projectFiles) {
        if (aPath == sameFolderPath) {
            return aPath;
        }
    }

    for (const fs::path& aPath : projectFiles) {
        if (aPath.filename().string() == includeFileName) {
            return aPath;
        }
    }

    return emptyPath;
}

//把所有文件里的include关系整理成一张记录表。
vector<IncludeRecord> CollectIncludeRecords(const vector<fs::path>& projectFiles)
{
    vector<IncludeRecord> records;

    for (const fs::path& aPath : projectFiles) {    //遍历所有文件路径
        ifstream inputFile(aPath);

        string lineText;
        bool isFirstLine = true;
        while (getline(inputFile, lineText)) {      //遍历一个文件的所有行
            //有些文件第一行开头会带不可见标记符（UTF-8 with BOM），所以从第一行第一个#开始截。
            if (isFirstLine) {
                int firstJingHao = lineText.find('#');  //第一个#号
                if (firstJingHao != string::npos) {
                    lineText = lineText.substr(firstJingHao);
                }

                isFirstLine = false;
            }

            string includeRaw = GetIncludeFromLine(lineText);  //保留<>或""的include原文
            if (includeRaw != "") {
                IncludeRecord aRecord;
                aRecord.includeFilePath = aPath;
                aRecord.includeRaw = includeRaw;
                aRecord.includedFilePath = FindHeaderFile(aPath, includeRaw, projectFiles);

                if (aRecord.includedFilePath.empty()) {
                    aRecord.isInProject = false;
                }
                else {
                    aRecord.isInProject = true;
                }

                records.push_back(aRecord);
            }
        }
    }

    return records;
}

//判断一个文件夹是不是配置里要求排除的文件夹。
bool IsExcludedFolder(const fs::path& folderPath, const IncludeConfig& config)
{
    string folderName = folderPath.filename().string();

    for (const string& aFolderName : config.excludeFolders) {
        if (folderName == aFolderName) {
            return true;
        }
    }

    return false;
}

//从用户输入的目录开始，递归查找需要的C++文件。
vector<fs::path> CollectProjectFiles(const fs::path& projectFolder, const IncludeConfig& config)
{
    vector<fs::path> projectFiles;
    fs::recursive_directory_iterator end;

    for (fs::recursive_directory_iterator entry(projectFolder); entry != end; entry++) {
        if (entry->is_directory()) {
            if (IsExcludedFolder(entry->path(), config)) {
                entry.disable_recursion_pending();
            }

            continue;
        }

        if (entry->is_regular_file() == false) {
            continue;
        }

        if (IsProjectFile(entry->path(), config)) {
            projectFiles.push_back(entry->path());
        }
    }

    return projectFiles;
}

//在顶点数组里找某个文件。
//这里先用最直观的写法，文件量不大时够用。
//返回值存的是graph.nodes的下标，找不到就返回-1。
int FindNodeByPath(const IncludeGraph& graph, const fs::path& filePath)
{
    for (int i = 0; i < graph.nodes.size(); i++) {
        if (graph.nodes[i].filePath == filePath) {
            return i;
        }
    }

    return -1;
}

//根据include记录表，构建include依赖图。
IncludeGraph BuildIncludeGraph(const vector<fs::path>& projectFiles, const vector<IncludeRecord>& includeRecords)
{
    IncludeGraph graph;

    //先把每个文件放进顶点数组。
    for (const fs::path& aPath : projectFiles) {
        FileNode node;
        node.filePath = aPath;
        graph.nodes.push_back(node);
    }

    //再把项目内头文件的include关系从record表里放进边数组。
    for (const IncludeRecord& aRecord : includeRecords) {
        if (aRecord.isInProject == false) {
            continue;
        }

        int includeFile = FindNodeByPath(graph, aRecord.includeFilePath);       //存的是graph.nodes的下标
        int includedFile = FindNodeByPath(graph, aRecord.includedFilePath);     //存的是graph.nodes的下标

        if (includeFile == -1 || includedFile == -1) {
            continue;
        }

        IncludeEdge aEdge;
        aEdge.includeFile = includeFile;
        aEdge.includedFile = includedFile;
        aEdge.includeRaw = aRecord.includeRaw;

        graph.edges.push_back(aEdge);

        int edge = graph.edges.size() - 1;   //存的是graph.edges的下标
        graph.nodes[includeFile].outEdges.push_back(edge);
        graph.nodes[includedFile].inEdges.push_back(edge);
    }

    return graph;
}

//统计外部或未找到的include数量。
int CountExternalIncludes(const vector<IncludeRecord>& includeRecords)
{
    int externalIncludeCount = 0;

    for (const IncludeRecord& aRecord : includeRecords) {
        if (aRecord.isInProject == false) {
            externalIncludeCount++;
        }
    }

    return externalIncludeCount;
}

//配置改变后要重新扫描文件、重新解析include、重新构图。
void RebuildProjectData(const fs::path& projectFolder, const IncludeConfig& config, vector<fs::path>& projectFiles, vector<IncludeRecord>& includeRecords, IncludeGraph& graph, int& externalIncludeCount)
{
    projectFiles = CollectProjectFiles(projectFolder, config);
    includeRecords = CollectIncludeRecords(projectFiles);
    graph = BuildIncludeGraph(projectFiles, includeRecords);
    externalIncludeCount = CountExternalIncludes(includeRecords);
}
