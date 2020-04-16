import sys

try:
  import VnVReader as OR
except:
  print("Failed to Load VnVReader -- Are you using the python3?")
  exit()


import jmespath
import RootNodeVisitor as RI
X = OR.getRootNode()#readOutputFile("sfsdfsdf","sfsdfsdf")

c = jmespath.compile("Children[1]")
print(RI.search(c,X))





