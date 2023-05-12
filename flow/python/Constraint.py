#
# @file PnR.py
# @author Mingjie Liu
# @date 09/30/2019
# @brief The class for generating symmetry constraints for PnR
#
import subprocess
import magicalFlow
import S3DET
import os
import ConstGenPy


##============================= constraint 布局约束生成 ================================##
class Constraint(object):
    def __init__(self, magicalDB):                      # 初始化方法，接收magicalDB数据库对象的引用
        self.mDB = magicalDB                            # 获取数据库mDB
        self.dDB = magicalDB.designDB.db                # 获取设计数据库dDB
        self.tDB = magicalDB.techDB                     # 获取工艺数据库tDB
        self.s3det = S3DET.S3DET(self.mDB)              # S3DET对象的引用

    def genConstraint(self, cktIdx, dirName):           # 生成电路cktIdx的布局约束，dirName为输出目录
        cktname = self.dDB.subCkt(cktIdx).name          # 获取电路名称cktname
        if not os.path.isfile(dirName+cktname+'.sym'):  # 检查是否已存在sym文件
            if self.primaryCell(cktIdx):                # 如果是primaryCell基本单元，则调用primarySym()生成约束
                #pass
                self.primarySym(cktIdx, dirName)
                #print "%s is a primary cell, generating constraints." % cktname
                #print "Constraints saved at %s.sym" % cktname 
            else:
                self.s3det.systemSym(cktIdx, dirName)   # 否则调用S3DET对象的systemSym()生成约束
                #print "%s is not a primary cell." % cktname
                #print "Constraints saved at %s.sym" % cktname 
        return self.parseSym(cktIdx, dirName)           # 调用parseSym()解析生成的sym文件，并返回解析结果symDict

    def parseSym(self, cktIdx, dirName):                # 用于解析sym文件，cktIdx为电路索引，dirName为输出目录
        cktname = self.dDB.subCkt(cktIdx).name          
        symDict = dict()
        symFile = dirName + cktname + ".sym"
        with open(symFile) as fin:                      # 打开sym文件，逐行解析得到约束信息，并存储在symDict字典中，最后返回该字典
            lines = fin.readlines()
            for line in lines:
                cellNames = line.split()
                if len(cellNames) > 1:
                    symDict[cellNames[0]] = cellNames[1]
        return symDict

    def primaryCell(self, cktIdx):                      # 检查cktIdx对应的电路是否为primary cell基本单元
        """
        @brief Checking if cell is primary

        """
        ckt = self.dDB.subCkt(cktIdx)
        print("ckt name", ckt.name)
        for nodeIdx in range(ckt.numNodes()):           # 这段代码遍历电路ckt的所有节点，检查每个子电路subckt的implType,如果有非设备则返回False，否则返回True
            instNode = ckt.node(nodeIdx)
            print("node name", instNode.name)
            subckt = self.dDB.subCkt(instNode.graphIdx)
            if not magicalFlow.isImplTypeDevice(subckt.implType):
                return False
        return True

    def primarySym(self, cktIdx, dirName):              # 为primaryCell基本单元生成sym约束文件
        constGen = ConstGenPy.ConstGen()                # 创建ConstGenPy.ConstGen()对象constGen，用于生成约束
        ckt = self.dDB.subCkt(cktIdx)                   # 获取cktIdx对应的电路ckt
        phyDB = self.dDB.phyPropDB()                    # 引用工艺数据库phyDB
        mosPinType = [ConstGenPy.D, ConstGenPy.G, ConstGenPy.S, ConstGenPy.B]      # 定义mos管引脚类型的列表
        capPinType = [ConstGenPy.THIS, ConstGenPy.THAT, ConstGenPy.OTHER]          # 定义电容器引脚类型的列表
        for net in range(ckt.numNets()):                                           # 遍历电路的所有网，调用constGen的addNet()方法添加，并判断索引是否匹配
            idx = constGen.addNet(ckt.net(net).name, net)
            assert net == idx, "Net index not matched!"                            # assert用法，如果net=idx,程序继续往下进行
        for nodeIdx in range(ckt.numNodes()):                                      # 遍历电路的所有节点，获取节点instNode
            instNode = ckt.node(nodeIdx)
            assert not magicalFlow.isImplTypeDevice(instNode.implType), "Circuit %s contains non instance %s" %(ckt.name + instNode.name)
            # 检查每个子电路instNode是否为设备类型，如果不是则抛出异常终止程序。异常信息：Circuit %s(电路名称) contains non instance %s(实例名称)，某某电路包含非实例
            # 备注:子电路包括电路和实例，实例是具体物理设备例如Mos管、电容器。而生成物理方面的约束，必须要基于实例和工艺数据库中的具体实现，如果包含其他电路，无法生成正确的约束信息。
            cirNode = self.dDB.subCkt(instNode.graphIdx)                           # 获取子电路cirNode和实现索引instPIdx，并根据implType添加器件
            instPIdx = cirNode.implIdx
            if cirNode.implType == magicalFlow.ImplTypePCELL_Nch:                  # 添加N型Mos管
                nch = phyDB.nch(instPIdx)
                idx = constGen.addInst(instNode.name, ConstGenPy.Nch, nch.width, nch.length, nch.numFingers)
                pinTypeArray = mosPinType
            elif cirNode.implType == magicalFlow.ImplTypePCELL_Pch:                # 添加P型Mos管
                pch = phyDB.pch(instPIdx)
                idx = constGen.addInst(instNode.name, ConstGenPy.Pch, pch.width, pch.length, pch.numFingers)
                pinTypeArray = mosPinType
            elif cirNode.implType == magicalFlow.ImplTypePCELL_Res:                # 添加电阻
                res = phyDB.resistor(instPIdx)
                idx = constGen.addInst(instNode.name, ConstGenPy.Res, res.wr, res.lr, res.segNum)
                pinTypeArray = capPinType
            elif cirNode.implType == magicalFlow.ImplTypePCELL_Cap:                # 添加电容器
                cap = phyDB.capacitor(instPIdx)
                idx = constGen.addInst(instNode.name, ConstGenPy.Cap, cap.w, cap.lr, cap.numFingers)
                pinTypeArray = capPinType
            else:
                assert False, "Device not supported %s" % instNode.name
                # 如果一个实例insNode的类型不属于MOS管、电阻或电容器，则抛出异常提示该实例类型不支持
            for pin_idx in range(instNode.numPins()):                              # 遍历insNode的所有引脚，pin_idx为引脚索引
                pinIdx = instNode.pinIdx(pin_idx)                                  # 获取pin_idx引脚对应的pinIdx
                netIdx = ckt.pin(pinIdx).netIdx                                    # 获取pin_idx引脚连接的网netIdx
                constGen.addInstPin(idx, netIdx, pinTypeArray[pin_idx])            
                # 调用constGen的addInstPin()方法，添加instNode的一个引脚约束，idx为实例索引，netIdx为网索引，pinTypeArray[pin_idx]为引脚类型
            assert nodeIdx == idx                                                  # insdtNode的节点索引nodeIdx与上面添加的实例索引idx相等，用于检查
        constGen.dumpResult(dirName + ckt.name)                                    # 调用constGen的dumpResult()方法生成约束文件   

    def primarySymFile(self, cktIdx, dirName):              # cktIdx为电路索引，dirName为输出目录 
        """
        @brief Generating constraint files with .iniObj     # 实现了为primary cell生成.iniObj约束文件的逻辑
        """
        self.writeInitObj(cktIdx, dirName)                  # 调用writeInitObj()方法，生成.iniObj文件
        cirname = self.dDB.subCkt(cktIdx).name              # 获取cktIdx对应的电路名称cirname
        cmd = "source /home/unga/jayliu/projects/develop/magical/magical/install/constraint_generation/test/run_init.sh " + cirname + " " + dirName + " &>/dev/null"
        # 定义shell命令cmd，用于运行run_init.sh脚本生成约束
        subprocess.call(cmd, shell=True)                    # 使用subprocess模块调用cmd命令，运行run_init.sh脚本生成约束

    def writeInitObj(self, cktIdx, dirName):
        """
        @brief write .initObj file                          # 生成.iniObj约束文件的逻辑
        """
        ckt = self.dDB.subCkt(cktIdx)                       # 获取电路ckt
        phyDB = self.dDB.phyPropDB()                        # 获取工艺数据库phyDB引用
        filename = dirName + ckt.name + '.initObj'          # 定义输出名filename
        fout = open(filename, "w")                          # 打开文件fout用于写入
        instId = 0                                          # instId为实例计数器，用于给实例编号
        for nodeIdx in range(ckt.numNodes()):               # 开始遍历电路u的所有节点
            instNode = ckt.node(nodeIdx)                    # 获取当前节点instNode
            assert not magicalFlow.isImplTypeDevice(instNode.implType), "Circuit %s contains non instance %s" %(ckt.name + instNode.name)
            cirNode = self.dDB.subCkt(instNode.graphIdx)    # 获取当前节点instNode对应的子电路cirNode
            instPIdx = cirNode.implIdx                      # 获取当前节点instNode实现索引instPIdx
            fout.write("Inst\n%d\n" % instId)               # 用于向文件fout写入实例instNode的类型和从工艺数据库phyDB获取的参数
            if cirNode.implType == magicalFlow.ImplTypePCELL_Nch:
                fout.write("NMOS\n%s\n" % instNode.name)    # 实例类型为NMOS，名称为instNode.name
                nch = phyDB.nch(instPIdx)                   # 调用phyDB获取nch信息，写入宽度、长度、numFingers
                fout.write("%de-12 %de-12 %d\n" %(nch.width, nch.length, nch.numFingers))
            elif cirNode.implType == magicalFlow.ImplTypePCELL_Pch:
                fout.write("PMOS\n%s\n" % instNode.name)    # 实例类型为PMOS，名称为instNode.name
                pch = phyDB.pch(instPIdx)                   # 调用phyDB获取pch信息，写入宽度、长度、numFingers
                fout.write("%de-12 %de-12 %d\n" %(pch.width, pch.length, pch.numFingers))
            elif cirNode.implType == magicalFlow.ImplTypePCELL_Res:
                fout.write("RES\n%s\n" % instNode.name)     # 实例类型为电阻，名称为instNode.name
                res = phyDB.resistor(instPIdx)              # 调用phyDB获取res信息，写入宽度、长度、numFingers
                fout.write("%de-12 %de-12\n" %(res.wr, res.lr))
            elif cirNode.implType == magicalFlow.ImplTypePCELL_Cap:
                fout.write("CAP\n%s\n" % instNode.name)     # 实例类型为电容器，名称为instNode.name
                cap = phyDB.capacitor(instPIdx)             # 调用phyDB获取cap信息，写入宽度、长度、numFingers
                fout.write("%de-12 %de-12\n" %(cap.w, cap.lr))
            else:
                fout.write("OTHER\n%s\n" % instNode.name)   # 实例类型不属于前面判断的几种，将类型写入为OTHER
            for pin_idx in range(instNode.numPins()):       # 遍历实例insNode的所有引脚
                pinIdx = instNode.pinIdx(pin_idx)
                netIdx = ckt.pin(pinIdx).netIdx
                fout.write("%d\n" % netIdx)                 # 获取引脚pinIdx对应的网编号netIdx，并写入文件fout
                #fout.write("%d %s\n" %(netIdx,ckt.net(netIdx).name))
            instId += 1                                     # 更新实例计数器instId
        for net in range(ckt.numNets()):                    # 遍历电路ckt所有的网net，将每个网的编号和名称写入fout文件
            fout.write("NET\n%d\n%s\n" % (net, ckt.net(net).name))

