//
// Created by Keren on 04/17/2018
//

#ifndef ZKUTIL_VECTOR2D_H_
#define ZKUTIL_VECTOR2D_H_

#include <vector>
#include <initializer_list>
#include "global/namespace.h"
#include "global/type.h"
#include "XY.h"

PROJECT_NAMESPACE_BEGIN

/// ================================================================================ 
/// Vector map class
/// A templete for 2D vector using flat vector
/// This is a Y-dominate implementation, that is elements in the
/// same column (with the same Y) are in a piece of consecutive memory.
/// Thus, when iterate through this 2D vector using a two-level for loop,
/// the following pattern is better from the memory/cache hit prespective
/// for (x ...)
/// {
///     for (y ....)
///     {
///         ....
///     }
/// }
///
/// Update: 10/26/2018 add option to yMajor
///矢量映射类
///使用平面向量的二维向量模板
///这是一个以Y为主的实现，即
///相同的列（具有相同的Y）在一段连续存储器中。
///因此当使用两级for循环迭代通过该2D向量时，
///从内存/缓存命中率的角度来看，以下模式更好
///对于（x…）
/// {
///对于（y….）
/// {
/// ....
/// }
/// }
///
/// ================================================================================ 
template <typename T>
class Vector2D 
{
    public:
        /// Class for initializer_list type
        enum class InitListType : Byte 
        {
            XMajor,
            yMajor
        };

    public:
        explicit Vector2D() = default;
        explicit Vector2D(IndexType xSize, IndexType ySize) : _vec(xSize * ySize), _xSize(xSize), _ySize(ySize) {}
        explicit Vector2D(IndexType xSize, IndexType ySize, const T &v) : _vec(xSize, ySize, v), _xSize(xSize), _ySize(ySize) {}
        explicit Vector2D(IndexType xSize, IndexType ySize, InitListType t, std::initializer_list<T> l);

        void                                      clear()                                                { _vec.clear(); _xSize = _ySize = 0; }
        void                                      resize(IndexType xSize, IndexType ySize)               { _vec.resize(xSize * ySize); _xSize = xSize; _ySize = ySize; }
        void                                      resize(IndexType xSize, IndexType ySize, const T &v)   { _vec.resize(xSize * ySize, v); _xSize = xSize; _ySize = ySize; }
        void                                      setType(InitListType type) { _type = type; }
        
        IndexType                                 xSize() const                                          { return _xSize; }
        IndexType                                 ySize() const                                          { return _ySize; }
        IndexType                                 size() const                                           { return _vec.size(); }
        InitListType                              type() const                                           { return _type; }

        const T &                                 at(IndexType x, IndexType y) const                     { return _vec.at(xyToIndex(x, y)); }
        T &                                       at(IndexType x, IndexType y)                           { return _vec.at(xyToIndex(x, y)); }
        const T &                                 at(XY<IndexType> xy) const                             { return _vec.at(xyToIndex(xy)); }
        T &                                       at(XY<IndexType> xy)                                   { return _vec.at(xyToIndex(xy)); }
        const T &                                 atIdx(IndexType idx) const                             { return _vec.at(idx); }
        T &                                       atIdx(IndexType idx)                                   { return _vec.at(idx); }

        /// Iterator        迭代程序
        typename std::vector<T>::iterator         begin()                                                { return _vec.begin(); }
        typename std::vector<T>::const_iterator   begin() const                                          { return _vec.begin(); }
        typename std::vector<T>::iterator         end()                                                  { return _vec.end(); }
        typename std::vector<T>::const_iterator   end() const                                            { return _vec.end(); }

        /// Conversion      转变 
        IndexType                                 xyToIndex(IndexType x, IndexType y) const;
        IndexType                                 xyToIndex(XY<IndexType> xy) const                      { return xyToIndex(xy.x(), xy.y()); }
        IndexType                                 idxToX(IndexType idx) const;
        IndexType                                 idxToY(IndexType idx) const;
        XY<IndexType>                             idxToXY(IndexType idx) const                           { return XY<IndexType>(idxToX(idx), idxToY(idx)); }

    private:
        std::vector<T> _vec;
        IndexType      _xSize = 0;
        IndexType      _ySize = 0;
        InitListType   _type = InitListType::XMajor;
};

template<typename T>
inline IndexType Vector2D<T>::xyToIndex(IndexType x, IndexType y) const 
{
    if (_type == InitListType::XMajor)
    {
        return _ySize * x + y;
    }
    else 
    {
        /// Y major
        return _xSize *y + x;
    }
}

template<typename T>
inline IndexType Vector2D<T>::idxToX(IndexType idx) const 
{
    if (_type == InitListType::XMajor)
    {
        return idx / _ySize;
    }
    else
    {
        return idx % _xSize;
    }
}

template<typename T>
inline IndexType Vector2D<T>::idxToY(IndexType idx) const 
{
    if (_type == InitListType::XMajor)
    {
        return idx % _ySize;
    }
    else
    {
        return idx / _xSize;
    }
}
/// Ctor using initializer_list     使用initializer_list的Ctor
template <typename T>
inline Vector2D<T>::Vector2D(IndexType xSize, IndexType ySize, InitListType t, std::initializer_list<T> l)
    : _vec(xSize * ySize), _xSize(xSize), _ySize(ySize) 
        {
        Assert(l.size == size());
        if (t == InitListType::XMajor) 
        {
           /// Data in Vector2D is in X-major manner        Vector2D中的数据采用X主方式 
           /// Just copy the initializer_list to the _vec       只需将initializer_list复制到_vec
            std::copy(l.begin, l.end(), _vec.begin());
        }
        else 
        { // t == InitiListType::YMajor     t==初始化列表类型：：YMajor
            for (IndexType idx = 0; idx < size(); ++idx) 
            {
                IndexType x = idx % _xSize;
                IndexType y = idx / _xSize;
                at(x, y) = *(l.begin() + idx);
            }
        }
}

PROJECT_NAMESPACE_END

#endif //ZKUTIL_VECTOR2D_H_
