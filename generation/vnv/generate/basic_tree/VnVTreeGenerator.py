# -*- coding: utf-8 -*-
import os
from io import StringIO
from enum import Enum
from .. import RestUtils
from . import VnVTreeGeneratorSchema
from jsonschema import ValidationError, validate
from ...vnv import VnVReader
from .. import configs
import json
import subprocess
import datetime

def JMES(jmes):
    return ".. vnv-jmes-path::\n  {}\n\nThis is the data value :vnv:`$`.\n\n".format(json.dumps(jmes))

def generateAllTrees(allTrees, outDir, filename):
     stream = StringIO()
     stream.write(RestUtils.getHeader("All Available Output Files"))
     tocTree = []
     for f in configs.getOutputFiles().values():
            tocTree.append(f.generateTree())
     stream.write(RestUtils.tocTree(tocTree))
     return RestUtils.writeFile(outDir,filename, stream.getvalue())

class VnVTreeGenerator:
     def config(self, **kwargs):
          try:
               validate(instance=kwargs, schema=VnVTreeGeneratorSchema.vnvTreeSchema)
          except ValidationError as e:
               print(e)
               raise RuntimeError("Invalid Configuration")
          self.jsonInfo = kwargs
          self.templates = self.jsonInfo.get("templates",{})
          self.jmesRoot = [self.jsonInfo.get("__jmes_root__","root")]

     def getTemplate(self, type, package, name, id):
          if type in self.templates:
               packages = self.templates[type]
               if package is packages and name in packages[package]:
                    temp = packages[package][name]
                    if "text" in temp:
                         return temp["text"]
                    elif "file" in temp:
                         return RestUtils.readFile(temp["file"])
                    else:
                         return None #kwargs validation should catch this case. .
          return None

     def getInformationText(self):
          return self.templates.get("Information Text")

     def visitTestNode(self,outputDirectory,node,jmespath, userArgs):
          if not userArgs.get("display",True):
               return None

          stream = StringIO()
          stream.write(RestUtils.getHeader(userArgs.get("title","Injection Point Test: {}::{}".format( node.getPackage(),node.getName()))))
          stream.write(JMES(jmespath))

          tocTree = []

          # Write the description for the unit test as fetched from the configs
          if "template" in userArgs:
               template = userArgs["template"]
          else:
               template = self.getTemplate("test",node.getPackage(),node.getName(),node.getId())

          tstream = StringIO()
          tstream.write(RestUtils.getHeader("Description"))
          tstream.write(JMES(jmespath))

          if template is None:
               tstream.write("This is an undocumented test executed as part of the {} package".format(node.getPackage()))
               tstream.write("Available Data Points are:")
               data = []
               for d in node.getData():
                    data.append("Name: {}, Type: {}".format(d.getId(),d.getType()))
               tstream.write(RestUtils.htmllist(data))
          else:
               tstream.write(template)
          tocTree.append(RestUtils.writeFile(outputDirectory,"Description",tstream.getvalue()))

          #Write the logs. This is stuff written out during the test.
          outDir = os.path.join(outputDirectory, "children")
          jmes = jmespath + [".Children"]
          tocTree.append(self.ftt("children", self.visitChildren(outDir, node.getChildren(), jmes, userArgs.get("children",{}))))
          stream.write(RestUtils.tocTree(tocTree))

          return RestUtils.writeFile(outputDirectory,userArgs.get("filename",node.getId()), stream.getvalue())

     def visitTests(self, outputDirectory, node, jmespath, userArgs):
          if not userArgs.get("display",True):
               return None

          stream = StringIO()
          stream.write(RestUtils.getHeader(userArgs.get("title","Injection Point Tests")))
          stream.write(JMES(jmespath))

          # Write the toc tree.
          tocTree = []
          targs = userArgs.get("children",{})
          for child in node:
               outDir = os.path.join(outputDirectory,child.getId())
               jmes = jmespath + ["[?Id == '{}'] | [0]".format(child.getId())]
               tocTree.append(self.ftt(child.getId(), self.visitTestNode(outDir, child, jmes, targs.get(child.getId(),{}))))

          stream.write(RestUtils.tocTree(tocTree))
          return RestUtils.writeFile(outputDirectory, "Tests", stream.getvalue())

     def visitInjectionPointNode(self,outputDirectory,node,jmespath,userArgs):
          if not userArgs.get("display",True):
               return None

          stream = StringIO()
          stream.write(RestUtils.getHeader(userArgs.get("title","Injection Point: {}::{}".format(node.getPackage(),node.getName()))))
          stream.write(JMES(jmespath))

          tocTree = []

          #Write the description.
          tstream= StringIO()
          tstream.write(RestUtils.getHeader("Description"))
          tstream.write(JMES(jmespath))

          if "template" in userArgs:
               template = userArgs["template"]
          else:
               template  = self.getTemplate("InjectionPoint",node.getPackage(), node.getName(), node.getId())
          if template is None:
               tstream.write("Undocumented Injection point from Package {}".format(node.getPackage()))
          else:
               tstream.write(template)
          tocTree.append(RestUtils.writeFile(outputDirectory,"Description", tstream.getvalue()))

          #Write the tests:
          outDir = os.path.join(outputDirectory, "tests")
          jmes = jmespath + [".Tests"]
          tocTree.append(self.ftt("tests", self.visitTests(outDir, node.getTests(), jmes, userArgs.get("tests",{}))))

          #Write the logs.
          outDir = os.path.join(outputDirectory, "children")
          jmes = jmespath + [".Children"]
          tocTree.append(self.ftt("children",self.visitChildren(outDir, node.getChildren(), jmes, userArgs.get("children",{}))))
          stream.write(RestUtils.tocTree(tocTree))

          # Write the children

          return RestUtils.writeFile(outputDirectory,userArgs.get("filename",node.getId()), stream.getvalue())


     def visitLogNode(self, outputDirectory, node, jmespath, userArgs):
          if not userArgs.get("display",True):
               return None

          stream = StringIO()
          stream.write(RestUtils.getHeader("Log Point"))
          stream.write(JMES(jmespath))
          stream.write("Log Point {}:{}\n".format(node.getPackage(), node.getLevel()))
          stream.write("``{}``\n".format(node.getMessage()));
          #stream.write("[{}:{}] {}".format(node.getPackage(),node.getStage(),node.getMessage()))
          return RestUtils.writeFile(outputDirectory,node.getId(), stream.getvalue())

     def visitDocumentationNode(self,outputDirectory, node, jmespath, userArgs):
          if not userArgs.get("display",True):
               return None

          stream = StringIO()
          stream.write(RestUtils.getHeader(userArgs.get("title","Documetation Point: {}::{}".format(node.getPackage(),node.getName()))))
          stream.write(JMES(jmespath))

          template = userArgs.get("template")
          if template is None:
               template  = self.getTemplate("Documentation",node.getPackage,node.getName(),node.getId())

          if template is None:
               stream.write("Undocumented Documentation point from Package {}".format(node.getPackage()))
               stream.write("Available Data Points are:")
               darray = node.getData()
               if darray is not None:
                    data = []
                    for d in node.getData():
                         data.append("Name: {}, Type: {}".format(d.getName(),VnVReader.type2Str[d.getType()]))
                    stream.write(RestUtils.htmllist(data))
          else:
               stream.write(template)

          return RestUtils.writeFile(outputDirectory,userArgs.get("filename",node.getId()), stream.getvalue())

     def visitChildren(self, outputDirectory, node, jmespath, userArgs):
          if not userArgs.get("display",True):
               return None

          stream = StringIO()
          stream.write(RestUtils.getHeader(userArgs.get("title","Children")))
          stream.write(JMES(jmespath))

          tocTree = []
          cargs = userArgs.get("children",{})
          for child in node:
               jmes = jmespath + ["[?Id == '{}'] | [0]".format(child.getId())]
               outDir = os.path.join(outputDirectory,child.getId())
               if VnVReader.type2Str[child.getType()] == "Log":
                    tocTree.append(self.ftt(child.getId(),self.visitLogNode(outDir, child, jmes, cargs.get(child.getId(),{}))))
               elif VnVReader.type2Str[child.getType()] == "Documentation":
                    tocTree.append(self.ftt(child.getId(),self.visitDocumentationNode(outDir, child, jmes, cargs.get(child.getId(),{}))))
               elif VnVReader.type2Str[child.getType()] == "InjectionPoint":
                    tocTree.append(self.ftt(child.getId(),self.visitInjectionPointNode(outDir, child, jmes, cargs.get(child.getId(),{}))))
               else:
                    print("{} not supported by this generator. Ignoring".format(VnVReader.type2Str[child.getType()]))

          stream.write(RestUtils.tocTree(tocTree))
          return RestUtils.writeFile(outputDirectory, userArgs.get("filename","Children"), stream.getvalue())

     def visitInfoNode(self, outputDirectory, node , jmespath, userArgs):
          if not userArgs.get("display",True):
               return None

          stream = StringIO()
          stream.write(RestUtils.getHeader(userArgs.get("title","Runtime Information")))
          stream.write(JMES(jmespath))
          if "template" in userArgs:
               template = userArgs["template"]
          else:
               template = self.getInformationText()

          if template is None:
               stream.write("Title: {}".format(node.getTitle()))
               stream.write("Date: {}".format(node.getDate()))
          else:
               stream.write(template)

          return RestUtils.writeFile(outputDirectory, userArgs.get("filename","info"), stream.getvalue())


     def visitUnitTestNode(self, outputDirectory, node, jmespath, userArgs):
         if not userArgs.get("display",True):
               return None

         stream = StringIO()
         stream.write(RestUtils.getHeader(userArgs.get("title",node.getName())))
         stream.write(JMES(jmespath))

         tocTree = []

         # Write the description for the unit test.
         if "template" in userArgs:
              template = userArgs["template"]
         else:
             template = self.getTemplate("unit-test",node.getPackage(),node.getName(), node.getId())

         tstream = StringIO()
         tstream.write(RestUtils.getHeader("Description"))
         tstream.write(JMES(jmespath))

         if template is None:
              tstream.write("This is an undocumented unit test executed as part of the {} package".format(node.getPackage()))
         else:
              tstream.write(template)

         tocTree.append(RestUtils.writeFile(outputDirectory,"Description",tstream.getvalue()))

         #Write the results for the unit Test.
         res = []
         for result in node.getResults():
              res.append("{} ... {}".format( result, "PASS" if node.getResults()[result] else "FAIL"))
         rstream = StringIO()
         rstream.write(RestUtils.getHeader("Results"))
         rstream.write(JMES(jmespath + [".Results"]))
         rstream.write(RestUtils.htmllist(res))
         tocTree.append(RestUtils.writeFile(outputDirectory,"Results",rstream.getvalue()))


         #Write the logs.
         outDir = os.path.join(outputDirectory, "children")
         jmes = jmespath + [".Children"]
         tocTree.append(self.ftt("children",self.visitChildren(outDir, node.getChildren(), jmes, userArgs.get("children",{}))))

         #Write the tocTree
         stream.write(RestUtils.tocTree(tocTree))

         return RestUtils.writeFile(outputDirectory, userArgs.get("filename",node.getName()), stream.getvalue())

     def visitUnitTests(self, outputDirectory, node, jmespath, userArgs):
          if not userArgs.get("display",True):
               return None

          stream = StringIO()
          stream.write(RestUtils.getHeader(userArgs.get("title","Unit Testing")))
          stream.write(JMES(jmespath))

          packages = {}
          for unitTest in node:
               package = unitTest.getPackage()
               name = unitTest.getName()
               if package not in packages:
                    packages[package] = []
               packages[package].append(unitTest)

          tocTree = []
          for package in packages:

               pargs = userArgs.get("packages",{}).get(package,{})
               if not pargs.get("display",True):
                    continue

               outDir = os.path.join(outputDirectory,package)
               pstream = StringIO()
               pstream.write(RestUtils.getHeader(pargs.get("title","Unit Testing for {}".format(package))))
               pstream.write(JMES(jmespath))

               ptocTree = []
               pnodes = pargs.get("unitTests",{})
               for test in packages[package]:
                    poutDir = os.path.join(outDir,test.getId())
                    pjmes = jmespath + ["[?Id=='{}'] | [0]".format(test.getId())]
                    ptocTree.append(self.ftt(test.getId(),self.visitUnitTestNode(poutDir, test, pjmes, pnodes.get(test.getId(),{}))))

               pstream.write(RestUtils.tocTree(ptocTree))
               tocTree.append(self.ftt(package,RestUtils.writeFile(outDir, pargs.get("filename",package), pstream.getvalue())))

          stream.write(RestUtils.tocTree(tocTree))
          return RestUtils.writeFile(outputDirectory, userArgs.get("filename","unit-tests"), stream.getvalue())

     def ftt(self, parent, toc):
          if isinstance(toc,str):
               return os.path.join(parent,toc)
          else:
               return [toc[0], os.path.join(parent,toc)]


     def visitRootNode(self, outputDirectory, rootNode, jmespath, userArgs):
          stream = StringIO()
          stream.write(RestUtils.getHeader(userArgs.get("title","RunTime Report")))
          stream.write(JMES(jmespath))

          tocTree = []

          outDir = os.path.join(outputDirectory,"info")
          jmes = jmespath + [".InfoNode"]
          tocTree.append(self.ftt("info", self.visitInfoNode(outDir, rootNode.getInfoNode(), jmes, userArgs.get("infoNode",{}))))

          outDir = os.path.join(outputDirectory,"unit-tests")
          jmes = jmespath + [".UnitTests"]
          tocTree.append(self.ftt("unit-tests",self.visitUnitTests(outDir, rootNode.getUnitTests(), jmes, userArgs.get("unitTests",{}))))

          outDir = os.path.join(outputDirectory,"Children")
          jmes = jmespath + [".Children"]
          tocTree.append(self.ftt("Children",self.visitChildren(outDir, rootNode.getChildren(), jmes, userArgs.get("children",{}))))

          stream.write(RestUtils.tocTree(tocTree))
          return RestUtils.writeFile(outputDirectory, userArgs.get("filename","root-node"), stream.getvalue())

     def generateTree(self, outputDirectory, rootNode):
          self.dir = RestUtils.createOutputDirectory(outputDirectory)

          stream = StringIO()

          #Write the tile
          stream.write(RestUtils.getHeader(self.jsonInfo.get("title","VnV Simulation Report")))
          stream.write(JMES(self.jmesRoot))

          #Write any provided bib info.
          bibInfo = self.jsonInfo.get("bib",{})

          for i in ["Author","Authors","Organization","Contact","Address","Version","Status","Date","Copyright"]:
               if i in bibInfo:
                    stream.write(":{}: {}".format(i, jsonInfo.get(i)))

          # Write the abstract and.or dedication
          for i in ["Dedication","Abstract"]:
               if i in bibInfo:
                   stream.write(":{}:".format(i))
                   val = jsonInfo[i].splitlines()
                   for line in val:
                        stream.write("  {}".format(line.lstrip()))
          # Create a new tocTree list
          tocTree = []

          #Write the introduction file
          DEFAULT_INTRODUCTION = "{}\n\n VnV Simulation Report".format(RestUtils.getHeader("Introduction"))
          intro = self.jsonInfo.get("beforeRootNode",[{"filename":"Introduction", "text":DEFAULT_INTRODUCTION}])

          for i in intro:
               tocTree.append(RestUtils.writeFile(self.dir, i["filename"], i["text"]))

          #Write the rootNode file.
          outDir = os.path.join(self.dir, "rootNode")
          jmes = self.jmesRoot
          tocTree.append(self.ftt("rootNode",self.visitRootNode(outDir, rootNode, jmes, self.jsonInfo.get("rootNode",{}))))

          #Write the Conclusion file.
          DEFAULT_CONCLUSION = "{}\n\nEnd VnV Simulation Report".format(RestUtils.getHeader("Conclusion"))
          conc = self.jsonInfo.get("afterRootNode",[{"filename":"Conclusion","text" : DEFAULT_CONCLUSION}])
          for i in conc:
               tocTree.append(RestUtils.writeFile(self.dir, i["filename"], i["text"] ))

          #Write the toc tree itself to the stream.
          stream.write(RestUtils.tocTree(tocTree))
          return RestUtils.writeFile(self.dir, self.jsonInfo.get("filename","index"),stream.getvalue())

     def genUnitTestNode(self,node):
          ch = {}
          for child in node.getChildren():
               ch[child.getId()] = self.genChildConfigNode(child)
          return {
                 "display" : True,
                 "title" : "{}::{}".format(node.getPackage(),node.getName()) ,
                 "filename" : node.getId(),
                 "children" : ch,
                 "results" : [ x for x in node.getResults() ]
          }

     def genTestNode(self,node):
          ch = {}
          for child in node.getChildren():
               ch[child.getId()] = self.genChildConfigNode(child)
          return {
                 "display" : True,
                 "title" : "{}::{}".format(node.getPackage(),node.getName()) ,
                 "filename" : node.getId(),
                 "children" : ch
          }


     def genChildConfigNode(self, node):
        if VnVReader.type2Str[node.getType()] == "Log":
             return { "display" : True }
        elif VnVReader.type2Str[node.getType()] == "Documentation" :
             return { "display" : True }
        else:
             children = []
             for i in node.getChildren():
                  children.append(self.genChildConfigNode(i))

             tests = {}
             for i in node.getTests():
                  tests[i.getId()] = self.genTestNode(i)

             return  {
                 "title" : "{}::{}".format(node.getPackage(),node.getName()),
                 "display" : True,
                 "filename" : "{}".format(node.getId()),
                 "children" : children,
                 "tests" : tests
             }

     def generateConfigFile(self,rootNode):
          out =      VnVTreeGeneratorSchema.defaultIntro
          #out = VnVTreeGenerationSchema.defaultIntro
          rout = out["rootNode"]

          cout = rout["children"]["children"]

          for child in rootNode.getChildren():
             cout[child.getId()] = self.genChildConfigNode(child)

          tout = rout["unitTests"]["packages"]
          packages = {}
          for unitTest in rootNode.getUnitTests():
               package = unitTest.getPackage()
               name = unitTest.getName()
               if package not in packages:
                    packages[package] = []
               packages[package].append(unitTest)
          for package in packages:
               unitTests = {}
               for test in packages[package]:
                  unitTests[test.getId()] = self.genUnitTestNode(test)

               tout[package] = {
                  "filename" : package,
                  "display" : True,
                  "unitTests" : unitTests
               }
          # VnVTreeGeneratorSchema.validateJson(out)
          try:
               validate(instance=out, schema=VnVTreeGeneratorSchema.vnvTreeSchema)
          except:
               print("Validation Failed On Generated Script -- Please post a bug report:")
          return json.dumps(out,indent=4, separators=(',', ': '))
if __name__ == "__main__":
     print("hello")
