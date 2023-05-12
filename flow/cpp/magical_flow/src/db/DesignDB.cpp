/*
 * @file DesignDB.cpp
 * @author Keren Zhu
 * @date 06/26/2019
 */

#include "db/DesignDB.h"
#include <unordered_map>

PROJECT_NAMESPACE_BEGIN


bool DesignDB::findRootCkt()                        // 设计找到层次结构根电路
{
    if (this->numCkts() == 0) { return true; }      // 检查设计中是否有电路，如果没有直接返回True

    std::stack<IndexType> sortStack; //< Store the sorted indices here
    std::vector<IntType> visited; //< Mark whether one sub circuit has been visited
    std::stack<IndexType> stack; //< for DFS
                                                    // 这几行定义了几个stack与vector用于DFS和拓扑排序
    // Initialize the variables
    visited.resize(this->numCkts(), 0);             // 初始化visited向量，大小为电路数量，每个元素初始化为0



    //============================ DFS 约束图缺少位置约束 =============================//
    
    for (IndexType startIdx = 0;  startIdx < this->numCkts(); ++startIdx)           // 开始遍历所有电路，进行DFS
    {
        if (visited[startIdx] == 1)                                                 // 下面几行判断当前电路是否访问过，如果访问过则继续循环，否则入栈
        {
            continue;
        }
        stack.push(startIdx);                   
       

       std::stack<IndexType> innerStack; //< Store the sorting of the current DFS
        while (!stack.empty())                                                      
        {
            auto nodeIdx = stack.top();
            stack.pop();
            visited.at(nodeIdx) = 1;                                                // 这几行开始DFS遍历，获取当前电路编号nodeIdx，标记为已访问，并从栈中弹出
            
            
            for (const auto & childNode : _ckts.at(nodeIdx).nodeArray())            // 开始遍历当前电路的所有节点
            {
                if (childNode.isLeaf())
                {
                    continue;
                }
                IndexType graphIdx = childNode.subgraphIdx();                       //============= 叶节点 ==============//
                if (visited[graphIdx] == 1)                                         // 这里的叶节点指在设计的层级结构中最底层的节点，不包含子电路
                {                                                                   // 叶节点通常对应一个实例或基本单元
                    continue;                                                       //==================================//
                }
                stack.push(graphIdx);                                               // 循环内判断子节点是否叶节点，以及对应子电路是否访问过，
                                                                                    // 如果是叶节点或已访问则继续循环，否则将子电路入栈
            }
            innerStack.push(nodeIdx); // Add to the vector (used as stack)          // 将当前电路入innerStack栈，用于后续拓扑排序
        }
        // Add the DFS result to the overall order
        while (!innerStack.empty())
        {
            sortStack.push(innerStack.top());
            innerStack.pop();
        }                                                                           // 这几行从innerstack中弹出元素，并将排序结果push到sortStack，实现当前DFS的拓扑排序
    }

    _rootCkt = sortStack.top();                                                     // 这行获取sortstack栈顶元素作为根电路
    return true;
}

PROJECT_NAMESPACE_END
