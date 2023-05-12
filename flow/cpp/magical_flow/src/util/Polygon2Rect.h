/**
 * @file Polygon2Rect.h
 * @author Keren Zhu
 * @date 11/06/2018
 */

#ifndef KLIB_POLYGON2RECT_H_
#define KLIB_POLYGON2RECT_H_

#include "Box.h"
#include <limbo/geometry/Polygon2Rectangle.h>


//using namespace limbo::geometry;


namespace limbo { namespace geometry {

/**
 * @brief if your point class setting is different from that in the default point_traits, please create a specialization
 *
 * The specialization should be in the same scope as the original template struct 
 */
template <typename T>
struct point_traits<PROJECT_NAMESPACE::XY<T>>
{
    /// point type 
    typedef PROJECT_NAMESPACE::XY<T> point_type;
    /// coordinate type 
    typedef T coordinate_type;

    /**
     * @brief access coordinates 
     * @param point a data point 
     * @param orient orientation can be HORIZONTAL(x) or VERTICAL(y)
     * @return x or y coordinate
     */
    static coordinate_type get(const point_type& point, orientation_2d orient) 
    {
        if (orient == HORIZONTAL) return point.x();
        else if (orient == VERTICAL) return point.y();
        else assert(0);
    }
    /**
     * @brief set coordinates       设置坐标 
     * @param point a data point    数据点
     * @param orient orientation can be HORIZONTAL(x) or VERTICAL(y)        方向可以是水平（x）或垂直（y）
     * @param value data value       数据值 
     */
    static void set(point_type& point, orientation_2d orient, coordinate_type value) 
    {
        if (orient == HORIZONTAL) return point.setX(value);
        else if (orient == VERTICAL) return point.setY(value);
        else assert(0);
    }
    /**
     * @brief construct a point object      构造点对象 
     * @param x x coordinate            X坐标
     * @param y y coordinate            y坐标
     * @return the point object         点对象
     */
    static point_type construct(coordinate_type x, coordinate_type y) 
    {
        point_type p; 
        p.setX(x); p.setY(y);
        return p;
    }
};

}}


namespace limbo { namespace geometry {

/**
 * @brief if your rectangle class setting is different from that in the default point_traits, please create a specialization
 *  如果你的矩形类设置与默认的point_traits中的不同，请创建一个专门化
 */
template <typename T>
struct rectangle_traits<PROJECT_NAMESPACE::Box<T>>
{
    /// rectangle type      矩形类型
    typedef PROJECT_NAMESPACE::Box<T> rectangle_type;
    /// coordinate type         坐标类型 
    typedef T coordinate_type;

    /**
     * @brief access coordinates        访问坐标
     * @param rect a rectangle object       矩形对象 
     * @param dir can be LEFT (xl), BOTTOM (yl), RIGHT (xh) or TOP (yh)     可以是LEFT（xl）、BOTTOM（yl）、RIGHT（xh）或TOP（yh）
     * @return coordinate       坐标
     */
    static coordinate_type get(const rectangle_type& rect, direction_2d dir) 
    {
        switch (dir)
        {
            case LEFT: return rect.xLo();
            case BOTTOM: return rect.yLo();
            case RIGHT: return rect.xHi();
            case TOP: return rect.yHi();
            default: assert(0);
        }
    }
    /**
     * @brief set coordinates           访问坐标
     * @param rect a rectangle object       矩形对象 
     * @param dir can be LEFT (xl), BOTTOM (yl), RIGHT (xh) or TOP (yh)     可以是LEFT（xl）、BOTTOM（yl）、RIGHT（xh）或TOP（yh）
     * @param value coordinate value        坐标值
     */
    static void set(rectangle_type& rect, direction_2d dir, coordinate_type value) 
    {
        switch (dir)
        {
            case LEFT: rect.setXLo(value); break;
            case BOTTOM: rect.setYLo(value); break;
            case RIGHT: rect.setXHi(value); break;
            case TOP: rect.YHi(value); break;
            default: assert(0);
        }
    }
    /**
     * @brief construct rectangle       构造矩形
     * @param xl, yl, xh, yh coordinates of rectangle   矩形坐标     
     * @return a rectangle object       矩形对象 
     */
    static rectangle_type construct(coordinate_type xl, coordinate_type yl, coordinate_type xh, coordinate_type yh) 
    {
        rectangle_type rect; 
        rect.setXLo(xl); rect.setYLo(yl);
        rect.setXHi(xh); rect.setYHi(yh);
        return rect;
    }
};

}}

namespace klib
{
    template<typename T>
    inline bool convertPolygon2Rects(const std::vector<PROJECT_NAMESPACE::XY<T>> &pts, std::vector<PROJECT_NAMESPACE::Box<T>> &rects)
    {
        typedef typename PROJECT_NAMESPACE::XY<T> PtType;
        typedef typename PROJECT_NAMESPACE::Box<T> RectType;

        limbo::geometry::Polygon2Rectangle<std::vector<PtType>, std::vector<RectType>> p2r(rects, pts.begin(), pts.end(), limbo::geometry::HOR_VER_SLICING);
        return p2r();
    }
}

#endif ///KLIB_POLYGON2RECT_H_
