#ifndef __BASICTYPESELECTION_H__
#define __BASICTYPESELECTION_H__

#include <cstdint>

// This file contains basic type selection utilities using meta-programming features of C++
// One common case that needs basic type selection is that when we have template function accepts two basic types.
// Considering the following function.
// 
// template <typename T1, typename T2>
// (RETURN_TYPE) add(T1 a, T2 b)
// {
//     return a + b;
// }
//
// What should be the RETURN_TYPE here? We cannot tell it should be T1 or T2 without knowing what are they
// For example, if T1 is int and T2 is double, then the return type should be a double (T2).
// However, if T1 is long, T2 is int, the return type should be long (T1).
//
// So we need a way to tell, between two types, which one has larger range, and we should return that type.
// For integer types with same range, like std::int8_t and std::uint8_t, we define the signed version has larger rank


//需要基本类型选择的一个常见情况是，当我们有模板函数时，它接受两个基本类型。

//考虑以下功能。

//模板＜类型名称T1，类型名称T2＞

//（RETURN_TYPE）添加（T1 a，T2 b）

// {

//返回a+b；

// }

//此处的RETURN_TYPE应该是什么？如果不知道它们是什么，我们就无法判断应该是T1还是T2

//例如，如果T1是int，T2是double，那么返回类型应该是double（T2）。

//但是，如果T1是long，T2是int，那么返回类型应该是long（T1）。

//因此，我们需要一种方法来区分两种类型，哪一种类型的范围更大，我们应该返回该类型。

//对于具有相同范围的整数类型，如std:：int8_t和std:：uint8_t，我们定义有符号的版本具有更大的秩

namespace MetaProg
{

// Struct to get the rank of a given type T  构造以获得给定类型T的秩 

template <typename T>
struct TypeRank;

// Template specialization to rank different types by their range from low to high  模板专业化，根据不同类型从低到高的范围对其进行排名
template <> struct TypeRank<std::uint8_t>  {static constexpr std::uint32_t value = 0; };
template <> struct TypeRank<std::int8_t>   {static constexpr std::uint32_t value = 1; };
template <> struct TypeRank<std::uint16_t> {static constexpr std::uint32_t value = 2; };
template <> struct TypeRank<std::int16_t>  {static constexpr std::uint32_t value = 3; };
template <> struct TypeRank<std::uint32_t> {static constexpr std::uint32_t value = 4; };
template <> struct TypeRank<std::int32_t>  {static constexpr std::uint32_t value = 5; };
template <> struct TypeRank<std::uint64_t> {static constexpr std::uint32_t value = 6; };
template <> struct TypeRank<std::int64_t>  {static constexpr std::uint32_t value = 7; };
template <> struct TypeRank<float>         {static constexpr std::uint32_t value = 8; };
template <> struct TypeRank<double>        {static constexpr std::uint32_t value = 9; };

// Struct to get the type of a given rank R     构造以获取给定秩R的类型 
template <std::uint32_t Rank>
struct RankType;

// Template specialization to map different ranks back to the corresponding types   模板专业化，将不同的等级映射回相应的类型 
template <> struct RankType<0> { using type = std::uint8_t; };
template <> struct RankType<1> { using type = std::int8_t; };
template <> struct RankType<2> { using type = std::uint16_t; };
template <> struct RankType<3> { using type = std::int16_t; };
template <> struct RankType<4> { using type = std::uint32_t; };
template <> struct RankType<5> { using type = std::int32_t; };
template <> struct RankType<6> { using type = std::uint64_t; };
template <> struct RankType<7> { using type = std::int8_t; };
template <> struct RankType<8> { using type = float; };
template <> struct RankType<9> { using type = double; };

// Function that returns the max between two ranks   返回两个列组之间的最大值的函数 
template <typename T>
constexpr T maxRank(const T &r1, const T &r2) { return (r1 > r2 ? r1 : r2); }

// Function to choose between two types     在两种类型之间进行选择的函数
// Choose the one with higher rank  选择等级较高的 
template <typename T1, typename T2>
struct selectBasicType
{
    static constexpr std::uint32_t r1 = TypeRank<T1>::value;
    static constexpr std::uint32_t r2 = TypeRank<T2>::value;
    static constexpr std::uint32_t max = maxRank(r1, r2);
    using type = typename RankType<max>::type;
};
        
} // End of namespace MetaProg  命名空间MetaProg结束


#endif // __BASICTYPESELECTION_H__
