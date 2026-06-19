import multiprocessing
import platform
import shutil
import subprocess
import sys
from pathlib import Path

system = ""
configuration = ""


def check_system():
    global system

    supported_systems = ["windows", "linux"]
    system = platform.system().lower()

    if system not in supported_systems:
        print(
            f"System '{system}' is not supported. Supported systems: {supported_systems}"
        )
        sys.exit(1)

    return


def check_configuration():
    global configuration

    supported_configurations = ["debug", "release"]
    configuration = configuration.lower()

    if configuration not in supported_configurations:
        print(
            f"Configuration '{configuration}' is not supported. Supported configurations: {supported_configurations}"
        )
        sys.exit(1)

    return


# ----- Helpers -----
def get_number_of_cores():
    return multiprocessing.cpu_count()


def has_executable(executable):
    return shutil.which(executable) is not None


def get_cmake_build_dir():
    cmake_build_dir = f"build/{system}/{configuration}/obj"
    return cmake_build_dir


def get_clang_build_dir():
    clang_build_dir = "build/clang"
    return clang_build_dir


# ----- Configure -----
def configure():
    subprocess.run(
        [
            "cmake",
            "-S scripts",
            f"-B {get_cmake_build_dir()}",
            "-DCMAKE_C_COMPILER=clang",
            "-DCMAKE_CXX_COMPILER=clang++",
            f"-DCMAKE_BUILD_TYPE={configuration}",
            # Change the linker here
            '-DCMAKE_EXE_LINKER_FLAGS="-fuse-ld=mold"',  # \"-fuse-ld=gold\"
            '-DCMAKE_SHARED_LINKER_FLAGS="-fuse-ld=mold"',  # \"-fuse-ld=gold\"
        ],
        check=True,
    )

    return


# ----- Build -----
def build():
    number_of_cores = get_number_of_cores()

    print(
        f"(Python) Number of cores: {number_of_cores} - Building {system} ({configuration})"
    )

    # Run make
    subprocess.run(
        ["make", f"-j{number_of_cores}"], cwd=get_cmake_build_dir(), check=True
    )

    return


# ----- Clean -----
def clean():
    subprocess.run(["make", "clean"], cwd=get_cmake_build_dir(), check=True)

    return


# ----- Format -----
def format_files():

    if not has_executable("clang-format"):
        return

    sources = list(Path("magnolia/").rglob("*.*"))

    subprocess.run(["clang-format", "-i", "-style=file", *sources], check=True)

    return


# ----- Lint -----
def lint():

    if not has_executable("cppcheck"):
        return

    sources = list(Path("magnolia/").rglob("*.*"))

    subprocess.run(
        [
            "cppcheck",
            "--std=c++23",
            "--check-level=exhaustive",
            f"--output-file={get_clang_build_dir()}/lint.txt",
            "--enable=all",
            "--suppress=missingInclude",
            "--suppress=missingIncludeSystem",
            "--suppress=useStlAlgorithm",
            "--suppress=unusedFunction",
            "--suppress=unknownMacro",
            "--suppress=unusedStructMember",
            "-Imagnolia/include",
            *sources,
        ],
        check=True,
    )

    return


# ----- Profile -----
def profile():

    if not has_executable("ClangBuildAnalyzer"):
        return

    print("(Python) Starting compilation profile")

    output_dir = Path(get_clang_build_dir())
    obj_dir = Path("build")
    profile_binary = "compilation_profile"
    result_dir = output_dir / profile_binary

    output_dir.mkdir(parents=True, exist_ok=True)

    # Clean first
    clean()

    # Then build
    build()

    subprocess.run(["ClangBuildAnalyzer", "--all", obj_dir, result_dir], check=True)
    subprocess.run(["ClangBuildAnalyzer", "--analyze", result_dir], check=True)

    return


# ----- Setup -----
def setup():

    if not has_executable("bear"):
        return

    print("(Python) Starting clang setup")

    output_dir = Path(get_clang_build_dir())
    output_dir.mkdir(parents=True, exist_ok=True)
    output_file = "compile_commands.json"

    # Configure first
    configure()

    # Then build
    subprocess.run(
        [
            "bear",
            "-o",
            output_dir / output_file,
            "--",
            "python",
            "scripts/build.py",
            "build",
            configuration,
        ],
        check=True,
    )

    return


def main():
    global configuration

    # Check for system support
    check_system()

    format_files()

    if len(sys.argv) != 3:
        print("(Python) Usage: <command> <configuration>")
        return

    command = str(sys.argv[1])
    configuration = str(sys.argv[2])

    # Check configuration
    check_configuration()

    if command == "setup":
        setup()

    elif command == "configure":
        configure()

    elif command == "build":
        build()

    elif command == "clean":
        clean()

    elif command == "lint":
        lint()

    elif command == "profile":
        profile()

    else:
        print(f"(Python) Invalid command: '{command}'")


main()
