#ifndef _B_LIB_ILP_H_
#define _B_LIB_ILP_H_

// ============================================================== //
//   Library for ILP applications      ILP应用程序库 
//   Author        : Bei Yu
//   Last update   : 12/2014
// ============================================================== //

#include <algorithm>
#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <cmath>

namespace bLib
{
    // Input  : lp file named by l_inIlpName        由l_inIlpName命名的lp文件
    // Output : standard output format in file named by l_outIlpName        以l_outIlpName命名的文件中的标准输出格式
    const  std::string  l_inIlpName  = "problem.lp";  // ILP input file name
    const  std::string  l_tmpIlpName = "tmp.lp";      // internal ILP output file name      内部ILP输出文件名 
    const  std::string  l_outIlpName = "problem.sol"; // ILP output file name
    inline std::string  getInIlpName()        { return l_inIlpName;  }
    inline std::string  getOutIlpName()       { return l_outIlpName; }


    // =====================================================
    //               for GUROBI solver  适用于GUROBI解算器 
    // =====================================================
    // max_time : maximum allowed runtime (in seconds)  max_time：允许的最大运行时间（秒） 
    // gap_abs  : the gap value when terminate ILP      gap_abs：终止ILP时的间隙值 
    // threads  : thread #                              threads ：思路
    inline void prepareGurobi(int max_time=-1, double gap_abs=0.0, int threads=-1)
    //{{{
    {
        // prepare rungrb.csh       准备运行GUROBI解算器
        std::ifstream cshtest("rungrb.csh");        // 读文件操作（std::ifstream)
        if (!cshtest.good())
        {
            std::ofstream outcsh("rungrb.csh");     // 定义ofstream对象outcsh("rungrb.csh")
            outcsh<<"#!/bin/csh -f"<<std::endl;     // 如果第一行以#开始, 系统会用Cshell执行script.等同于#!/bin/csh
            outcsh<<"gurobi.sh ./rungrb.grb"<<std::endl;
            outcsh.close();
        }
        cshtest.close();

        // prepare rungrb.grb       准备运行GUROBI解算器
        std::ifstream grbtest("rungrb.grb");
        if (!grbtest.good())
        {
            std::ofstream outgrb("rungrb.grb");
            outgrb<<"from gurobipy import *"<<std::endl;
            outgrb<<"from gurobipy import *"<<std::endl;
            outgrb<<"m = read(\"" << l_inIlpName<<"\");"<<std::endl;
            if (max_time>0)          outgrb<<"m.setParam(\'TimeLimit\', "<<max_time<<");"<<std::endl;
            if (threads >0)          outgrb<<"m.setParam(\'Threads\',   "<<threads <<");"<<std::endl;
            if (fabs(gap_abs)>1e-10) outgrb<<"m.setParam(\'MIPGapAbs\', "<<gap_abs <<");"<<std::endl;
            outgrb<<"m.optimize();"<<std::endl;
            outgrb<<"m.write(\"" << l_outIlpName << "\");"<<std::endl;
            outgrb<<"quit"<<std::endl;
            outgrb.close();
        }
        grbtest.close();

        system("chmod a+x rungrb.csh rungrb.grb");
    }
    //}}}

    inline bool solveGurobi(bool sim_out)
    {
        std::ifstream test_file(l_inIlpName.c_str());
        if(! test_file.good()) return false; 

        char command[100];
        if (false == sim_out) sprintf(command, "./rungrb.csh");
        else                  sprintf (command, "./rungrb.csh| grep ^yb ");
        system(command);  return true;
    }

    inline void cleanupGurobi()
    {
        system("rm -rf rungrb.csh rungrb.grb gurobi.log");
        char command[100];
        sprintf(command, "rm -rf %s %s", l_inIlpName.c_str(), l_outIlpName.c_str());
        system(command);
    }


