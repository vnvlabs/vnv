

echo "Fomatting injection/src"
find injection/src/ -iname *.h -o -iname *.cpp | xargs clang-format-9 -style=file -i

echo "Formatting injection/include"
find injection/include/ -iname *.h -o -iname *.cpp | xargs clang-format-9 -style=file -i

echo "Formatting arguments"
find examples/ -iname *.h -o -iname *.cpp | xargs clang-format-9 -style=file -i

echo "Formatting extraction"
find extraction/ -iname *.h -o -iname *.cpp | xargs clang-format-9 -style=file -i

echo "Formatting VnV Python code"
find generation/ -iname *.py | xargs autopep8 --in-place --aggressive --aggressive 

echo "Formatting all CMakeFiles"
find . -iname CMakeLists.txt | xargs cmake-format --config-files .cmake-format --in-place
