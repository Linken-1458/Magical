/**
 * @file GdsWriter.h
 * @brief Writing GDSII layout file 
 * @author Keren Zhu
 * @date 07/08/2019
 */

#ifndef MAGICAL_FLOW_GDS_WRITER_H_
#define MAGICAL_FLOW_GDS_WRITER_H_

#include "db/DesignDB.h"
#include "db/TechDB.h"
#include "util/GdsHelper.h"

PROJECT_NAMESPACE_BEGIN

class GdsWriter
{
    // Point type for the gds db        晶体管数据库的点类型
    typedef boost::polygon::point_data<IntType> point_type;
    public:
        /// @brief constructor
        /// @param first: a design database             设计数据库 
        /// @param second: a technology database        技术数据库 
        explicit GdsWriter(DesignDB &designDB, TechDB &techDB) : _designDB(designDB), _techDB(techDB) {} 
        /// @brief write the layout of a circuit into GDSII     将电路布局写入GDSII
        /// @param first: the index of circuit graph        电路图索引
        /// @param second: the output file name         输出文件名
        void writeGdsLayout(IndexType cktIdx, const std::string &filename);
    private:
        /// @brief add CktGraph to the gds DB       将CktGraph添加到gds数据库 
        /// @param the index of CktGraph        CktGraph索引
        void addCktGraph(IndexType cktGraphIdx);        
        /// @brief add box to the cell      将方框添加到单元格
        /// @param reference to the cell   对单元格的引用     
        /// @param rectangle
        /// @param db layer
        /// @param datatype
        void addRect2Cell(::GdsParser::GdsDB::GdsCell &gdsCell, const Box<LocType> &rect, IndexType dbLayer, IntType datatype);
        /// @brief add text to the cell     向单元格添加文本
        /// @param reference to the cell    引用单元格
        /// @param coordinate of the text   文本坐标
        /// @param db layer                 数据库层
        /// @param the string for the text
        void addText2Cell(::GdsParser::GdsDB::GdsCell &gdsCell, const XY<LocType> &coord, IndexType dbLayer, const std::string &str);
        /// @brief add a cellreference to a gdscell     为晶体管单元添加单元引用
        /// @param the reference to the gdscell     对晶体管单元的引用
        /// @param the CktNode want to add      添加电流节点
        void addCellRef2Cell(::GdsParser::GdsDB::GdsCell &gdsCell, CktNode &node);
        /// @brief add text to the cell     向单元格添加文本
        /// @brief convert XY type to point_type        将XY类型转换为point_type
        point_type convertXY(const XY<LocType> &pt)
        {
            return point_type(pt.x(), pt.y());
        }
    private:
        ::GdsParser::GdsDB::GdsDB _gdsDB; ///< The database for the GDS     GDS的数据库     
        DesignDB &_designDB; ///< The design database       设计数据库
        TechDB &_techDB; ///< The technology database       技术数据库
};

inline void GdsWriter::writeGdsLayout(IndexType cktIdx, const string &filename)
{
    // Config header and units      配置头和单元
    _gdsDB.cells().clear();
    _gdsDB.setUnit(_techDB.units().dbuUU());
    _gdsDB.setPrecision(_techDB.units().dbuM());
    _gdsDB.setHeader(_techDB.units().gdsHeader());

    // Add layouts          添加布局 
    this->addCktGraph(cktIdx);

    // Write out        
    ::GdsParser::GdsDB::GdsWriter gw (_gdsDB);
    gw(filename.c_str());
    INF("Flow::GdsWriter:: Write circuit %s layout to %s \n", _designDB.subCkt(cktIdx).name().c_str(), filename.c_str());
}

