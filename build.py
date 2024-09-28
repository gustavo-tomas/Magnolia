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
  assert os.system(f"ext{bar}{system}{bar}{executable} gmake2 && cd build && make config={configuration} -j{number_of_cores}") == 0

  # Copy shared libs and executables to the same folder
  shaders_src_dir = f"build{bar}{system}{bar}shaders"
  shaders_dst_dir = f"build{bar}{system}{bar}magnolia{bar}shaders"
  
  try:
    shutil.copytree(shaders_src_dir, shaders_dst_dir, dirs_exist_ok=True)
  except Exception as e:
    print(f"Error copying folders: {e}")
  return

# ----- Run -----
def run(system, configuration):
  assert os.system(f"build{bar}{system}{bar}sprout{bar}sprout_{configuration}") == 0
  return

# ----- Clean -----
def clean(configuration):
  assert os.system(f"cd build && make clean config={configuration}") == 0
  return

# ----- Format -----
def format():
  os.system(f"find magnolia/src/ -iname *.hpp -o -iname *.cpp -o -iname *.h | xargs clang-format -i -style=file")
  os.system(f"find sprout_editor/src/ -iname *.hpp -o -iname *.cpp -o -iname *.h | xargs clang-format -i -style=file")
  return

# ----- Lint -----
def lint():
  os.system(f"cppcheck --enable=warning,performance,portability,style,information --suppress=missingInclude --std=c++20 magnolia/src/**") == 0
  return

# ----- Shaders -----
def shaders(system):
  glslc_exe = "glslc" + executable_extension
  shader_dir = f"sprout_editor{bar}assets{bar}shaders"
  output_dir = f"build{bar}{system}{bar}shaders"

  print("----- Compiling shaders -----")
  if system == "linux":
    os.system(f"mkdir -p {output_dir}")
  else:
    os.system(f"mkdir {output_dir} 2>NUL")

  # Find all shader files
  shader_files = [f for f in os.listdir(shader_dir) if f.endswith(".vert") or f.endswith(".frag")]

  # Compile shaders
  for shader_file in shader_files:
    input_path = os.path.join(shader_dir, shader_file)
    output_path = os.path.join(output_dir, f"{shader_file}.spv")
    include_path = "sprout_editor/assets/shaders"

    print(f"Compiling {input_path}")

    # @TODO: we cant use -O flags to optimize yet. Some data (for example the descriptor binding name) is still
    # needed to set uniform variables.
    assert os.system(f"ext{bar}{system}{bar}{glslc_exe} -I{include_path} {input_path} -o {output_path}") == 0
  return

def main():

  # Check for system support
  check_system()

  format()

  if len(sys.argv) == 2:
    configuration = str(sys.argv[1])
    shaders(system)
    lint()
    build(system, configuration)
    run(system, configuration)
  
  elif len(sys.argv) < 3:
    print("Usage: <command> <configuration>")

  else:
    command = str(sys.argv[1])
    configuration = str(sys.argv[2])

    if command == "build":
      shaders(system)
      lint()
      build(system, configuration)
    
    elif command == "run":
      run(system, configuration)
    
    elif command == "clean":
      clean(configuration)

    elif command == "lint":
      lint()

    else:
      print(f"Invalid command: '{command}'")

main()
