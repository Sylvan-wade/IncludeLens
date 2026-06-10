#pragma once

#include "IncludeModel.h"

//把一段文本按分隔符切开，用来解析配置里的分号列表。
vector<string> SplitText(const string& text, char separator);

//把字符串数组拼成一段文本，用来保存配置和打印配置。
string JoinText(const vector<string>& texts, char separator);

//生成程序第一次运行时使用的默认配置。
IncludeConfig GetDefaultConfig();

//根据工具所在目录，得到配置文件路径。
fs::path GetConfigFilePath(const fs::path& toolFolder);

//把当前配置写入配置文件。
void SaveConfigFile(const IncludeConfig& config, const fs::path& configFilePath);

//从配置文件读取配置；如果文件不存在，就创建默认配置文件。
IncludeConfig LoadConfigFile(const fs::path& configFilePath);

//在控制台打印当前配置。
void PrintConfig(const IncludeConfig& config, const fs::path& configFilePath);

//把用户输入的后缀整理成带点的形式，比如cpp变成.cpp。
vector<string> NormalizeExtensions(const vector<string>& extensions);

//配置文件菜单，允许用户查看和修改配置。
void ShowConfigMenu(IncludeConfig& config, const fs::path& configFilePath, bool& shouldRebuild);
