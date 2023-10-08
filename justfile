# @TODO: add windows configuration

set positional-arguments

@default: (build "profile") (run "profile")

@debug: (build "debug") (run "debug")
@profile: (build "profile") (run "profile")
@release: (build "release") (run "release")

@build cfg:
  ./libs/premake/premake5_linux gmake2 && make config=$1 -j4

@run cfg:
  bin/linux/$1/magnolia

@clean cfg:
  make clean config=$1
