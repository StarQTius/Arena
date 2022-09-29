import sys
from os import makedirs, path, symlink
from re import search
from subprocess import check_call

from git.refs.tag import Tag
from git.repo.base import Repo
from setuptools import Extension, setup, find_packages
from setuptools.command.build_ext import build_ext
from pathlib import Path

repository = Repo(path.dirname(__file__))
version = search(r"(?<=v)(\d+\.\d+\.\d+)", Tag.list_items(repository)[-1].path).group(0)
build_dir = f"build/build_{sys.version_info.major}_{sys.version_info.minor}"

Path(build_dir).mkdir(parents=True, exist_ok=True)
Path(f"{build_dir}/arena").mkdir(exist_ok=True)
Path(f"{build_dir}/arena/sail_the_world").mkdir(exist_ok=True)
Path(f"{build_dir}/arena/the_cherry_on_the_cake").mkdir(exist_ok=True)

class CMakeExtension(Extension):
    def __init__(self, name, sourcedir):
        Extension.__init__(self, name, sources=[])
        self.sourcedir = path.abspath(sourcedir)


class CMakeBuild(build_ext):
    def build_extension(self, ext):
        extdir = path.abspath(path.dirname(self.get_ext_fullpath(ext.name)))
        
        check_call(
            [
                "cmake",
                "-B", build_dir,
                f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={extdir}",
                "-DCMAKE_BUILD_TYPE=Release",
                f"-DPython3_EXECUTABLE={sys.executable}",
            ],
        )
        check_call(["cmake", "--build", build_dir, "-t", "module", "-j4"])


setup(
    name="eurobot-arena",
    version=version,
    packages=["arena", "arena.sail_the_world", "arena.the_cherry_on_the_cake"],
    package_dir={
        "arena":f"{build_dir}/arena",
        "arena.sail_the_world":f"{build_dir}/arena/sail_the_world",
        "arena.the_cherry_on_the_cake":f"{build_dir}/arena/the_cherry_on_the_cake"
    },
    ext_modules=[CMakeExtension(name="arena._details", sourcedir=".")],
    cmdclass={"build_ext": CMakeBuild},
    install_requires="pygame",
)
