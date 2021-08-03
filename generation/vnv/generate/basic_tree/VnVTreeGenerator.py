from io import StringIO

from ...vnv import VnVReader
from .. import RestUtils
import textwrap
import os
from ...directives.nodes.VnVNodes import getResultsTableForData

def _writeHeader(title, val, stream):
    stream.write(RestUtils.getHeader(title))
    stream.write(textwrap.dedent(val))  # Write the template for this node.
    stream.write("\n\n")


def writeValue(inode, stream):
    node = VnVReader.castDataBase(inode)
    val = node.getValue() if hasattr(node, "getValue") else ""
    stream.write("\n\n")
    stream.write(textwrap.dedent(val))
    stream.write("\n\n")


collapseId = 0


def getCollapseId():
    global collapseId
    collapseId += 1
    return str(collapseId)


collapse_start = '''
.. raw:: html

    <div class="panel-group" {comm}>
      <div class="panel panel-default">
        <div class="panel-heading">
          <h4 class="panel-title">
            {icon}<a data-toggle="collapse" href="#{id}">{title}</a>
          </h4>
        </div>
        <div id="{id}" class="panel-collapse collapse{show}">
           <div class="panel-body">

'''
_collapse_end = '''
.. raw:: html

    </div></div></div></div>
    <script>
       $('#{id}').on('shown.bs.collapse', function() {{
         var thisNode = document.getElementById('{id}')
         var resizers = thisNode.querySelectorAll('.resizer')
         Array.prototype.forEach.call(resizers, function(el) {{
            el.iFrameResizer.resize();
         }});
       }});
    </script>


'''

commMapText = '''
.. vnv-comm-map::
  :height: {height}
  :width: {width}
  
  
'''

def getCommMap(width=600, height=400):
    return commMapText.format(height=height,width=width)

def getIcon(passed, right_pad=8):
    color = "green" if passed else "red"
    id_ = 10004 if passed else 10007
    return '''<span style="color:{color}; padding-right:{pad}px;";>&#{id}</span>'''.format(
        id=id_, color=color, pad=right_pad)


def collapse_end(idr):
    return _collapse_end.format(id=idr)


def collapseStart(title, comm=None, show=False, icon=""):
    s = ".show" if show else ""
    id_ = getCollapseId()
    comm_ = "" if comm is None else 'vnvcomm="{}"'.format(comm)
    title = title if comm is None else "{} (comm {})".format(title,comm)
    return collapse_start.format(title=title, id=id_, show=s, icon=icon, comm=comm_), id_


def writeNode(node, stream):
    stream.write("\n\n.. vnv-node:: " + str(node.getId()) + "\n\n")