    // =====================================================
    //               for CPLEX solver       用于CPLEX解算器
    // =====================================================
    inline void prepareCplex()
    //{{{
    {
        // prepare runcpl.csh      准备runcpl.csh
        // call runcpl.cplex to solve ILP;      调用runcpl.cplex来解决ILP
        // call formatCPLEX.py to generate standard format result       调用formatCPLEX.py生成标准格式结果
        std::ifstream cshtest("runcpl.csh");
        if (!cshtest.good())
        {
            std::ofstream outcsh("runcpl.csh");
            outcsh<<"#!/bin/csh -f"<<std::endl<<std::endl;
            outcsh<<"source /import/pdtools11/external/CPLEX-12.2.0.0/setup.csh"<<std::endl;
            outcsh<<"rm -rf "<<l_tmpIlpName<<std::endl;
            outcsh<<"rm -rf "<<l_outIlpName<<std::endl;
            outcsh<<"cplex < runcpl.cplex"<<std::endl;
            outcsh<<"python formatCPLEX.py > "<<l_outIlpName<<std::endl;
            outcsh.close();
        }
        cshtest.close();

        // prepare runcpl.cplex
        // core file to run ILP solver CPLEX
        std::ifstream cpltest("runcpl.cplex");
        if (!cpltest.good())
        {
            std::ofstream outcpl("runcpl.cplex");
            outcpl<<"read "<<l_inIlpName<<std::endl;
            outcpl<<"optimize"<<std::endl;
            outcpl<<"write "<<l_tmpIlpName<<std::endl;
            outcpl<<"quit"<<std::endl;
            outcpl.close();
        }
        cpltest.close();

        // prepare formatCPLEX.py
        // python codes transferring l_tmpIlpName file to standard l_outIlpName file
        std::ifstream pytest("formatCPLEX.py");
        if (!pytest.good())
        {
            std::ofstream outpy("formatCPLEX.py");
            outpy<<"import sys"<<std::endl;
            outpy<<"from xml.dom import minidom"<<std::endl;
            outpy<<"xmldoc = minidom.parse('" << l_tmpIlpName << "')"<<std::endl;
            outpy<<"itemlist = xmldoc.getElementsByTagName('variable')"<<std::endl;
            outpy<<"for s in itemlist:"<<std::endl;
            outpy<<"print '%s  %s' % (s.attributes['name'].value, s.attributes['value'].value)"<<std::endl;
            outpy.close();
        }
        pytest.close();

        system("chmod a+x runcpl.csh runcpl.cplex");
    }
    //}}}


    inline bool solveCplex(bool sim_out)
    {
        std::ifstream test_file(l_inIlpName.c_str());
        if(! test_file.good()) return false; 

        char command[100];
        if (false == sim_out) sprintf(command, "./runcpl.csh");
        else                  sprintf(command, "./runcpl.csh | grep ^yb ");
        system(command);  return true;
    }

    inline void cleanupCplex()
    {
        system("rm -rf runcpl.csh runcpl.cplex formatCPLEX.py");
        char command[100];
        sprintf(command, "rm -rf %s %s %s", l_inIlpName.c_str(), l_tmpIlpName.c_str(), l_outIlpName.c_str());
        system(command);
    }


    // =====================================================
    //                 for CBC solver   用于CBC解算器
    // =====================================================
    // NOTE: CURRENTLY CBC OUTPUT FILE IS NOT STANDARD      注意：当前CBC输出文件不是标准的
    inline bool solveCbc(bool sim_out)
    {
        FILE* command;
        if (false == sim_out) command = popen("./bin/cbc", "w");
        else                  command = popen("./bin/cbc > ilp.out", "w");

        fprintf(command, "import %s\n", l_inIlpName.c_str());
        fprintf(command, "solv\n");
        fprintf(command, "solution %s\n", l_outIlpName.c_str());
        fprintf(command, "exit\n");
        pclose(command); return true;
    }
};

/*
// ==== Implementation Log:
//
// 12/2014: more parameters in prepareGurobi()
// 10/2014: maximum runtime as parameter in prepareGurobi()
// 05/2014: defined as inline functions
// 05/2014: introduce namespace bLib
/2014年12月：准备中的更多参数Gurobi（）
//2014年10月：prepareGurobi（）中的最大运行时间参数
//2014年5月：定义为内联函数
//2014年5月：引入命名空间bLib 
//
*/

#endif

