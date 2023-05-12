##
# @file Magical.py
# @author Keren Zhu
# @date 06/27/2019
# @brief Main file to run the Magical hierarchical flow
#

import os
import sys
import Params, MagicalDB, Flow

class Magical(object):
    def __init__(self, jsonFile):
        self.params = Params.Params()               # 初始化时，接受jsonFile参数，加载到self.params变量
        self.params.load(jsonFile)
        self.db = MagicalDB.MagicalDB(self.params)  # 初始化MagicalDB对象DB     The flow database
        self.db.parse()                             # 调用了parse()方法         parsing the input files
        self.flow = Flow.Flow(self.db)
    def run(self):                                  # 定义了一个run（ ）方法
        self.flow.run()

if __name__ == "__main__":
    """
    @brief main function to invoke the entire Magical flow  调用整个Magical的主函数 
    """

    params = Params.Params()                        # 加载命令行传入的json文件到params变量
                                                    # A=B.C() B这里是Params类名，表示实例化的这个类;A和C表示创建的对象，用来访问对象的各种属性和方法
                                                    # 创建B类的一个实例对象C，并将其赋值给A的变量
    params.printWelcome()
    if len(sys.argv) == 1 or '-h' in sys.argv[1:] or '--help' in sys.argv[1:]:
        params.printHelp()                          # 如果没有给出命令行参数len(sys.argv) == 1，或者给出-h或-help参数，调用params.printWelcome()打印帮助信息，然后退出
                                                    # python ../../flow/python/Magical.py -h
        exit()
    elif len(sys.argv) != 2:
        print("[E] One input parameters in json formatn required")
        params.printHelp()                          # 如果命令行参数数量不等于2，会提示需要一个json格式的参数输入，调用params.printWelcome()打印帮助信息，然后退出
                                                    # python ../../flow/python/Magical.py adc1.json 
        exit()

    ##=========================== load parameters 加载参数 ======================================##
    
    params.load(sys.argv[1])                        # 加载命令行传入的json参数文件到params对象（Params类的实例）
    
    print("[I] parameters = %s" % (params))         # 打印加载的参数信息params

    db = MagicalDB.MagicalDB(params)                # 初始化MagicalDB对象DB,并传入params对象

    db.parse() #parsing the input files             # 调用了parse()方法进行解析

    flow = Flow.Flow(db)                            # 初始化Flow对象flow，传入db对象

    flow.run()                                      # 调用了run()方法

#   Workaround for Pyinstaller. ref:https://github.com/pyinstaller/pyinstaller/issues/2820
if 0: 
    import UserList
    import UserString
    import UserDict
    import itertools
    import collections
    import future.backports.misc
    import commands
    import base64
    import __buildin__
    import math
    import reprlib
    import functools
    import re
    import subprocess
