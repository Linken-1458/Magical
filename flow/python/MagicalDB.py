##enerateDeviceenerateDevice
# @file MagicalDB.py
# @author Keren Zhu
# @date 06/27/2019
# @brief The database for the magical flow. Ideally it should include everything needed
#

import DesignDB
import magicalFlow

class MagicalDB(object): 
    def __init__(self, params):    
        self.designDB = DesignDB.DesignDB()         # 初始化DesignDB对象designDB
        self.params = params                        # 保存了传入的params参数对象
        self.digitalNetNames = ["clk"]              # 初始化digitalNetNames列表来存储数字信号网名
        self.techDB = magicalFlow.TechDB()          # 初始化TechDB对象techDB

    def parse(self):
        self.parse_input_netlist(self.params)                       # 调用parse_input_netlist()解析输入的网表文件(从params对象获取)
        self.parse_simple_techfile(self.params.simple_tech_file)    # 调用parse_simple_techfile()解析简单工艺文件(从params对象获取)
        self.designDB.db.findRootCkt()                              # 调用designDB.db.findRootCkt()查找层次结构的根电路,DFS             After the parsing, find the root circuit of the hierarchy
        self.postProcessing()                                       # 调用postProcessing()进行后处理
        return True

    def postProcessing(self):                                       # 进行后处理
        self.markPowerNets()                                        # 标记功率网
        self.markDigitalNets()                                      # 标记数字信号网
        #self.computeCurrentFlow()                                  # 计算电流流

    def parse_simple_techfile(self, params):                        
        magicalFlow.parseSimpleTechFile( params, self.techDB)       # 调用magicalFlow.parse_simple_techfile()解析简单工艺文件，传入techDB对象和params参数

    def parse_input_netlist(self, params):                          # 用于解析输入的网表文件。它会根据params对象中的网表文件对应解析
        if (params.hspice_netlist is not None):                      
            self.read_hspice_netlist(params.resultDir+params.hspice_netlist)
            return
        if (params.spectre_netlist is not None):
            self.read_spectre_netlist(params.resultDir+params.spectre_netlist)
            return
        raise ParamException("No input netlist file!")

    def read_spectre_netlist(self, sp_netlist):         
        self.designDB.read_spectre_netlist(sp_netlist)              # 调用designDB的同名方法进行spectre网表的解析

    def read_hspice_netlist(self, sp_netlist):
        self.designDB.read_hspice_netlist(sp_netlist)               # 调用designDB的同名方法进行hspice网表的解析

    """"""""""""""""""""""""""""""""""""""""""""""""""""
    Current & Signal Flow   电流和信号流方法
    """""""""""""""""""""""""""""""""""""""""""""""""""
    def computeCurrentFlow(self):
        csflow = magicalFlow.CSFlow(self.designDB.db)                                   # 初始化magicalFlow.CSFlow对象csflow,传入designDB.db
        for cktIdx in range(self.designDB.db.numCkts()):                                # 遍历所有的电路cktIdx
            ckt = self.designDB.db.subCkt(cktIdx)
            if ckt.implType == magicalFlow.ImplTypeUNSET:                               # 如果ckt的implType是magicalFlow.ImplTypeUNSET
                csflow.computeCurrentFlow(ckt)                                          # 调用csflow.computeCurrentFlow(ckt)计算该电路的电流
                with open(self.params.resultDir + ckt.name + '.sigpath','w') as f:      # 打开结果目录的ckt.name + '.sigpath'文件
                    pinNamePaths = csflow.currentPinPaths();                            # 确认管脚名路径
                    cellNamePaths = csflow.currentCellPaths();                          # 确认单元名路径
                    assert len(pinNamePaths) == len(cellNamePaths)                      # 确认管脚名路径和单元名路径长度相等
                    for i in range(len(pinNamePaths)):                                  # 遍历管脚名路径
                        assert len(pinNamePaths[i]) == len(cellNamePaths[i])
                        for j in range(len(pinNamePaths[i])):                           # 遍历单元名路径
                            f.write(cellNamePaths[i][j] + " " + pinNamePaths[i][j] + " ")   # 将管脚名和单元名路径中的每一项写入文件，间隔一个空格
                        f.write("\n")                                                   # 写入一空行
    
    """""""""""""""""""""""""""""""""""""""""""""""""""
    Post-processing     后处理
    """""""""""""""""""""""""""""""""""""""""""""""""""
    
    ##==================================== markPowerNets()用于标记功率网 =========================================##
    def markPowerNets(self):                                    # markPowerNets()用于标记功率网
        for cktIdx in range(self.designDB.db.numCkts()):        # 遍历所有电路cktIdx
            ckt = self.designDB.db.subCkt(cktIdx)               # 
            # using flags from body connections
            for psubIdx in range(ckt.numPsubs()):               
                psubNet = ckt.psub(psubIdx)                     # 遍历所有的psubnets
                psubNet.markVssFlag()                           # 调用psubNet.markVssFlag()标记为VSS网
            for nwellIdx in range(ckt.numNwells()):
                nwellNet = ckt.nwell(nwellIdx)                  # 遍历所有的nwellNets
                nwellNet.markVddFlag()                          # 调用nwellNet.markVddFlag()标记为VDD网
            # Using external naming-based labeling
            vddNetNames = self.params.vddNetNames               # 获取VddNetNames列表
            vssNetNames = self.params.vssNetNames               # 获取VssNetNames列表
            for netIdx in range(ckt.numNets()):
                net = ckt.net(netIdx)                           # 遍历所有的Nets
                if net.name in vddNetNames:                     # 如果net的名字在VddNetNames中，调用net.markVddFlag()标记为VDD网
                    net.markVddFlag()
                if net.name in vssNetNames:                     # 如果net的名字在VssNetNames中，调用net.markVssFlag()标记为VSS网
                    net.markVssFlag()
    ##======================== 上面方法使用了psubnet、nwell连接以及网名来标记VDD和VSS功率网 ==========================##
    

    ##================================== markDigitalNets()用于标记数字信号网 =====================================##
    def markDigitalNets(self):
        for cktIdx in range(self.designDB.db.numCkts()):        
            ckt = self.designDB.db.subCkt(cktIdx)               # 遍历所有电路cktIdx
            # Using external naming-based labeling
            digitalNetNames = self.params.digitalNetNames       # 获取digitalNetNames列表
            for netIdx in range(ckt.numNets()):
                net = ckt.net(netIdx)                           # 遍历所有的nets
                if net.name in digitalNetNames:                 # 如果net的名字在digitalNetNames中
                    net.markDigitalFlag()                       # 调用net.markDigitalFlag()标记为数字信号网
                else:
                    net.markAnalogFlag()                        # 调用net.markAnalogFlag()标记为模拟信号网、
    ##============================== 上面方法使用网名来标记数字和模拟信号网 =========================================##

    """
    Some wrapper
    """
    ##=============================== topCktIdx()方法用于获取层次结构的根电路图的索引 ==============================##
    def topCktIdx(self):
        """
        @brief Get the index of circuit graph of the hierarchy  # topCktIdx()方法用于获取层次结构的根电路图的索引
        """
        return self.designDB.db.rootCktIdx()                    # 简单调用designDB.db.rootCktIdx()获取
    
    
    """
    utility
    """

    ##=============================== implTypeStr()工具函数，将implType转换为字符串 ==============================##
    def implTypeStr(self, implType):
        if implType == magicalFlow.ImplTypeUNSET:
            return "UNSET"
        if implType == magicalFlow.ImplTypePCELL_Cap:
            return "PCELLL_CAP"
        if implType ==  magicalFlow.ImplTypePCELL_Res:
            return "PCELL_RES"
        if implType == magicalFlow.ImplTypePCELL_Nch:
            return "PCELL_NCH"
        if implType == magicalFlow.ImplTypePCELL_Pch:
            return "PCELL_PCH"
        return "UNKNOWN"
