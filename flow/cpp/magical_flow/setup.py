import os
import re
import sys
import platform
import subprocess

from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext
from distutils.version import LooseVersion


class CMakeExtension(Extension):
    def __init__(self, name, sourcedir=''):
        Extension.__init__(self, name, sources=[])
        self.sourcedir = os.path.abspath(sourcedir)


class CMakeBuild(build_ext):
    def run(self):
        try:
            out = subprocess.check_output(['cmake', '--version'])
        except OSError:
            raise RuntimeError("CMake must be installed to build the following extensions: " +
                               ", ".join(e.name for e in self.extensions))

        if platform.system() == "Windows":
            cmake_version = LooseVersion(re.search(r'version\s*([\d.]+)', out.decode()).group(1))
            if cmake_version < '3.1.0':
                raise RuntimeError("CMake >= 3.1.0 is required on Windows")

        for ext in self.extensions:
            self.build_extension(ext)

    def build_extension(self, ext):
        extdir = os.path.abspath(os.path.dirname(self.get_ext_fullpath(ext.name)))
        cmake_args = ['-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=' + extdir,
                      '-DPYTHON_EXECUTABLE=' + sys.executable]

        cfg = 'Debug' if self.debug else 'Release'
        build_args = ['--config', cfg]

        if platform.system() == "Windows":
            cmake_args += ['-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_{}={}'.format(cfg.upper(), extdir)]
            if sys.maxsize > 2**32:
                cmake_args += ['-A', 'x64']
            build_args += ['--', '/m']
        else:
            cmake_args += ['-DCMAKE_BUILD_TYPE=' + cfg]
            build_args += ['--', '-j2']

        env = os.environ.copy()
        env['CXXFLAGS'] = '{} -DVERSION_INFO=\\"{}\\"'.format(env.get('CXXFLAGS', ''),
                                                              self.distribution.get_version())
        if not os.path.exists(self.build_temp):
            os.makedirs(self.build_temp)
        subprocess.check_call(['cmake', ext.sourcedir] + cmake_args, cwd=self.build_temp, env=env)
        subprocess.check_call(['cmake', '--build', '.'] + build_args, cwd=self.build_temp)

setup(
    name='magicalFlow',
    version='0.0.1',
    author='Keren Zhu, Mingjie Liu',
    author_email='keren.zhu@utexas.edu, jay_liu@utexas.edu',
    description='The flow and database for Magical',
    long_description='',
    ext_modules=[CMakeExtension('magicalFlow')],
    cmdclass=dict(build_ext=CMakeBuild),
    zip_safe=False,
)

"""

在setup.py定义并编译安装magicalFlow模块后,在Python中可以直接导入它。
setup.py文件的作用就是编译和安装magicalFlow模块,使其可以在Python中导入和使用。安装完成后,magicalFlow模块就会放在以下位置:
1. 系统的site-packages目录下,例如/usr/lib/python3.7/site-packages。这需要root权限,一般不推荐。
2. 虚拟环境的site-packages目录下。如果是在虚拟环境中安装,magicalFlow就会放在虚拟环境的site-packages目录。
3. 当前用户的site-packages目录下,例如~/.local/lib/python3.7/site-packages。如果在用户环境下编译安装,magicalFlow就会放在这里。
将magicalFlow模块放在Python可搜索的路径后,就可以在Python中直接导入它了

"""