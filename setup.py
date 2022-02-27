from os import environ, makedirs, path
from re import search
from subprocess import check_call

from git.refs.tag import Tag
from git.repo.base import Repo
from setuptools import Extension, setup
from setuptools.command.build_ext import build_ext

repository = Repo(path.dirname(__file__))
version = search(r"(?<=v)(\d+\.\d+\.\d+)", Tag.list_items(repository)[-1].path).group(0)


class CMakeExtension(Extension):
    def __init__(self, name, sourcedir):
        Extension.__init__(self, name, sources=[])
        self.sourcedir = path.abspath(sourcedir)


class CMakeBuild(build_ext):
    def build_extension(self, ext):
        extdir = path.abspath(path.dirname(self.get_ext_fullpath(ext.name)))

        if not path.exists(self.build_temp):
            makedirs(self.build_temp)

        check_call(
            [
                "cmake",
                ext.sourcedir,
                "-DCMAKE_BUILD_TYPE=Release",
                f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={extdir}",
                "-DARENA_BUILD_MODULE=ON",
            ],
            cwd=self.build_temp,
        )
        check_call(["cmake", "--build", "."], cwd=self.build_temp)


setup(
    name="eurobot-arena",
    version=version,
    install_requires="pygame",
    ext_modules=[CMakeExtension(name="arena", sourcedir=".")],
    cmdclass={"build_ext": CMakeBuild},
)
