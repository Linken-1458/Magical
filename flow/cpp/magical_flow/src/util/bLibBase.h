#ifndef _B_LIB_BASE_H_
#define _B_LIB_BASE_H_

// ============================================================== //
//   Library for some common functions
//
//   Author        : Bei Yu
//   Last update   : 11/2014
// ============================================================== //

#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>
#include <cstdio>
#include <cmath>            // ceilf(), floorf()
#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include <sys/resource.h>   // timer()
#include <sys/time.h>       // timer()

namespace bLib
{
    // =================================================================
    //                  Parse string    分析字符串
    // =================================================================
    // trim left space of a string      修剪字符串的左空格
    inline std::string & ltrim(std::string &s)
    {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
        return s;
    }

    // trim right space of a string     修剪字符串的右空格
    inline std::string & rtrim(std::string &s)
    {
        s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
        return s;
    }

    // trim spaces from both ends       从两端修剪空间
    inline std::string & trim(std::string &s)
    {
        return ltrim(rtrim(s));
    }

    // given path in str_path, output the design name       在str_path中给定路径，输出设计名称
    // for example: with input "path1/path2/sparc_exu_alu", output "sparc_exu_alu"
    inline std::string parsePath2Name(std::string str_path)
    {
        size_t pos = str_path.rfind('/');
        if (pos == std::string::npos) return str_path;

        return str_path.substr(pos+1);
    }

    // parse filename suffix        解析文件名后缀
    inline std::string parseSuffix(std::string str)
    {
        size_t pos = str.rfind('.');
        if (pos == std::string::npos) return str;
        return str.substr(pos+1);
    }

    // trim filename suffix         修剪文件名后缀
    inline std::string trimSuffix(std::string str)
    {
        size_t pos = str.rfind('.');
        return str.substr(0, pos);
    }


    // ============================================================
    //                   Read File Functions  读取文件函数   
    // ============================================================
    // check whether file exists
    inline bool isFileExist(const char* filename)
    {
        std::ifstream infile(filename);
        return infile.good();
    }

    // read from ifstream in, search target1 & target2,                         从ifstream中读取，搜索target1&target2，
    // if found, return true, and the whole line is stored in "str_line"        如果找到，则返回true，整行存储在“str_line”中
    // if reach EOF, return false                                               如果达到EOF，则返回false
    inline bool readSearchUntil(std::ifstream& in, std::string & str_line, const std::string target1, const std::string target2="")
    {
        while (!in.eof())
        {
            getline( in, str_line );  if (in.eof())          return false;
            if (std::string::npos != str_line.find(target1)) return true;
            if (target2.length() <= 0) continue;
            if (std::string::npos != str_line.find(target2)) return true;
        } // while
        return false;
    }

    // new version of readSearchUntil() to support arbitrary number targets     新版readSearchUntil（）支持任意数量的目标
    inline bool readSearchUntil(std::ifstream& in, std::string& str_line, const std::vector<std::string> targets)
    {
        while (!in.eof())
        {
            getline( in, str_line );  if (in.eof())          return false;
            for (uint32_t i=0; i<targets.size(); i++)
            {
                std::string target = targets[i];
                if (std::string::npos != str_line.find(target)) return true;
            }
        } // while
        return false;
    }


    // ============================================================
    //               Numerical functions    数字函数
    // ============================================================
    inline bool isInteger(float fvalue)
    {
        if ( ceilf (fvalue) != fvalue ) return false;
        if ( floorf(fvalue) != fvalue ) return false;
        return true;
    }


    // ==============================================
    //             timer functions     时间函数
    // ==============================================
    inline double timerSelf()
    {
        rusage r;
        getrusage(RUSAGE_SELF, &r);
        return (double)(r.ru_utime.tv_sec+r.ru_utime.tv_usec/(double)1000000);
    }

    inline double timerChild()
    {
        rusage r;
        getrusage(RUSAGE_CHILDREN, &r);
        return (double)(r.ru_utime.tv_sec+r.ru_utime.tv_usec/(double)1000000);
    }

    inline double timer()
    {
        return timerSelf() + timerChild();
    }


    // ==============================================
    //            STL extension     STL 扩展库
    // ==============================================
    // erase vec[id], can NOT keey the previous order in rest of the vector     擦除vec[id]，不能保留容器其余部分的前一顺序
    template<class Type>
    inline bool erase_fast(std::vector<Type>& vec, int id)
    {
        int size = vec.size();
        if (id<0 || id>=size) return false;
        vec[id] = vec[size-1];  vec.resize(size-1);
        return true;
    }

    template<class Type>
    inline bool erase_fast(std::vector<Type>& vec, typename std::vector<Type>::iterator itr)
    {
        int size = vec.size();
        if (vec.end() == itr) return false;
        (*itr) = vec.back();  vec.resize(size-1);
        return true;
    }
}

/*
// ==== Implementation Log:
//
// 11/2014: new function erase_fast(std::vector<>&, int)
// 10/2014: new function readSearchUntil() to support arbitrary number of targets
// 08/2014: new function parseSuffix()
// 06/2014: new function readSearchUntil(), added target2
// 06/2014: add function trimSuffix()
// 05/2014: add namespace bLib, discard class
// 05/2014: new function isFileExist()
// 05/2014: check fstream with is_open() function
//
*/


#endif


