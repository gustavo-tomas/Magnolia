import sys
import os
import platform
import shutil
import multiprocessing

# @TODO: lord have mercy
system = ""
bar = ""
executable_extension = ""

def check_system():
  global system
  global bar
  global executable_extension

  supported_systems = ["windows", "linux"]
  system = platform.system().lower()

  print(f"System: {system}\n")
  assert system in supported_systems, f"System '{system}' is not supported\n"

  if system == "linux":
    bar = "/"
    executable_extension = ""

  elif system == "windows":
    bar = "\\"
    executable_extension = ".exe"

  return

# ----- Helpers -----
def get_number_of_cores():
  return multiprocessing.cpu_count()

# ----- Build -----
def build(system, configuration):
  executable = f"premake5" + executable_extension
  number_of_cores = get_number_of_cores()

  print(f"(Python) Number of cores: {number_of_cores}")
  assert os.system(f"ext{bar}{system}{bar}{executable} gmake && cd build && make config={configuration} -j{number_of_cores}") == 0

  return

# ----- Clean -----
def clean(configuration):
  assert os.system(f"cd build && make clean config={configuration}") == 0
  return

# ----- Format -----
def format():
  os.system(f"find magnolia/src/ -iname *.hpp -o -iname *.cpp -o -iname *.h | xargs clang-format -i -style=file")
  return

# ----- Lint -----
def lint():
  os.system(f"cppcheck --enable=warning,performance,portability,style,information --suppress=missingInclude --std=c++20 magnolia/src/**") == 0
  return

def main():

  # Check for system support
  check_system()

  format()

  if len(sys.argv) == 2:
    configuration = str(sys.argv[1])
    lint()
    build(system, configuration)
  
  elif len(sys.argv) < 3:
    print("Usage: <command> <configuration>")

  else:
    command = str(sys.argv[1])
    configuration = str(sys.argv[2])

    if command == "build":
      build(system, configuration)
    
    elif command == "clean":
      clean(configuration)

    elif command == "lint":
      lint()

    else:
      print(f"Invalid command: '{command}'")

main()
