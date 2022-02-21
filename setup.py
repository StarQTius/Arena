from subprocess import check_call
from os import path, makedirs

from setuptools import Extension, setup
from setuptools.command.build_ext import build_ext

class CMakeExtension(Extension):
  def __init__(self, name, sourcedir):
    Extension.__init__(self, name, sources=[])
    self.sourcedir = path.abspath(sourcedir)

class CMakeBuild(build_ext):
  def build_extension(self, ext):
    extdir = path.abspath(path.dirname(self.get_ext_fullpath(ext.name)))
    if not path.exists(self.build_temp):
      makedirs(self.build_temp)

    check_call(["cmake", ext.sourcedir, "-DCMAKE_BUILD_TYPE=Release", f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={extdir}", "-DArena_BUILD_MODULE=ON"], cwd=self.build_temp)
    check_call(["cmake", "--build", "."], cwd=self.build_temp)

setup(
  name="arena",
  ext_modules=[CMakeExtension(name="arena", sourcedir=".")],
  cmdclass={"build_ext": CMakeBuild}
)
