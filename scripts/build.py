import sys
import os
import platform
import shutil
import multiprocessing
import subprocess
import glob
from pathlib import Path

system = ""

def check_system():
  global system

  supported_systems = ["windows", "linux"]
  system = platform.system().lower()

  if system not in supported_systems:
    print(f"System '{system}' is not supported. Supported systems: {supported_systems}")
    sys.exit(1)

  return

# ----- Helpers -----
def get_number_of_cores():
  return multiprocessing.cpu_count()

def has_executable(executable):
  return shutil.which(executable) != None

# ----- Build -----
def build(configuration):
  number_of_cores = get_number_of_cores()

  print(f"(Python) Number of cores: {number_of_cores} - Building {system} ({configuration})")

  cmake_build_dir = f"build/{system}/{configuration}/obj"

  # Run cmake
  result = subprocess.run([
    "cmake",
    "-S scripts",
    f"-B {cmake_build_dir}",
    f"-DCMAKE_C_COMPILER=clang",
    f"-DCMAKE_CXX_COMPILER=clang++",
    f"-DCMAKE_BUILD_TYPE={configuration}",

    # Change the linker here
    f"-DCMAKE_EXE_LINKER_FLAGS=\"-fuse-ld=mold\"", # \"-fuse-ld=gold\"
    f"-DCMAKE_SHARED_LINKER_FLAGS=\"-fuse-ld=mold\"", # \"-fuse-ld=gold\"
  ],
  check = True)

  # Run make
  result = subprocess.run([
    "make",
    f"-j{number_of_cores}"
  ],
  cwd = cmake_build_dir,
  check = True)

  return

# ----- Clean -----
def clean(configuration):
  cmake_build_dir = f"build/{system}/{configuration}/obj"

  result = subprocess.run([
    "make",
    "clean"
  ],
  cwd = cmake_build_dir,
  check = True)

  return

# ----- Format -----
def format_files():

  if not has_executable("clang-format"):
    return

  sources = list(Path("magnolia/").rglob("*.*"))

  subprocess.run([
    "clang-format",
    "-i",
    "-style=file",
    *sources
  ],
  check = True)

  return

# ----- Lint -----
def lint():
  
  if not has_executable("cppcheck"):
    return
  
  sources = list(Path("magnolia/").rglob("*.*"))

  result = subprocess.run([
    "cppcheck",
    "--std=c++23",
    "--check-level=exhaustive",
    "--output-file=build/clang/lint.txt",
    "--enable=all",
    "--suppress=missingInclude", "--suppress=missingIncludeSystem", "--suppress=useStlAlgorithm",
    "--suppress=unusedFunction", "--suppress=unknownMacro", "--suppress=unusedStructMember", 
    "-Imagnolia/include",
    *sources
  ], 
  check = True)

  return

# ----- Profile -----
def profile(configuration):
  
  if not has_executable("ClangBuildAnalyzer"):
    return

  print(f"(Python) Starting compilation profile")

  output_dir = Path("build/clang")
  obj_dir = Path("build")
  profile_binary = "compilation_profile"
  result_dir = output_dir / profile_binary

  output_dir.mkdir(parents = True, exist_ok = True)
  
  # Clean first
  clean(configuration)
  
  # Then build
  build(configuration)

  result = subprocess.run([
    "ClangBuildAnalyzer",
    "--all",
    obj_dir,
    result_dir
  ],
  check = True)

  result = subprocess.run([
    "ClangBuildAnalyzer", 
    "--analyze", 
    result_dir
  ],
  check = True)

  return

# ----- Setup -----
def setup(configuration):

  if not has_executable("bear"):
    return

  print(f"(Python) Starting clang setup")

  output_dir = Path("build/clang")
  output_dir.mkdir(parents = True, exist_ok = True)
  output_file = "compile_commands.json"

  # Then build
  subprocess.run([
    "bear",
    "-o", output_dir / output_file,
    "--",
    "python", "scripts/build.py", "build", configuration
  ],
  check = True)

  return

def main():

  # Check for system support
  check_system()

  format_files()

  if len(sys.argv) != 3:
    print("(Python) Usage: <command> <configuration>")
    return

  command = str(sys.argv[1])
  configuration = str(sys.argv[2])

  if command == "setup":
    setup(configuration)

  elif command == "build":
    build(configuration)
  
  elif command == "clean":
    clean(configuration)

  elif command == "lint":
    lint()
  
  elif command == "profile":
    profile(configuration)

  else:
    print(f"(Python) Invalid command: '{command}'")

main()
