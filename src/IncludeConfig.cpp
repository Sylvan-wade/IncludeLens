#include "IncludeConfig.h"

#include <iostream>
#include <fstream>
#include <cstdlib>

//把一段文本按分隔符切开，用来解析配置里的分号列表。
vector<string> SplitText(const string& text, char separator)
{
    vector<string> parts;
    string currentText = "";

    for (int i = 0; i < text.size(); i++) {
        if (text[i] == separator) {
            if (currentText != "") {
                parts.push_back(currentText);
            }

            currentText = "";
        }
        else {
            currentText += text[i];
        }
    }

    if (currentText != "") {
        parts.push_back(currentText);
    }

    return parts;
}

//把字符串数组拼成一段文本，用来保存配置和打印配置。
string JoinText(const vector<string>& texts, char separator)
{
    string result = "";

    for (int i = 0; i < texts.size(); i++) {
        result += texts[i];

        if (i != texts.size() - 1) {
            result += separator;
        }
    }

    return result;
}

//生成程序第一次运行时使用的默认配置。
IncludeConfig GetDefaultConfig()
{
    IncludeConfig config;
    config.scanExtensions.push_back(".cpp");
    config.scanExtensions.push_back(".h");
    config.scanExtensions.push_back(".hpp");
    config.excludeFolders.push_back("build");
    config.excludeFolders.push_back("x64");
    config.excludeFolders.push_back("Debug");
    config.excludeFolders.push_back("Release");
    config.excludeFolders.push_back(".git");
    config.maxShowCount = 10;
    return config;
}

//根据工具所在目录，得到配置文件路径。
fs::path GetConfigFilePath(const fs::path& toolFolder)
{
    return toolFolder / "IncludeLens_config.txt";
}

//把当前配置写入配置文件。
void SaveConfigFile(const IncludeConfig& config, const fs::path& configFilePath)
{
    ofstream outputFile(configFilePath);

    if (outputFile.is_open() == false) {
        cout << "配置文件保存失败：" << configFilePath.string() << endl;
        return;
    }

    outputFile << "scan_extensions=" << JoinText(config.scanExtensions, ';') << endl;
    outputFile << "exclude_folders=" << JoinText(config.excludeFolders, ';') << endl;
    outputFile << "max_show_count=" << config.maxShowCount << endl;
    outputFile.close();
}

//从配置文件读取配置；如果文件不存在，就创建默认配置文件。
IncludeConfig LoadConfigFile(const fs::path& configFilePath)
{
    IncludeConfig config = GetDefaultConfig();

    if (fs::exists(configFilePath) == false) {
        SaveConfigFile(config, configFilePath);
        return config;
    }

    ifstream inputFile(configFilePath);
    string lineText;

    //配置文件只保存英文键名、文件后缀、文件夹名和数字，所以这里按普通文本读取即可。
    while (getline(inputFile, lineText)) {
        string scanKey = "scan_extensions=";
        string excludeKey = "exclude_folders=";
        string countKey = "max_show_count=";

        if (lineText.find(scanKey) == 0) {
            string value = lineText.substr(scanKey.size());
            vector<string> extensions = SplitText(value, ';');

            if (extensions.empty() == false) {
                config.scanExtensions = extensions;
            }
        }
        else if (lineText.find(excludeKey) == 0) {
            string value = lineText.substr(excludeKey.size());
            config.excludeFolders = SplitText(value, ';');
        }
        else if (lineText.find(countKey) == 0) {
            string value = lineText.substr(countKey.size());
            int count = atoi(value.c_str());

            if (count > 0) {
                config.maxShowCount = count;
            }
        }
    }

    return config;
}

//在控制台打印当前配置。
void PrintConfig(const IncludeConfig& config, const fs::path& configFilePath)
{
    cout << "配置文件路径：" << configFilePath.string() << endl;
    cout << "扫描后缀：" << JoinText(config.scanExtensions, ' ') << endl;
    cout << "排除目录：" << JoinText(config.excludeFolders, ' ') << endl;
    cout << "排序最多显示：" << config.maxShowCount << " 个" << endl;
}

//把用户输入的后缀整理成带点的形式，比如cpp变成.cpp。
vector<string> NormalizeExtensions(const vector<string>& extensions)
{
    vector<string> result;

    for (string aExtension : extensions) {
        if (aExtension == "") {
            continue;
        }

        if (aExtension[0] != '.') {
            aExtension = "." + aExtension;
        }

        result.push_back(aExtension);
    }

    return result;
}

//配置文件菜单，允许用户查看和修改配置。
void ShowConfigMenu(IncludeConfig& config, const fs::path& configFilePath, bool& shouldRebuild)
{
    while (true) {
        cout << "配置文件菜单：" << endl;
        cout << "1. 查看当前配置" << endl;
        cout << "2. 修改扫描文件后缀" << endl;
        cout << "3. 修改排除文件夹名称" << endl;
        cout << "4. 修改排序最多显示数量" << endl;
        cout << "5. 恢复默认配置" << endl;
        cout << "6. 从配置文件重新读取" << endl;
        cout << "0. 返回主菜单" << endl;
        cout << "请输入功能编号：";

        int choice = 0;
        cin >> choice;

        if (cin.fail()) {
            cout << "功能编号无效，已返回主菜单。" << endl;
            return;
        }

        if (choice == 1) {
            PrintConfig(config, configFilePath);
        }
        else if (choice == 2) {
            cout << "请输入要扫描的后缀，用分号分隔，比如 .cpp;.h;.hpp" << endl;
            cin.ignore(10000, '\n');

            string inputText;
            getline(cin, inputText);

            vector<string> extensions = NormalizeExtensions(SplitText(inputText, ';'));
            if (extensions.empty()) {
                cout << "输入为空，配置未修改。" << endl;
            }
            else {
                config.scanExtensions = extensions;
                SaveConfigFile(config, configFilePath);
                shouldRebuild = true;
                cout << "已保存扫描后缀配置。" << endl;
            }
        }
        else if (choice == 3) {
            cout << "请输入要排除的文件夹名称，用分号分隔，比如 build;x64;Debug" << endl;
            cin.ignore(10000, '\n');

            string inputText;
            getline(cin, inputText);

            config.excludeFolders = SplitText(inputText, ';');
            SaveConfigFile(config, configFilePath);
            shouldRebuild = true;
            cout << "已保存排除目录配置。" << endl;
        }
        else if (choice == 4) {
            cout << "请输入排序最多显示数量：";

            int count = 0;
            cin >> count;

            if (cin.fail() || count <= 0) {
                cout << "数量无效，配置未修改。" << endl;
                cin.clear();
            }
            else {
                config.maxShowCount = count;
                SaveConfigFile(config, configFilePath);
                cout << "已保存排序显示数量。" << endl;
            }
        }
        else if (choice == 5) {
            config = GetDefaultConfig();
            SaveConfigFile(config, configFilePath);
            shouldRebuild = true;
            cout << "已恢复默认配置。" << endl;
        }
        else if (choice == 6) {
            config = LoadConfigFile(configFilePath);
            shouldRebuild = true;
            cout << "已重新读取配置文件。" << endl;
        }
        else if (choice == 0) {
            return;
        }
        else {
            cout << "功能编号无效。" << endl;
        }

        cout << endl;
    }
}
