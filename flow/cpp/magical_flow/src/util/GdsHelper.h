/**
 * @file GdsHelper.h
 * @brief Use utilities of the Gds db
 * @author Keren Zhu
 * @date 02/07/2019
 */

#ifndef ZKUTIL_GDS_HELPER_H_
#define ZKUTIL_GDS_HELPER_H_

#include <limbo/parsers/gdsii/gdsdb/GdsIO.h>
#include <limbo/parsers/gdsii/gdsdb/GdsObjectHelpers.h>
#include "global/global.h"

namespace klib 
{

    /// @brief detailed struct for the functions of processing gds shapes   用于处理gds（晶体管）形状的函数的详细结构
    namespace GetSRefNameActionDetails
    {

        /// @brief default type     默认类型 
        template<typename ObjectType>
        inline void getSName(std::string &name, ObjectType *object)
        {
        }

        /// @brief SREF type        sref和status是编码器中两个重要的参数。sref用来描述编码器输出信号的形式，而status则表示编码器当前的状态。
        template<>
        inline void getSName(std::string &name,  ::GdsParser::GdsDB::GdsCellReference *object)
        {
            name = object->refCell();
        }
    }

    /// @brief aution function object to get the cell reference name        aution函数对象以获取单元格引用名称
    struct GetSRefNameAction
    {
        /// @param A reference to the string to record the name of the sref     对记录sref名称的字符串的引用
        GetSRefNameAction(std::string &name) : _name(name) {}
        template<typename ObjectType>
        void operator()(::GdsParser::GdsRecords::E     numType type, ObjectType* object)
        {
            GetSRefNameActionDetails::getSName(_name, object);
        }

        /// @return a message of action for debug       用于调试的操作消息
        std::string message() const 
        {
            return "GetSRefNameAction";
        }


        std::string &_name; ///< The cell reference name        单元格引用名称
    };
    /// @brief find the top cell of the db      找到数据库的顶部单元格
    /// @param a gds db                         gds数据库
    /// @return the name of the top cell        顶部单元格的名称 
    inline std::string topCell(const ::GdsParser::GdsDB::GdsDB & db)
    {
        // Whether each cell is found as the subcell of the other       是否发现每个单元都是另一个单元的子单元
        std::map<std::string, bool> nameFound;
        // Iterate all the cells and record their names                 迭代所有单元格并记录它们的名称
        // Add reversely                                                反向添加
        for (int idx = db.cells().size() - 1; idx >= 0; idx--)
        {
            nameFound[db.cells().at(idx).name()] = false;
        }
        for (auto &cell : db.cells())
        {
            for (auto obj : cell.objects())
            {
                std::string name = "";
                ::GdsParser::GdsDB::GdsObjectHelpers()(obj.first, obj.second, GetSRefNameAction(name));
                if (name != "")
                {
                    nameFound[name] = true;
                }
            }
        }
        // Return the name that was not included in the other cells     返回未包含在其他单元格中的名称
        for (auto pair : nameFound)
        {
            if (pair.first != "" &&  !pair.second)
            {
                return pair.first;
            }
        }

        AssertMsg(0 ,"Reading Gds::%s: all cells are referenced in other cells! \n", __FUNCTION__);
        return "";
    }
}
#endif //ZKUTIL_GDS_HELPER_H_
