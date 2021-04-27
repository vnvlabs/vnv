from io import StringIO
from pathlib import Path

import sphinx
from sphinx.util.console import nocolor
from sphinx.cmd.quickstart import *

from ...vnv import VnVReader
from .. import RestUtils
import textwrap
import os,sys

#Write the template from the node -- this is the comment above the text.
def writeTemplate(inode, stream):
    node = VnVReader.castDataBase(inode)

    val = node.getValue() if hasattr(node, "getValue") else ""
    stream.write("\n\n")
    stream.write(textwrap.dedent(val))
    stream.write("\n\n")

def pushNode(node, stream):
    stream.write("\n\n.. vnv-push-node:: " + str(node.getId()) + "\n\n")

def popNode(stream):
    stream.write("\n\n.. vnv-pop-node:: \n\n")


class RSTGenerator:

    def dispatcher(self, node):
        typeStr = VnVReader.type2Str[node.getType()]
        m = getattr(self, "visit" + typeStr, None)
        if m is None:
            return self.visitGenericNode(node)
        else:
            return m(VnVReader.castDataBase(node))

    def visitGenericNode(self, node):
        stream = StringIO()
        pushNode(node, stream)
        writeTemplate(node, stream)
        popNode(node, stream)
        return RestUtils.writeFile(self.dir,node.getId(),stream.getvalue(),textName=node.getName())

    def visitLog(self, node: VnVReader.ILogNode):
        stream = StringIO()
        pushNode(node,stream)
        stream.write("\n\n.. vnv-log:: {}\n\n".format(node.getId()))
        popNode(node,stream)
        return RestUtils.writeFile(self.dir,node.getId(),stream.getvalue(),textName=node.getMessage())

    def writeLogs(self, node):
        return self.writeList(node, "Logs", "\n\n.. vnv-logs::\n\n")

    def writeInjectionPoints(self, node):
        return self.writeList(node,"Injection Points", "\n\n.. vnv-injection-points::\n\n")

    def writeTests(self, node):
        return self.writeList(node, "Tests", "\n\n.. vnv-tests::\n\n")

    def writeIterators(self, node):
        return self.writeList(node, "Iterators", "\n\n.. vnv-iterators::\n\n")

    def writeList(self,node,textName,intro):
        stream = StringIO()
        pushNode(node, stream)
        stream.write(intro)

        # Write the logs (these are the children
        tocTree = []
        for i in node.getChildren():
            tocTree += self.dispatcher(node)

        stream.write("\n\n")
        stream.write(RestUtils.tocTree(tocTree))
        popNode(stream)
        return RestUtils.writeFile(self.dir, node.getId(), stream.getvalue(), textName=textName)

    def visitTest(self, node: VnVReader.ITestNode):
        stream = StringIO()
        pushNode(node,stream) # make this node the current node.
        # Write the test
        writeTemplate(node, stream)

        if len(node.getChildren()) > 0:
            tocTree = self.writeLogs(node.getChildren())
            stream.write(RestUtils.tocTree(tocTree))

        popNode(stream)
        return RestUtils.writeFile(self.dir,node.getId(),stream.getvalue(),textName=node.getName())

    def visitInjectionPoint(self, node: VnVReader.IInjectionPointNode):
        stream = StringIO()
        pushNode(node,stream)
        writeTemplate(node, stream)
        iptoc = []
        if len(node.getTests()) > 0:
            iptoc = iptoc + self.writeTests(node.getTests())
        if len(node.getIterators()) > 0:
            iptoc = iptoc + self.writeIterators(node.getIterators())
        if len(node.getChildren()) > 0:
            iptoc = iptoc + self.writeInjectionPoints(node.getChildren())

        stream.write(RestUtils.tocTree(iptoc))
        popNode(stream)
        popNode(stream)
        return RestUtils.writeFile(self.dir, node.getId(), stream.getvalue(), textName=node.getName())

    def writeTestResults(self,node):
        stream = StringIO()
        stream.write("\n\n.. vnv-test-results:\n\n")
        for testName in node.getResults():
            test = node.getResults()[testName]
            m = node.getTestTemplate(test.getName())

            stream.write("\t ..vnv-test-result:\n")
            stream.write("\t\t:name: {}\n".format(test.getName()))
            stream.write("\t\t:result: {}\n\n".format(test.getValue()))
            stream.write(textwrap.indent(textwrap.dedent(m), "\t\t"))

        return RestUtils.writeFile(self.dir, node.getId()+"_test-results",stream.getvalue(),textName="Test Results")

    def visitUnitTest(self, node: VnVReader.IUnitTestNode):
        stream = StringIO()
        pushNode(node,stream)
        writeTemplate(node, stream)

        # Write the logs that occurred during the tests.
        tocTree = self.writeTestResults(node)

        if len(node.getChildren()) > 0:
            tocTree += self.writeLogs(node.getChildren)

        stream.write(RestUtils.tocTree(tocTree))
        popNode(stream)
        return RestUtils.writeFile(self.dir, node.getId(),stream.getvalue(),textName=node.getName())

    def visitInfo(self, node):
        stream = StringIO()
        pushNode(node,stream)
        writeTemplate(node, stream)
        popNode(stream)
        return RestUtils.writeFile(self.dir, node.getId(),stream.getvalue(),textName="Information")

    def __init__(self, filename, key, reader="json", **kwargs ):
        # Read the file -- We do this to walk the heirarchy
        self.node = VnVReader.Read(filename, reader , "{}")
        self.filename = filename
        self.reader = reader
        self.key = key
        self.dir = os.path.join(kwargs.get("dir",""), key)
        os.makedirs(self.dir)

    def writeFile(self):
        stream = StringIO()
        stream.write("\n\n.. vnv-read: {} {} {} \n\n".format(self.key,self.reader,self.filename))
        stream.write("\n\n.. vnv-push-file: {} \n\n".format(self.key))
        pushNode(self.node,stream)

        tocTree = []

        #Write the introduction
        introText = textwrap.dedent(self.node.getIntro())
        tocTree += RestUtils.writeFile(self.dir,"intro", introText, textName="Introduction")

        # Write any information section
        tocTree += self.dispatcher(self.node.getInfoNode())

        # Add the unit testing report for the node.
        if len(self.node.getUnitTests()) > 0:
            tocTree += self.dispatcher(self.node.getUnitTests())

        # Add the children nodes
        if len(self.node.getChildren()) > 0:
            tocTree += self.writeInjectionPoints(self.node.getChildren())

        #Write the conclusion.
        concText = textwrap.dedent(self.node.getConclusion())
        tocTree += RestUtils.writeFile(self.dir, "conclusion", concText, textName="Conclusion")

        stream.write(RestUtils.tocTree(tocTree))
        popNode(stream)
        stream.write("\n\n.. vnv-pop-file: {} \n\n".format(self.key))
        return RestUtils.writeFile(self.dir, "index", stream.getvalue(), textName="Vnv Simulation Report: {}".format(self.key))

