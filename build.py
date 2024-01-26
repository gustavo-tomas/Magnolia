import sys
import os
import platform

# ----- Build -----
def build(system, configuration):
  # @TODO: lord have mercy
  executable = f"premake5"
  bar = "/"
  if system == "windows":
    executable += ".exe"
    bar = "\\"
  assert os.system(f"ext{bar}{system}{bar}{executable} gmake2 && cd build && make config={configuration} -j4") == 0
  return

# ----- Run -----
def run(system, configuration):
  bar = "/"
  if system == "windows":
    bar = "\\"
  assert os.system(f"build{bar}{system}{bar}magnolia_{configuration}") == 0
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
  shader_dir = "assets/shaders"
  output_dir = "build/shaders"

  print("----- Compiling shaders -----")
  os.system(f"mkdir -p {output_dir}")

  # Find all shader files
  shader_files = [f for f in os.listdir(shader_dir) if f.endswith(".vert") or f.endswith(".frag")]

  # Compile shaders
  for shader_file in shader_files:
      input_path = os.path.join(shader_dir, shader_file)
      output_path = os.path.join(output_dir, f"{shader_file}.spv")
      print(f"Compiling {input_path}")

      if system == "linux":
        assert os.system(f"ext/linux/glslc {input_path} -o {output_path}") == 0
      
      # @TODO: add windows glslc
      elif system == "windows":
        print("MISSING WINDOWS GLSL.EXE")
        assert false
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
    build(system, configuration)
    shaders(system)
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
