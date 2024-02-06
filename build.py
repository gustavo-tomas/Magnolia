import sys
import os
import platform
import shutil

# ----- Build -----
def build(system, configuration):
  # @TODO: lord have mercy
  executable = f"premake5"
  bar = "/"
  if system == "windows":
    executable += ".exe"
    bar = "\\"
  assert os.system(f"ext{bar}{system}{bar}{executable} gmake2 && cd build && make config={configuration} -j4") == 0
  
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
  bar = "/"
  if system == "windows":
    bar = "\\"
  assert os.system(f"build{bar}{system}{bar}magnolia{bar}magnolia_{configuration}") == 0
  return

# ----- Clean -----
def clean(configuration):
  assert os.system(f"cd build && make clean config={configuration}") == 0
  return

# ----- Format -----
def format():
  os.system(f"find magnolia/src/ -iname *.hpp -o -iname *.cpp -o -iname *.h | xargs clang-format -i -style=file")
  return

# ----- Shaders -----
def shaders(system):
  bar = "/"
  if system == "windows":
    bar = "\\"
  shader_dir = f"assets{bar}shaders"
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
    print(f"Compiling {input_path}")

    if system == "linux":
      assert os.system(f"ext/linux/glslc {input_path} -o {output_path}") == 0
    
    elif system == "windows":
      assert os.system(f"ext\\windows\\glslc.exe {input_path} -o {output_path}") == 0
  return

def main():

  # Check for system support
  supported_systems = ["windows", "linux"]
  system = platform.system().lower()

  print(f"System: {system}\n")
  assert system in supported_systems, f"System '{system}' is not supported\n"

  format()

  if len(sys.argv) == 2:
    configuration = str(sys.argv[1])
    shaders(system)
    build(system, configuration)
    run(system, configuration)
  
  elif len(sys.argv) < 3:
    print("Usage: <command> <configuration>")

  else:
    command = str(sys.argv[1])
    configuration = str(sys.argv[2])

    if command == "build":
      build(system, configuration)
    
    elif command == "run":
      run(system, configuration)
    
    elif command == "clean":
      clean(configuration)

    else:
      print(f"Invalid command: '{command}'")

main()