inline void GdsWriter::addCktGraph(IndexType cktGraphIdx)
{
    auto &cktGraph = _designDB.subCkt(cktGraphIdx); // CktGraph     电流关系图
    // Check if the cell has been added to the gdsDB        检查单元格是否已添加到gds DB
    ::GdsParser::GdsDB::GdsCell const* getCellResult = _gdsDB.getCell(cktGraph.name());
    if (getCellResult != NULL)
    {
        // If the return is not NULL, then the cell has already been added to the gdsDB     如果返回值不是NULL，那么单元格已经被添加到gdsDB中
        // No need to add again     不需要再添加
        return;
    }
    auto &gdsCell = _gdsDB.addCell(cktGraph.name()); // GdsCell     晶体管单元
    
    // Add layout       添加布局
    auto cktLayout = cktGraph.layout(); // Layout
    for (IndexType layerIdx = 0; layerIdx < cktLayout.numLayers(); ++layerIdx)
    {
        for (IndexType rectIdx = 0; rectIdx < cktLayout.numRects(layerIdx); ++rectIdx)
        {
            this->addRect2Cell(gdsCell, cktLayout.rect(layerIdx, rectIdx).rect(), layerIdx, cktLayout.rect(layerIdx, rectIdx).datatype()); // FIXME For >M6 layer, need to use datatype=40
        }
        for (IndexType textIdx = 0; textIdx < cktLayout.numTexts(layerIdx); ++textIdx)
        {
            this->addText2Cell(gdsCell, cktLayout.text(layerIdx, textIdx).coord(), layerIdx, cktLayout.text(layerIdx, textIdx).text());
        }
    }

    // Add cell reference       添加单元格引用
    for (IndexType nodeIdx = 0; nodeIdx < cktGraph.numNodes(); ++nodeIdx)
    {
        auto & node = cktGraph.node(nodeIdx);
        if (node.isLeaf())
        {
            continue;
        }
        // If it is not leaf, add a reference cell and recursively add the cell     如果不是leaf，则添加一个引用单元格并递归添加该单元格
        // this->addCktGraph(node.subgraphIdx());
        // this->addCellRef2Cell(gdsCell, node);
    }
}

inline void GdsWriter::addRect2Cell(::GdsParser::GdsDB::GdsCell &gdsCell, const Box<LocType> &rect, IndexType dbLayer, IntType datatype)
{
    IntType pdkLayer = static_cast<IntType>(_techDB.dbLayerToPdk(dbLayer));
    std::vector<point_type> pts;
    pts.emplace_back(this->convertXY(XY<LocType>(rect.xLo(), rect.yLo())));
    pts.emplace_back(this->convertXY(XY<LocType>(rect.xLo(), rect.yHi())));
    pts.emplace_back(this->convertXY(XY<LocType>(rect.xHi(), rect.yHi())));
    pts.emplace_back(this->convertXY(XY<LocType>(rect.xHi(), rect.yLo())));
    pts.emplace_back(this->convertXY(XY<LocType>(rect.xLo(), rect.yLo())));

    // Add to the cells     添加到单元
    gdsCell.addPolygon(pdkLayer, datatype, pts);

}

inline void GdsWriter::addText2Cell(::GdsParser::GdsDB::GdsCell &gdsCell, const XY<LocType> &coord, IndexType dbLayer, const std::string &str)
{
    IntType pdkLayer = static_cast<IntType>(_techDB.dbLayerToPdk(dbLayer));
    // Trick: set datatype to int max so that the GDS writter will be forced to skip the keyword      技巧:设置datatype为int max，这样GDS写入器将被迫跳过关键字
    gdsCell.addText(pdkLayer, std::numeric_limits<int>::max(), 0, str, convertXY(coord), std::numeric_limits<int>::max(), 5, 0, 0.2, 0);
}

inline void GdsWriter::addCellRef2Cell(GdsParser::GdsDB::GdsCell &gdsCell, CktNode &node)
{
    double angle = 0; 
    bool flip = false;          // flip 翻转
    if (node.orient() == OriType::N)
    {
        angle = 0;
        flip = false;
    }
    else
    {
        angle = 0;
        flip = true;
        // TODO:
        // AssertMsg(false, "GdsWriter::addCellRef2Cell. TODO: Not yet implemented. Only N orientation is supported right now. \n");
    }
    int strans;
    if (flip)
    {
        strans = 32768;
    }
    else
    {
        strans = 0;
    }
    // 1. reference cell name 2. angle 3. magnification 4. strans       1. 引用单元名称 2.角 3.放大 4.strans
    gdsCell.addCellReference(node.refName(), this->convertXY(node.offset()), angle, 1, strans);
}


namespace WRITER
{
    /// @brief write the layout for circuit to GDSII        将电路布局写入GDSII
    /// @param first: circuit graph index           电路图索引
    /// @param second: output file name             输出文件名
    /// @param third: design database               设计数据库
    /// @param fourth: technology database          技术数据库
    inline void writeGdsLayout(IndexType cktIdx, const std::string &filename, DesignDB &designDB, TechDB &techDB)
    {
        GdsWriter(designDB, techDB).writeGdsLayout(cktIdx, filename);
    }
}
PROJECT_NAMESPACE_END
#endif //MAGICAL_FLOW_GDS_WRITER_H_
