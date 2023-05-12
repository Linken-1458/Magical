#ifndef __BOX_H__
#define __BOX_H__

#include <string>
#include <sstream>
#include "global/namespace.h"
#include "XY.h"

PROJECT_NAMESPACE_BEGIN

template<typename T>
class Box
{
public:
    explicit Box() = default;
    explicit Box(const XY<T> &p) : _ll(p), _ur(p) {}
    explicit Box(const XY<T> &ll, const XY<T> &ur) : _ll(ll), _ur(ur) {}
    explicit Box(T xLo, T yLo, T xHi, T yHi) : _ll(xLo, yLo), _ur(xHi, yHi) {}

    // Getters
    T                xLo() const                     { return _ll.x(); }
    T                yLo() const                     { return _ll.y(); }
    T                xHi() const                     { return _ur.x(); }
    T                yHi() const                     { return _ur.y(); }
    T                xLen() const                    { return _ur.x() - _ll.x(); }
    T                yLen() const                    { return _ur.y() - _ll.y(); }
    const XY<T> &    ll() const                      { return _ll; }
    const XY<T> &    ur() const                      { return _ur; }
    const XY<T>      center() const                  { return XY<T>((_ll.x() + _ur.x()) / 2 , (_ll.y() + _ur.y()) / 2); }
    bool             valid() const                   { return _ll.x() <= _ur.x() && _ll.y() <= _ur.y(); }
    T                area() const                    { return (_ur.x() - _ll.x()) * (_ur.y() - _ll.y()); }

    // Setters
    void            set(T xLo, T yLo, T xHi, T yHi)  { _ll.setXY(xLo, yLo); _ur.setXY(xHi, yHi); }
    void            setXLo(T xLo)                    { _ll.setX(xLo); }
    void            setYLo(T yLo)                    { _ll.setY(yLo); }
    void            setXHi(T xHi)                    { _ur.setX(xHi); }
    void            setYHi(T yHi)                    { _ur.setY(yHi); }
    void            join(const XY<T> &pt);

    // String conversion
    operator std::string() const {
        std::ostringstream oss;
        oss <<"Box"<<":";
        oss << "x = [" << xLo() << "," << xHi() << "];";
        oss << "y = [" << yLo() << "," << yHi() << "]:";
        return oss.str();
    }

    bool operator==(const Box<T> &rhs) const { return this->ll() == rhs.ll() && this->ur() == rhs.ur(); }

    std::string toStr() const { return std::string(*this); }

    // Check if this box contains a given (x, y) coordinate     选中此框是否包含给定的（x，y）坐标 
    template <typename U>
    bool            contain(U x, U y) const          { return ((T)x >= _ll.x() && (T)x <= _ur.x() && (T)y >= _ll.y() && (T)y <= _ur.y()); }

    // Check if this box contains a given point     选中此框是否包含给定的点
    template <typename U>
    bool            contain(const XY<U> &p) const    { return ((T)p.x() >= _ll.x() && (T)p.x() <= _ur.x() && (T)p.y() >= _ll.y() && (T)p.y() <= _ur.y()); }

    /// @breif check if this box contain another box    选中此框是否包含另一个框 
    bool contain(const Box<T> &other) const { return this->xLo() <= other.xLo() && this->yLo() <= other.yLo() && this->xHi() >= other.xHi() && this->yHi() >= other.yHi(); }

    T               hpwl() const                     { return xHi() - xLo() + yHi() - yLo(); }

    /// @brief Check if this Box is overlapped with another Box     检查此方框是否与另一个方框重叠 
    bool            overlap(const Box<T> &other) const;
    /// @brief Check if this box is intersected with another box       检查此框是否与另一个框相交
    bool            intersect(const Box<T> &other) const;

    /// @brief Check if the box is cover another box        检查盒子是否覆盖另一个盒子
    bool cover(const Box<T> &other) const { return this->contain(other.ll()) && this->contain(other.ur()); }
    
