#!/bin/sh
#This command checks whether all source files adhere to the clang formatting rules, it does not modify anything
find -path "*vcpkg*" -prune -o \( -iname "*.c" -o -iname "*.C" -o -iname "*.c++" -o -iname "*.cc" -o -iname "*.cpp" -o -iname "*.cxx" -o -iname "*.hpp" -o -iname "*.h" -o -iname "*.hh" -o -iname "*.h++" -o -iname "*.hxx" \) -exec clang-format -Werror -n \{\} \+
