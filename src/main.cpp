#include <iostream>

#include "IncludeConfig.h"
#include "IncludeScanner.h"
#include "IncludeOutput.h"

/*
IncludeModel.h：所有结构体。
IncludeConfig：配置文件读写和配置菜单。
IncludeScanner：扫描文件、解析 include、构建图。
IncludeAnalyze：Tarjan、循环依赖、影响路径分析。
IncludeOutput：菜单、控制台输出、Mermaid 生成。
main.cpp：只保留主流程
*/

int main(int argc, char* argv[])
{
    cout << "IncludeLens - C++ include依赖图分析工具" << endl;
    cout << "请输入一个 C++ 项目目录：";

    string input;
    getline(cin, input);

    fs::path projectFolder = fs::path(input);
    if (fs::exists(projectFolder) == false || fs::is_directory(projectFolder) == false) {
        cout << "目录无效。" << endl;
        return 1;
    }

    fs::path toolFolder;
    if (argc > 0) {
        toolFolder = GetToolFolder(argv[0]);
    }
    else {
        toolFolder = fs::current_path();
    }

    fs::path configFilePath = GetConfigFilePath(toolFolder);
    IncludeConfig config = LoadConfigFile(configFilePath);
    vector<fs::path> projectFiles;
    vector<IncludeRecord> includeRecords;
    IncludeGraph graph;
    int externalIncludeCount = 0;
    RebuildProjectData(projectFolder, config, projectFiles, includeRecords, graph, externalIncludeCount);

    cout << endl;
    cout << "共找到 " << projectFiles.size() << " 个 C/C++ 文件。" << endl;

    cout << endl;
    cout << "图构建完成，按回车进入菜单。";
    cin.get();

    while (true) {
        ClearScreen();
        PrintMainMenu();

        int menuChoice = -1;
        cin >> menuChoice;

        ClearScreen();

        if (cin.fail()) {
            cout << "功能编号无效。" << endl;
            WaitForEnter();
            continue;
        }

        if (menuChoice == 1) {
            PrintProjectSummary(graph, projectFiles, externalIncludeCount, config);
            WaitForEnter();
        }
        else if (menuChoice == 2) {
            PrintMostIncludeFiles(graph, projectFiles, config.maxShowCount);
            WaitForEnter();
        }
        else if (menuChoice == 3) {
            PrintMostIncludedFiles(graph, projectFiles, config.maxShowCount);
            WaitForEnter();
        }
        else if (menuChoice == 4) {
            PrintIncludeCycles(graph, projectFiles);
            WaitForEnter();
        }
        else if (menuChoice == 5) {
            ShowAffectedFilesMenu(graph, projectFiles);
            WaitForEnter();
        }
        else if (menuChoice == 6) {
            ShowMermaidMenu(graph, projectFiles, toolFolder);
            WaitForEnter();
        }
        else if (menuChoice == 7) {
            bool shouldRebuild = false;
            ShowConfigMenu(config, configFilePath, shouldRebuild);

            if (shouldRebuild) {
                RebuildProjectData(projectFolder, config, projectFiles, includeRecords, graph, externalIncludeCount);
                cout << "已按最新配置重新扫描项目。" << endl;
                cout << "当前文件顶点数：" << graph.nodes.size() << endl;
            }

            WaitForEnter();
        }
        else if (menuChoice == 0) {
            cout << "已退出 IncludeLens。" << endl;
            break;
        }
        else {
            cout << "功能编号无效。" << endl;
            WaitForEnter();
        }
    }

    return 0;
}
