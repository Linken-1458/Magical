##
# @file Params.py
# @author Keren Zhu
# @date 06/27/2019
# @brief The parameters of the flow
#

import json
import math

class Params:
    """
    @brief Parameter class              # 定义了一个Params类，包含了芯片设计的相关参数，比如网表文件、工艺文化、标准单元等信息
                                        # 能随意的获取、转换、存储其中的参数数据
    """
    def __init__(self):
        """
        @brief initialization
        """
        self.spectre_netlist = None     # 保存了输入的网表文件  Input spectre netlist file
        self.hspice_netlist = None      # 保存了输入的网表文件  Input hspice netlist file
        self.simple_tech_file = ""      # 保存了工艺文件       Input simple tech file
        self.techfile = ""              # 保存了工艺文件
        self.lef = ""                   # 存储了工艺LEF文件
        self.vddNetNames = ["VDD", "vdd", "vdda", "vddd"]                   # 保存了电源网名
        self.vssNetNames = ["VSS", "GND", "vss", "gnd", "vssa", "vssd"]     # 保存了接地网名
        self.digitalNetNames = ["clk"]  # 保存了时钟信号CLK的网名
        self.stdCells = ['SR_Latch_LVT','NR2D8BWP_LVT','BUFFD4BWP_LVT','DFCND4BWP_LVT','INVD4BWP_LVT','DFCNQD2BWP_LVT', 'DFCND4BWP_LVT_stupid']     # 保存了标准单元的名字
        
        ##======================这部分代码定义了很多表格，用来给不同情况下的导线宽度和VIA切口数量赋值===============================##
        self.resultDir = None               # 存储了结果目录
        self.powerLayer = 6                 # 存储了芯片的功率层
        self.psubLayer = self.powerLayer    # 存储了衬底接触层      same as power pin
        self.smallModuleAreaThreshold = 60  # 存储了小模块的面积阈值，单位是um^2
        self.signalAnalogWireWidthTable = \
        [ [0, 0.1],  [120, 0.2]]            # 给模拟信号赋予不同长度下的导线宽度～～～～～～～～～～～～～～～～～～～～～～～～～
        self.signalDigitalWireWidthTable = \
        [ [0, 0.1], [100, 0.12]]            # 给数字信号赋予不同长度下的导线宽度～～～～～～～～～～～～～～～～～～～～～～～～～
        self.powerWireWidthTable = \
        [[0, 0.5]]                          # 给功率信号赋予导线宽度～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～
        self.dpowerWireWidthTable = \
        [[0, 0.3]]                          # 给数字功率信号赋予导线宽度～～～～～～～～～～～～～～～～～～～～～～～～～～～～～
        # via  [um in length, # of cuts, # of rols, # of cols]
        self.signalAnalogViaCutsTable = \
        [[0, 2, 1, 2], [80, 4, 2, 2], [120, 9, 3, 3]]       # 给模拟信号的VIA在不同长度下分配切口数量～～～～～～～～～～～～
        self.signalDigitalViaCutsTable = \
        [[0, 2, 1, 2], [120, 4, 2, 2]]                      # 给数字信号的VIA在不同长度下分配切口数量～～～～～～～～～～～～
        self.powerViaCutsTable = \
        [[0, 4, 2, 2], [100, 9, 3, 3]]                      # 给电源信号的VIA在不同长度下分配切口数量～～～～～～～～～～～～
        # Additional spacing
        # increase module spacing by area [um^2, um]
        self.blockSpacingFromAreaTableX = \
        [[0.0, 0.2], [100,0.8]]                             # 根据模块面积为模块间的X方向间距赋值
        self.blockSpacingFromAreaTableY = \
        [[0.0, 0.2], [100, 0.4]]                            # 根据模块面积为模块间的Y方向间距赋值

    def printWelcome(self):
        """
        @brief print welcome message                        # 打印欢迎开始程序的信息
        """
        content = """\
========================================================
    MAGICAL: Machine Generated Analog IC Layout            
        https://github.com/magical-eda/MAGICAL
========================================================"""
        print(content)

    def printHelp(self):
        """
        @brief print help message for JSON parameters
        """
        content = """\
                    JSON Parameters
========================================================
spectre_netlist [required for spectre netlist]      | input .sp file 
hspice_netlist [required for hspice netlist]        | input .sp file 
simple_tech_file [required]                         | input simple techfile 
        """ % (self.spectre_netlist,
                self.hspice_netlist,
                self.simple_tech_file
                )
        print(content)


    def toJson(self):
        """
        @brief convert to json                              # 将对象序列化为json数据 
        """
        data = dict()
        data['spectre_netlist'] = self.spectre_netlist      # 网表文件
        data['hspice_netlist'] = self.hspice_netlist        # 网表文件
        data['simple_tech_file'] = self.simple_tech_file    # 工艺文件
        data['resultDir'] = self.resultDir                  # 结果目录
        return data 

    def fromJson(self, data):
        """
        @brief load form json                               # 从json数据反序列化为对象 1
        """
        if 'spectre_netlist' in data: self.spectre_netlist = data['spectre_netlist']        # 保存了输入的网表文件
        if 'hspice_netlist' in data: self.hspice_netlist = data['hspice_netlist']           # 保存了输入的网表文件
        if 'simple_tech_file' in data: self.simple_tech_file = data['simple_tech_file']     # 保存了工艺文件
        if 'resultDir' in data: self.resultDir = data['resultDir']                          # 存储了结果目录
        if 'lef' in data : self.lef = data['lef']                                           # 存储了工艺LEF文件
        if 'techfile' in data : self.techfile = data['techfile']                            # 保存了工艺文件
        if 'vddNetNames' in data : self.vddNetNames = data['vddNetNames']                   # 保存了电源网名
        if 'vssNetNames' in data : self.vssNetNames = data['vssNetNames']                   # 保存了接地网名

    def dump(self, filename):
        """
        @brief dump to json file        # 对象和json文件格式之间的序列化
        """
        with open(filename, 'w') as f:
            json.dump(self.toJson(), f)

    def load(self, filename):
        """
        @brief load from json file      # 对象和json文件格式之间的反序列化
        """
        with open(filename, 'r') as f:
            self.fromJson(json.load(f))

    def __str__(self):
        """
        @brief string                   # 对象的字符串表示
        """
        return str(self.toJson())

    def __repr__(self):
        """
        @brief print                    # 输出字符串
        """
        return self.__str__()
