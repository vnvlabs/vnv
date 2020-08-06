#!/usr/bin/env python3
import os
import pathlib
import re
import sys

if len(sys.argv) < 2:
  print('''This simple python script creates a package specific header file for the current
  vnv toolkit. Run it with ./subpackage.py <PACKAGENAME>  to generate a VnV.h
  header file for the given package Name. This removes the need to specify your package name
  at the front on every VnV.h call. 

  On the flipside, this will tie your code to the chosen packageName. Up to you. 

  Usage: ./subpackage.py #PACKAGENAME# > <outfilename> 
  
  If you want to prefix everything to avoid conflicts: Do this. 
  
  Usage: ./subpackage.py #PACKAGENAME# #PREFIX#  


  Add outfilename to a directory where the compiler can find it and include it instead
  of VnV.h whereever needed. ''')
  exit()

pname = sys.argv[1]
prefix = sys.argv[2] if len(sys.argv) > 2 else pname 

current_dir = pathlib.Path(__file__).parent.absolute()
r = re.compile(r'#define[ \t]+INJECTION_(.*?)\((.*?)\)')
define = "#define {prefix}_{name}({args}) INJECTION_{name}({pname}{comma}{args})"

def getDefine(name,args):
    sp = args.split(",")
    com = "," if len(sp)>1 else ""
    return define.format(prefix=prefix, pname=pname,name=name,args=",".join(sp[1:]), comma=com)

print("#include VnV.h \n")
with open(os.path.join(current_dir, "macros.info")) as f:
   for line in f:
       for match in re.finditer(r,line):
         print(getDefine(match.group(1),match.group(2)))