    /// @brief Make this Box become the union of the original and another box   使此长方体成为原始长方体和另一个长方体的并集
    void          unionBox(const Box<T> &other); 

    /// @brief Return the offset box        返回偏移框
    Box<T>          offsetBox(const XY<T> &origin) const { return Box<T>(xLo() + origin.x(), yLo() + origin.y(), xHi() + origin.x(), yHi() + origin.y()); }
    void            offsetBy(const XY<T> &origin) { setXLo(xLo() + origin.x()); setYLo(yLo() + origin.y()); setXHi(xHi() + origin.x()); setYHi(yHi() + origin.y()); }

    /// @brief Enlarge the boundary of the box by a number      将框的边界放大一个数字
    /// @param The distance for enlarging
    void        enlargeBy(T dis) { setXLo(xLo() - dis); setYLo(yLo() - dis); setXHi(xHi() + dis); setYHi(yHi() + dis); }
private:
    XY<T>  _ll; // Lower left corner of this box        此框的左下角
    XY<T>  _ur; // Upper right corner of this box       此框的右下角
};

/// @brief Extend the box to cover the given point      将方框扩展到给定的点
template <typename T>
inline void Box<T>::join(const XY<T> &pt)
{
    _ll.setX(std::min(pt.x(), _ll.x()));
    _ll.setY(std::min(pt.y(), _ll.y()));
    _ur.setX(std::max(pt.x(), _ur.x()));
    _ur.setY(std::max(pt.y(), _ur.y()));
}

template <typename T>
inline bool Box<T>::overlap(const Box<T> &other) const
{
    if (_ll.x() >= other.xHi() || other.xLo() >= _ur.x())
    {
        return false;
    }
    if (_ll.y() >= other.yHi() || other.yLo() >= _ur.y())
    {
        return false;
    }
    return true;
}

template <typename T>
inline bool Box<T>::intersect(const Box<T> &other) const
{
    if (_ll.x() > other.xHi() || other.xLo() > _ur.x())
    {
        return false;
    }
    if (_ll.y() > other.yHi() || other.yLo() > _ur.y())
    {
        return false;
    }
    return true;
}

template <typename T>
inline void Box<T>::unionBox(const Box<T> &other)
{
    this->setXLo(std::min(this->xLo(), other.xLo()));
    this->setYLo(std::min(this->yLo(), other.yLo()));
    this->setXHi(std::max(this->xHi(), other.xHi()));
    this->setYHi(std::max(this->yHi(), other.yHi()));
}

