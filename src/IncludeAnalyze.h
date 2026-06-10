#pragma once

#include "IncludeModel.h"

//检查一个强连通分量是不是真正的include循环。
bool IsRealIncludeCycle(const IncludeGraph& graph, const vector<int>& group);

//检查项目内部有没有循环include，返回循环依赖组。
vector<vector<int>> FindIncludeCycles(const IncludeGraph& graph);

//沿着反向边继续往上找，得到直接和间接受影响的路径。
void FindAffectedPaths(const IncludeGraph& graph, int currentNodeIndex, vector<string>& nodeState, vector<int>& currentPath, vector<vector<int>>& affectedPaths);
