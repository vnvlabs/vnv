version=12

find injection/src/ -iname *.h -o -iname *.cpp | xargs clang-tidy-${version} -p build --fix --fix-errors