namespace klib {
    /// @brief determine whether two boxes are parallel overlapped    确定两个方框是否平行重叠 
    template<typename T>
    inline bool boxAreParallelOverlap(const Box<T> &lhs, const Box<T> &rhs)
    {
        if (lhs.xLo() >= rhs.xHi() || rhs.xLo() >= lhs.xHi())
        {
            if (lhs.yLo() < rhs.yHi() && rhs.yLo() < lhs.yHi())
            {
                return true;
            }
        }
        if (lhs.yLo() >= rhs.yHi() || rhs.yLo() >= lhs.yHi())
        {
            if (lhs.xLo() < rhs.xHi() && rhs.xLo() < lhs.xHi())
            {
                return true;
            }
        }
        return false;
    }
    /// @brief compute the parallel run length between two boxes        计算两个盒子之间的平行行程长度
    template <typename T>
    inline T boxParallelRun(const Box<T> &lhs, const Box<T> &rhs)
    {
        if (lhs.xLo() >= rhs.xHi() || rhs.xLo() >= lhs.xHi())
        {
            if (lhs.yLo() < rhs.yHi() && rhs.yLo() < lhs.yHi())
            {
                return std::min(lhs.yHi(), rhs.yHi()) - std::max(lhs.yLo(), rhs.yLo());
            }
        }
        if (lhs.yLo() >= rhs.yHi() || rhs.yLo() >= lhs.yHi())
        {
            if (lhs.xLo() < rhs.xHi() && rhs.xLo() < lhs.xHi())
            {
                return std::min(lhs.xHi(), rhs.xHi()) - std::max(lhs.xLo(), rhs.xLo());
            }
        }
        return 0;
    }
    /// @brief compute the spacing between two boxes    计算两个方框之间的间距 
    template <typename T>
    inline T boxSpacing(const Box<T> &lhs, const Box<T> &rhs)
    {
        if (lhs.xLo() >= rhs.xHi() || rhs.xLo() >= lhs.xHi())
        {
            if (lhs.yLo() < rhs.yHi() && rhs.yLo() < lhs.yHi())
            {
                return std::max(lhs.xLo(), rhs.xLo()) - std::min(lhs.xHi(), rhs.xHi());
            }
            else 
            {
                //  Ifno parallel run and two shapes are disjointed, then count the spacing as the distance between one _ur and one _ll     fno平行运行，两个形状不相交，然后将间距计算为一个_ur和一个_ll之间的距离
                double xDif = std::min(lhs.xHi(), rhs.xHi()) - std::max(lhs.xLo(), rhs.xLo());
                double yDif = std::min(lhs.yHi(), rhs.yHi()) - std::max(lhs.yLo(), rhs.yLo());
                return static_cast<T>(std::hypot(xDif, yDif));
            }
        }
        if (lhs.yLo() >= rhs.yHi() || rhs.yLo() >= lhs.yHi())
        {
            if (lhs.xLo() < rhs.xHi() && rhs.xLo() < lhs.xHi())
            {
                return std::max(lhs.yLo(), rhs.yLo()) - std::min(lhs.yHi(), rhs.yHi());
            }
        }
        // Overlap
        return 0;
    }
    /// @brief compute the width of two box, in y or in x depending on their parallel run overlap     计算两个长方体的宽度，以y或x为单位，取决于它们的平行行程重叠 
    template<typename T>
    inline std::pair<T, T> boxWidth(const Box<T> &lhs, const Box<T> &rhs)
    {
        if (lhs.xLo() >= rhs.xHi() || rhs.xLo() >= lhs.xHi())
        {
            if (lhs.yLo() < rhs.yHi() && rhs.yLo() < lhs.yHi())
            {
                return std::make_pair(lhs.xLen(), rhs.xLen());
            }
        }
        if (lhs.yLo() >= rhs.yHi() || rhs.yLo() >= lhs.yHi())
        {
            if (lhs.xLo() < rhs.xHi() && rhs.xLo() < lhs.xHi())
            {
                return std::make_pair(lhs.yLen(), rhs.yLen());
            }
        }
        return std::make_pair(0, 0);
    }

    /// @brief whether the overlapping of the two boxes are still an rectangle      两个框的重叠是否仍然是一个矩形 
    template<typename T>
    inline bool boxUnionRectangle(const Box<T> &lhs, const Box<T> & rhs)
    {
        // If the union is one of the input, then return true       如果并集是输入之一，则返回true
        if (lhs.cover(rhs) || rhs.cover(lhs))
        {
            return true;
        }
        // Otherwise check the two edges of the union       否则，检查活接头的两个边缘
        if (lhs.xLo() == rhs.xLo() && lhs.xHi() == rhs.xHi())
        {
            if (lhs.yLo() >= rhs.yHi() || rhs.yLo() >= lhs.yHi())
            {
                return true;
            }
        }
        if (lhs.yLo() == rhs.yLo() && lhs.yHi() == rhs.yHi())
        {
            if (lhs.xLo() >=  rhs.xHi() || rhs.xLo() >= lhs.xHi())
            {
                return true;
            }
        }
        return false;
    }
}

PROJECT_NAMESPACE_END


#endif // __BOX_H__
