import sys
import os

# ----- Build -----
def build(system, configuration):
  # @TODO: resolve system
  assert os.system(f"libs/premake/premake5_linux gmake2 && cd build && make config={configuration} -j4") == 0
  return

# ----- Run -----
def run(system, configuration):
  # @TODO: resolve system
  assert os.system(f"build/linux/{configuration}/magnolia") == 0
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
def shaders():
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
      os.system(f"glslc {input_path} -o {output_path}")
  return

def main():

  format()

  if len(sys.argv) == 2:
    configuration = str(sys.argv[1])
    build("@TODO", configuration)
    shaders()
    run("@TODO", configuration)
  
  elif len(sys.argv) < 3:
    print("Usage: <command> <configuration>")

  else:
    command = str(sys.argv[1])
    configuration = str(sys.argv[2])

    if command == "build":
      build("@TODO", configuration)
    
    elif command == "run":
      run("@TODO", configuration)
    
    elif command == "clean":
      clean(configuration)

    else:
      print(f"Invalid command: '{command}'")

main()
