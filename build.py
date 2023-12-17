import sys
import os

# ----- Build -----
def build(system, configuration):
  # @TODO: resolve system
  assert os.system(f"libs/premake/premake5_linux gmake2 && make config={configuration} -j4") == 0
  return

# ----- Run -----
def run(system, configuration):
  # @TODO: resolve system
  os.system(f"bin/linux/{configuration}/magnolia") == 0
  return

# ----- Clean -----
def clean(configuration):
  os.system(f"make clean config={configuration}") == 0
  return

def main():
  if len(sys.argv) == 2:
    configuration = str(sys.argv[1])
    build("@TODO", configuration)
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
