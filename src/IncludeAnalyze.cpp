#include "IncludeAnalyze.h"
#include <stack>
#include <algorithm>

//检查一个强连通分量是不是真正的include循环。
bool IsRealIncludeCycle(const IncludeGraph& graph, const vector<int>& group)
{
    if (group.size() > 1) {
        return true;
    }

    int nodeIndex = group[0];    //存的是graph.nodes的下标

    //单个文件默认不算循环，除非它有一条指向自己的边。
    for (int edgeIndex : graph.nodes[nodeIndex].outEdges) {
        //edgeIndex存的是graph.edges的下标
        if (graph.edges[edgeIndex].includedFile == nodeIndex) {
            return true;
        }
    }

    return false;
}

//Tarjan算法用来找强连通分量。
//在include图里，一个强连通分量表示：这个组里的文件可以沿着include边互相走到。
//如果这个组满足IsRealIncludeCycle，就说明它是真正的循环include。
//currentNodeIndex存的是graph.nodes的index。
//dfn记录第一次访问这个点的时间，low记录这个点能追溯到的最早dfn。
//belong记录每个点属于哪个强连通分量，0表示还没有分配。
void TarjanSearch(const IncludeGraph& graph, int currentNodeIndex, int& SCCindex, vector<int>& dfn, vector<int>& low, stack<int>& nodeStack, vector<int>& belong, vector<vector<int>>& cycleGroups)
{
    //从已有dfn里找最大值，再给当前点加一。
    //能进入这个函数，说明currentNodeIndex还没有被访问过，所以这里直接分配dfn。
    for (int i = 0; i < dfn.size(); i++) {
        if (dfn[i] > dfn[currentNodeIndex]) {
            dfn[currentNodeIndex] = dfn[i];
        }
    }
    dfn[currentNodeIndex]++;
    low[currentNodeIndex] = dfn[currentNodeIndex];    //一开始只能追溯到自己

    //栈里放的是还没有分配强连通分量的点，存的是graph.nodes的下标。
    nodeStack.push(currentNodeIndex);

    for (int edgeIndex : graph.nodes[currentNodeIndex].outEdges) {
        //edgeIndex存的是graph.edges的下标
        int nextNodeIndex = graph.edges[edgeIndex].includedFile;    //nextNodeIndex指向这条include记录中被include的文件在graph.nodes的下标

        if (dfn[nextNodeIndex] == 0) {
            //树边：nextNodeIndex还没有访问过。
            TarjanSearch(graph, nextNodeIndex, SCCindex, dfn, low, nodeStack, belong, cycleGroups);
            low[currentNodeIndex] = min(low[currentNodeIndex], low[nextNodeIndex]);
        }
        else if (belong[nextNodeIndex] == 0) {
            //回边：nextNodeIndex访问过，但还没有分配强连通分量，说明它还在栈里。
            low[currentNodeIndex] = min(low[currentNodeIndex], dfn[nextNodeIndex]);
        }
        else {
            //弃边：nextNodeIndex已经属于某个强连通分量，这条边不用更新low。
        }
    }

    //如果low和dfn相等，说明currentNodeIndex是一个强连通分量的根。
    //这时从栈顶一直弹到currentNodeIndex，弹出来的这些点就是一个组。
    if (low[currentNodeIndex] == dfn[currentNodeIndex]) {
        vector<int> aGroup;    //里面存的是graph.nodes的下标
        SCCindex++;

        while (nodeStack.empty() == false) {
            int nodeIndex = nodeStack.top();    //存的是graph.nodes的下标
            nodeStack.pop();
            belong[nodeIndex] = SCCindex;    //记录这个点属于第几个强连通分量
            aGroup.push_back(nodeIndex);

            if (nodeIndex == currentNodeIndex) {
                break;
            }
        }

        if (IsRealIncludeCycle(graph, aGroup)) {
            cycleGroups.push_back(aGroup);
        }

    }
}
//HNU-GJF 作品
//检查项目内部有没有循环include，返回循环依赖组。
vector<vector<int>> FindIncludeCycles(const IncludeGraph& graph)
{
    vector<vector<int>> cycleGroups;
    vector<int> dfn;          //每个点第一次被访问的时间，0表示还没访问
    vector<int> low;          //每个点能追溯到的最早dfn
    stack<int> nodeStack;     //Tarjan算法里的栈，里面存的是graph.nodes的下标
    vector<int> belong;       //每个点属于哪个强连通分量，0表示还没有分配
    int SCCindex = 0;         //当前已经分到了第几个强连通分量

    for (int i = 0; i < graph.nodes.size(); i++) {
        dfn.push_back(0);
        low.push_back(0);
        belong.push_back(0);
    }

    for (int i = 0; i < graph.nodes.size(); i++) {
        if (dfn[i] == 0) {
            TarjanSearch(graph, i, SCCindex, dfn, low, nodeStack, belong, cycleGroups);
        }
    }

    return cycleGroups;
}

//沿着反向边继续往上找，得到直接和间接受影响的路径。
//currentNodeIndex存的是graph.nodes的下标。
//currentPath是一条正在查找的影响路径，里面存的是graph.nodes的下标。
//affectedPaths是二维数组，每一行是一条影响路径。
void FindAffectedPaths(const IncludeGraph& graph, int currentNodeIndex, vector<string>& nodeState, vector<int>& currentPath, vector<vector<int>>& affectedPaths)
{
    for (int edgeIndex : graph.nodes[currentNodeIndex].inEdges) {
        //edgeIndex存的是graph.edges的下标
        int nextNodeIndex = graph.edges[edgeIndex].includeFile;    //include了currentNodeIndex的文件，存的是graph.nodes的下标

        if (nodeState[nextNodeIndex] == "未记录") {
            nodeState[nextNodeIndex] = "已记录";
            currentPath.push_back(nextNodeIndex);
            affectedPaths.push_back(currentPath);
            FindAffectedPaths(graph, nextNodeIndex, nodeState, currentPath, affectedPaths);
            currentPath.pop_back();
        }
    }
}
