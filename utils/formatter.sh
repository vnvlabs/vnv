version=12

echo "Fomatting injection/src"
find injection/src/ -iname *.h -o -iname *.cpp | xargs clang-format-${version} -style=file -i

echo "Formatting injection/include"
find injection/include/ -iname *.h -o -iname *.cpp | xargs clang-format-${version} -style=file -i

echo "Formatting arguments"
find examples/ -iname *.h -o -iname *.cpp | xargs clang-format-${version} -style=file -i

echo "Formatting extraction"
find extraction/ -iname *.h -o -iname *.cpp | xargs clang-format-${version} -style=file -i


