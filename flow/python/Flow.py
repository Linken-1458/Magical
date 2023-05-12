##
# @file Flow.py
# @author Keren Zhu
# @date 07/07/2019
# @brief The flow
#

import magicalFlow
import Device_generator
import Constraint
import PnR
import StdCell
import subprocess
import time

##============================= flow工艺流程 ==========================##
class Flow(object):
    def __init__(self, db):
        self.mDB = db                                           # 初始化mDB 作为MagicalDB对象的引用
        self.dDB = db.designDB.db                               # 初始化dDB 作为designDB(C++)数据库的引用
        self.constraint = Constraint.Constraint(self.mDB)       # 初始化constraint作为Constraint对象的引用
        self.params = self.mDB.params                           # 初始化params 作为mDB的params参数对象的引用
        self.pnrs = []                                          # 初始化pnrs列表
        self.runtime = 0                                        # 初始化运行时间

    def run(self):                                              # run()主要执行流程
        """
        @brief the main function to run the flow
        @return if successful
        """
        self.resultName = self.mDB.params.resultDir             # 获取结果目录名resultName
        topCktIdx = self.mDB.topCktIdx()                        # 获取顶层电路索引topcktIdx
        start = time.time()                                     # 记录开始时间
        self.implCktLayout(topCktIdx)                           # 调用implCktLayout()实现顶层电路的布局
        end = time.time()                                       # 记录结束时间
        print("runtime ", end - start)                          # 输出运行时间
        for pnr in self.pnrs:                                   # 对pnrs列表中的每一个PnR对象调用routeOnly()进行布线
            pnr.routeOnly()
        return True

    def generateConstraints(self):                                                      # 生成除设备和标准单元之外的电路布局约束
        for cktIdx in range(self.dDB.numCkts()):                                        # 遍历所有的电路CktIdx
            ckt = self.dDB.subCkt(cktIdx) #magicalFlow.CktGraph
            if magicalFlow.isImplTypeDevice(ckt.implType):
                continue
            if self.isCktStdCells(cktIdx):
                continue
            self.symDict = self.constraint.genConstraint(cktIdx, self.resultName)       # 对于不是设备或者标准单元的电路，调用constraint.genConstraint()生成约束，并将结果存储在symDict中


    def isCktStdCells(self, cktIdx):                                                    # 通过电路名判断电路是否是标准单元
        if self.dDB.subCkt(cktIdx).name in self.params.stdCells:                        # 检查电路名是否存在在params.stdCells列表中，不是则返回False
            return True
        else:
            return False


    def setup(self, cktIdx):                                                            # 用于设置
        ckt = self.dDB.subCkt(cktIdx)                                                   
        for nodeIdx in range(ckt.numNodes()):                                           # 遍历所有的节点cktnode
            flipCell = False
            cktNode = ckt.node(nodeIdx)
            # Flip cell if is in the "right" half device of symmetry
            if cktNode.name in self.symDict.values():                                   # 如果cktNode在symDict中，说明是对称单元，flipCell设为True
                flipCell = True
            if cktNode.isLeaf():                                                        # 如果cktNode是叶节点，跳过此次循环直接进入下个循环
                continue
            subCktIdx = self.dDB.subCkt(cktIdx).node(nodeIdx).graphIdx                  # 不是叶节点，获取cktNode的子电路索引subCktIdx
            devGen = Device_generator.Device_generator(self.mDB)                        # 实例化一个Device_generator设备生成器对象devGen，在初始化时提供MagicalDB对象作为参数，以支持后续设备布局生成操作
            if magicalFlow.isImplTypeDevice(self.dDB.subCkt(subCktIdx).implType):       # 如果subCktIdx是设备
                if flipCell:                                                            # 且flipCell为True，调用Device_generator生成对称设备布局
                    devGen.generateDevice(subCktIdx, self.resultName+'/gds/', True)     #FIXME: directly add to the database
                else:                                                                   # 否则生成标准设备布局，并读取GDS
                    devGen.generateDevice(subCktIdx, self.resultName+'/gds/', False)    
                devGen.readGDS(subCktIdx, self.resultName+'/gds/')
            else:                                                                       # 如果subCktIdx不是设备
                if flipCell:
                    cktNode.flipVertFlag = True                                         # 如果flipCell为True，设置cktNode的flipVertFlag为True
    """
    这个方法的作用：
    1、对非叶节点的子电路,检查是否为对称设备
    2、如果是对称设备,生成对称或非对称设备布局并读取GDS
    3、如果不是设备,但是对称单元,设置flipVertFlag以进行对称布局
    这个方法利用Device_generator对象来生成设备布局,为下一步(PnR布局和布线)做准备
    """


    def implCktLayout(self, cktIdx):
        """
        @brief implement the circuit layout                                             # 用于电路的布局
        """
        dDB = self.mDB.designDB.db #c++ database                                        # 这里的dDB是C++数据库
        ckt = dDB.subCkt(cktIdx) #magicalFlow.CktGraph                                  # 电路拓扑结构ckt引用
        # If the ckt is a device, generation will be added in setup()                   # 如果ckt是设备，则通过Device_generator生成设备布局，并return
        if magicalFlow.isImplTypeDevice(ckt.implType):
            Device_generator.Device_generator(self.mDB).generateDevice(cktIdx, self.resultName+'/gds/') #FIXME: directly add to the database
            return
        # If the ckt is a standard cell                                                 # 如果ckt是标准单元，则通过StdCell对象进行设置，并return
        # This version only support DFCNQD2BWP and NR2D8BWP, hard-encoded
        # TODO: This should be parsed from the json file
        if self.isCktStdCells(cktIdx):
            StdCell.StdCell(self.mDB).setup(cktIdx, self.resultName)
            return
        # If the ckt is actually a circuit instead of a device
        for nodeIdx in range(ckt.numNodes()):                                           # 遍历ckt所有的节点，numNodes()获得节点数量
            cktNode = ckt.node(nodeIdx) # magicalFlow.CktNode                           # 获取节点cktNode的引用，node()方法传入节点索引nodeIdx获取相应节点
            if cktNode.isLeaf(): # Do not go deeper for leaf node                       # 如果cktNode是叶节点，这两行跳过，继续下一个节点处理
                continue
            subCkt = dDB.subCkt(cktNode.graphIdx)
            if (subCkt.isImpl): # Do not duplicately implement the layout               # 这两行获取cktNode的子电路subCkt引用，并检查是否已经布局，如果是则跳过后面操作，继续下个节点
                continue
            self.implCktLayout(cktNode.graphIdx) # Recursively implement all the children   # 这行递归调用自身方法来实现子电路subCkt的布局，实现层级布局
            ckt = dDB.subCkt(cktIdx) # just to make sure the reference is not messed up     # 这行重新获取ckt的引用，以防在递归调用中引用被改变
        # After all the children being implemented. P&R at this circuit
        ckt = dDB.subCkt(cktIdx) #magicalFlow.CktGraph                                  # 这行重新获取ckt的引用，以防在递归调用中引用被改变
        self.symDict = self.constraint.genConstraint(cktIdx, self.resultName)           # 生成布局约束symDict，并调用setup()方法进行设置
        self.setup(cktIdx)
        pnr = PnR.PnR(self.mDB)                                                         # 创建PnR对象pnr
        pnr.placeOnly(cktIdx, self.resultName)                                          # 调用placeOnly()方法进行布局
        self.runtime += pnr.runtime                                                     # 累加pnr运行时间
        self.pnrs.append(pnr)                                                           # 将其添加到pnrs列表
        #PnR.PnR(self.mDB).implLayout(cktIdx, self.resultName)
