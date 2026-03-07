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

def has_executable(executable):
  return shutil.which(executable) != None

# ----- Build -----
def build(system, configuration):
  executable = f"premake5" + executable_extension
  number_of_cores = get_number_of_cores()

  print(f"(Python) Number of cores: {number_of_cores} - Building {system} ({configuration})")
  assert os.system(f"ext{bar}{system}{bar}{executable} gmake && cd build && make config={configuration} -j{number_of_cores}") == 0

  return

# ----- Clean -----
def clean(configuration):
  assert os.system(f"cd build && make clean config={configuration}") == 0
  return

# ----- Format -----
def format_files():

  if not has_executable("clang-format"):
    return

  directory = "magnolia/"

  # I dislike python
  file_paths = [os.path.join(dirpath, f) for (dirpath, dirnames, filenames) in os.walk(directory) for f in filenames]
  file_paths_str = str()
  for path in file_paths:
    file_paths_str += " " + path

  os.system(f"clang-format {file_paths_str} -i -style=file")
  return

# ----- Lint -----
def lint():
  
  if not has_executable("cppcheck"):
    return
  
  cmd = "cppcheck --std=c++23 --check-level=exhaustive "
  cmd += "--output-file=build/lint_cppcheck.txt "
  cmd += "--enable=all "
  cmd += "--suppress=missingInclude --suppress=missingIncludeSystem "
  cmd += "--suppress=noExplicitConstructor --suppress=unusedFunction --suppress=unknownMacro "
  os.system(f"{cmd} -Imagnolia/include magnolia/**") == 0
  return

# ----- Profile -----
def profile(system, configuration):
  
  if not has_executable("ClangBuildAnalyzer"):
    return

  print(f"(Python) Starting compilation profile")

  output_dir = "build"
  profile_binary = "compilation_profile"
  
  # Clean first
  clean(configuration)
  
  os.system(f"mkdir -p {output_dir}")
  os.system(f"ClangBuildAnalyzer --start {output_dir}")
  
  # Then build
  build(system, configuration)

  os.system(f"ClangBuildAnalyzer --stop {output_dir} {output_dir}/{profile_binary}")
  os.system(f"ClangBuildAnalyzer --analyze {output_dir}/{profile_binary}")

  return

# ----- Setup -----
def setup(configuration):

  if not has_executable("bear"):
    return

  print(f"(Python) Starting clang setup")

  output_dir = "build"
  output_file = "compile_commands.json"
  
  # Clean first
  clean(configuration)
  
  # Then build
  os.system(f"mkdir -p {output_dir}")
  os.system(f"bear -o {output_dir}/{output_file} -- python build.py {output_dir} {configuration}")

  return

def main():

  # Check for system support
  check_system()

  format_files()

  if len(sys.argv) == 2:
    configuration = str(sys.argv[1])
    lint()
    build(system, configuration)
  
  elif len(sys.argv) < 3:
    print("Usage: <command> <configuration>")

  else:
    command = str(sys.argv[1])
    configuration = str(sys.argv[2])

    if command == "setup":
      setup(configuration)

    elif command == "build":
      build(system, configuration)
    
    elif command == "clean":
      clean(configuration)

    elif command == "lint":
      lint()
    
    elif command == "profile":
      profile(system, configuration)

    else:
      print(f"Invalid command: '{command}'")

main()
