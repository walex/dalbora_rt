import subprocess
import sys
import os
import shutil
import fnmatch


class VsVersions:

    def __init__(self):

        self.vs_info = ["Visual Studio 18 2026", "VS180COMNTOOLS", "msvc18x64"]

    def get(self):

        try:
            return self.vs_info[0], self.vs_info[1], self.vs_info[2]
        except:
            return None, None, None


vsVersions = VsVersions()


def main():

    # check for windows
    if os.name != "nt":
        print("Visual studio solution can only be created on a windows machine")
        return

    # parse optional parameters
    generateOnly = True

    cmake_title, vsEnv, vsFolder = vsVersions.get()
    if cmake_title == None:
        print("Visual studio version not found or not supported")
        return

    cmake_exe = os.path.join("cmake.exe")

    # get visual studio environment
    vs_path = os.environ[vsEnv]
    if len(vs_path) == 0:
        print(
            "%s enviroment variable does not exits, please check for visual studio %s instalation"
            % (vsEnv, vsVersion)
        )
        return

    if os.path.isdir(vs_path) == False:
        print(
            "%s enviroment variable does not point to a valid visual studio common tools folder, please check for visual studio %s instalation"
            % (vsEnv, vsVersion)
        )
        return

    # save directories
    cur_dir = os.path.dirname(os.path.realpath(__file__))

    # create build directory
    build_dir = os.path.join(cur_dir, "build", vsFolder)
    if os.path.isdir(build_dir) == True:
        ans = input(
            "WARNING: build directory already exists.Remove and build again? (Y/N) "
        )
        if ans.strip().lower() != "y":
            return
    
    sdl_path=os.path.join(cur_dir, "thirdparty", "SDL3-3.4.0")
    
    if os.path.isdir(build_dir) == True:
        shutil.rmtree(build_dir)
    os.makedirs(build_dir)

    os.chdir(build_dir)

    cxx_flags = ""
    command = ('"%s" -G "%s" -A x64 -DCMAKE_PREFIX_PATH="%s" -DCMAKE_CXX_FLAGS="%s" "%s"') % (
        cmake_exe,
        cmake_title,
        sdl_path,
        cxx_flags,
        cur_dir,
    )
    subprocess.call(command, shell=True)

    # if generateOnly == True:
    #    return

    # os.chdir(cur_dir)

    # build and install
    # subprocess.call(('compilemsvc.bat %s "%s" CardDetection Release 1') % (vsEnv, os.path.join(build_dir, "CardDetection.sln")), shell=True)


if __name__ == "__main__":
    main()
