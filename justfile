# @TODO: add windows configuration

set positional-arguments

@default: (build "debug") (run "debug")

@debug:   (lint) (format) (build "debug")   (run "debug")
@profile: (lint) (format) (build "profile") (run "profile")
@release: (lint) (format) (build "release") (run "release")

@lint:
  cppcheck --enable=warning,performance,portability,style,information --disable=missingInclude --std=c++20 magnolia/src/**

@format:
  find magnolia/src/ -iname *.hpp -o -iname *.cpp | xargs clang-format -i -style=file

@build cfg:
  ./libs/premake/premake5_linux gmake2 && make config=$1 -j4

@run cfg:
  bin/linux/$1/magnolia

@clean cfg:
  make clean config=$1