class VnVTreeGenerator:
    title = "VnV Testing Report"
    commMapOn = False
    commMap = {}

    def setTitle(self, title):
        self.title = title

    def setCommMapOn(self, on):
        self.commMapOn = on

    def getRestructuredText(self, node):
        return self.dispatcher(node)

    def dispatcher(self, node):
        typeStr = VnVReader.type2Str[node.getType()]
        m = getattr(self, "visit" + typeStr, None)
        if m is None:
            return self.visitGenericNode(node)
        else:
            return m(VnVReader.castDataBase(node))

    def writeChildren(self, title, node, stream, icon=""):
        if title is not None:
            stream.write(RestUtils.getHeader(title))
        for child in node:
            print(node.__class__,  )
            try:
                t, i = collapseStart(
                    child.getPackage() + ": " + child.getName().capitalize(), child.getComm(), icon=icon)
            except AttributeError:
                t, i = collapseStart(
                    child.getPackage() + ": " + child.getName().capitalize(), icon=icon)

            # Method does not exist; What now
            stream.write(t)
            writeNode(child, stream)
            stream.write(collapse_end(i))
        stream.write("\n\n")

    def visitGenericNode(self, node):
        return ""

    def visitLog(self, node: VnVReader.ILogNode):
        stream = StringIO()
        stream.write("\n.. vnv-log::\n")
        stream.write("    :package: " + node.getPackage() + "\n")
        stream.write("    :level: " + node.getLevel() + "\n")
        stream.write("    :stage: " + node.getStage() + "\n")
        stream.write("\n")

        mess = node.getMessage().splitlines()
        for i in mess:
            stream.write("   " + i + "\n")
        return stream.getvalue()

    def visitTest(self, node: VnVReader.ITestNode):
        stream = StringIO()
        writeValue(node, stream)
        if len(node.getChildren()) > 0:
            t, i = collapseStart("Testing Logs")
            stream.write(t)
            self.writeChildren(None, node.getChildren(), stream)
            stream.write(collapse_end(i))

        return stream.getvalue()

    def visitInjectionPoint(self, node: VnVReader.IInjectionPointNode):
        stream = StringIO()
        writeValue(node, stream)

        if len(node.getTests()) > 0:
            t, i = collapseStart("Injection Point Tests")
            stream.write(t)
            self.writeChildren(None, node.getTests(), stream)
            stream.write(collapse_end(i))

        if len(node.getChildren()) > 0:
            t, i = collapseStart("Child Injection Points")
            stream.write(t)
            self.writeChildren(None, node.getChildren(), stream)
            stream.write(collapse_end(i))
        return stream.getvalue()

    def visitUnitTest(self, node: VnVReader.IUnitTestNode):
        stream = StringIO()
        writeValue(node, stream)
        # Write the logs that occurred during the tests.

        for testName in node.getResults():
            test = node.getResults()[testName]
            icon = getIcon(test.getValue())
            m = node.getTestTemplate(test.getName())
            xt, xi = collapseStart(test.getName(), "unitTest", icon=icon)
            stream.write(xt)
            stream.write(textwrap.dedent(m))
            stream.write(collapse_end(xi))

        if len(node.getChildren()) > 0:
            t, i = collapseStart("Logs And Warnings")
            stream.write(t)
            self.writeChildren(None, node.getChildren(), stream)
            stream.write(collapse_end(i))
        return stream.getvalue()

    def visitInfo(self, node):
        stream = StringIO()
        writeValue(node, stream)
        return stream.getvalue()

    def visitRootNode(self, node: VnVReader.IRootNode):
        stream = StringIO()
        t, i = collapseStart(self.title, show=True)
        stream.write(t)

        if (self.commMapOn):
            stream.write(getCommMap())
            stream.write("\n\n")

        # Write the introduction.
        stream.write(textwrap.dedent(node.getVnVSpec().intro()))

        # Add the unit testing report for the node.
        if len(node.getUnitTests()) > 0:

            allPassed = True
            jsonData = []

            substream = StringIO()
            for child in node.getUnitTests():


                passed = True
                res = child.getResults()
                print(res)
                jdata = []
                for i in res:

                    jdata.append({"name":res[i].getName(), "value": res[i].getValue()})
                    if not res[i].getValue():
                        passed = False
                        allPassed = False

                jsonData.append({"name" : child.getName(), "value" : passed , "_children" : jdata})
                icon = getIcon(passed)
                tx, ix = collapseStart(
                    child.getPackage() + ": " + child.getName().capitalize(), icon=icon)
                substream.write(tx)
                writeNode(child, substream)
                substream.write(collapse_end(ix))
                substream.write("\n\n")

            tt, ii = collapseStart(
                "Unit Testing Results", icon=getIcon(allPassed))
            stream.write(tt)
            print(getResultsTableForData(jsonData,True))
            stream.write(getResultsTableForData(jsonData,True))
            stream.write(substream.getvalue())
            stream.write(collapse_end(ii))

        # Add the children nodes
        if len(node.getChildren()) > 0:
            ttt, iii = collapseStart("Injection Points")
            stream.write(ttt)
            self.writeChildren(None, node.getChildren(), stream)
            stream.write(collapse_end(iii))

        # Write the Conclusion. First, we need to set ourselves as the current node again
        # so any JMES searches in the conclusion text get processed
        # accordingly.
        stream.write(
            "\n\n.. vnv-set-id:: {id}\n\n".format(id=str(node.getId())))
        stream.write(textwrap.dedent(node.getVnVSpec().conclusion()))
        stream.write(collapse_end(i))
        return stream.getvalue()


def on_environment_ready(app):
    # Make sure the javascript is available.
    src_dir = os.path.join(os.path.abspath(os.path.dirname(__file__)), "data")
    app.config.html_static_path.append(src_dir)
    app.add_js_file("bootstrap.min.js")
    app.add_css_file("bootstrap.min.css")
    app.add_css_file("gen-style.css")


def setup(app):
    app.connect("builder-inited", on_environment_ready)
