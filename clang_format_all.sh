#!/bin/sh
#This command applies clang formatting to all te source files
find -path "*vcpkg*" -prune -o \( -iname "*.c" -o -iname "*.C" -o -iname "*.c++" -o -iname "*.cc" -o -iname "*.cpp" -o -iname "*.cxx" -o -iname "*.hpp" -o -iname "*.h" -o -iname "*.hh" -o -iname "*.h++" -o -iname "*.hxx" \) -exec clang-format -i \{\} \+