def writeSingleFile(file, key, reader="json", **kwargs):

    VnVReader.Initialize([],{
            "runTests": True,
            "runScopes": [
                {
                    "name": "all",
                    "run": True
                }
            ],
            "logging": {
                "on": True,
                "filename": "stdout",
                "logs": {}
            },
            "additionalPlugins": kwargs.get("plugins",{}),
            "outputEngine": {
                "type": "json",
                "config": {}
            },
            "injectionPoints": [
            ]
    })

    generator = RSTGenerator(file, key, reader, **kwargs)
    files = generator.writeFile()
    print("Wrote file {} (with title {})" , files[0], files[1])



def vnv_main(argv:List[str] = sys.argv[1:]) -> dict:
    sphinx.locale.setlocale(locale.LC_ALL, '')
    sphinx.locale.init_console(os.path.join(package_dir, 'locale'), 'sphinx')

    if not color_terminal():
        nocolor()

    # parse options
    parser = get_parser()
    try:
        args = parser.parse_args(argv)
    except SystemExit as err:
        return err.code

    d = vars(args)
    # delete None or False value
    d = {k: v for k, v in d.items() if v is not None}

    # handle use of CSV-style extension values
    d.setdefault('extensions', [])
    for ext in d['extensions'][:]:
        if ',' in ext:
            d['extensions'].remove(ext)
            d['extensions'].extend(ext.split(','))

    try:
        if 'quiet' in d:
            if not {'project', 'author'}.issubset(d):
                print(('"quiet" is specified, but any of "project" or '
                         '"author" is not specified.'))
                return 1

        if {'quiet', 'project', 'author'}.issubset(d):
            # quiet mode with all required params satisfied, use default
            d.setdefault('version', '')
            d.setdefault('release', d['version'])
            d2 = DEFAULTS.copy()
            d2.update(d)
            d = d2

            if not valid_dir(d):
                print()
                print('Error: specified path is not a directory, or sphinx'
                      ' files already exist.')
                print('sphinx-quickstart only generate into a empty directory.'
                      ' Please specify a new root path.')
                return 1
        else:
            ask_user(d)
    except (KeyboardInterrupt, EOFError):
        print()
        print('[Interrupted.]')
        return 130  # 128 + SIGINT

    for variable in d.get('variables', []):
        try:
            name, value = variable.split('=')
            d[name] = value
        except ValueError:
            print(('Invalid template variable: %s') % variable)

    generate(d, overwrite=False, templatedir=args.templatedir)
    return d


if __name__ == '__main__':

    key = sys.argv[1]
    filename = sys.argv[2]
    reader = sys.argv[3]

    # Call sphinx quick-start to get us going.
    d = vnv_main(sys.argv[4:]) if len(sys.argv) > 4 else vnv_main([])
    srcPath = os.path.join(d['path'], 'source') if d['sep'] else d['path']

    confFile = os.path.join(srcPath, "conf.py")
    indexFile = os.path.join(srcPath, "index.rst")
    with open(indexFile, 'r') as file:
        filedata = file.read()

    tocTree = writeSingleFile(filename, key, reader)

    # Replace the target string
    replaceText = ".. vnv-read:: {reader} {filename}\n    :title: VnV Simulation report\n\n{toctree}".format(
        reader=reader, filename=filename, toctree=RestUtils.tocTree(tocTree))

    filedata = filedata.replace('.. toctree::', replaceText)

    # Write the file out again
    with open(indexFile, 'w') as file:
        file.write(filedata)

    theme = '''
    try:
       import sphinx_rtd_theme
       html_theme = 'sphinx_rtd_theme'
    except:
       pass
    '''
    d = Path(os.path.dirname(os.path.abspath(__file__)))
    with open(confFile, 'r') as file:
        fileData = file.read()

    with open(confFile, 'w') as file:
        file.write("import os\n")
        file.write("import sys\n\n")
        file.write(fileData)
        file.write('\n\nsys.path.insert(0,"{}")\n\n'.format(d))
        file.write('\n\nextensions.append("vnv")\n\n')
        file.write(textwrap.dedent(theme))

## This one should generate the restructed text files for the report. It


