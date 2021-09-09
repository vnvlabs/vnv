import json
import os
from datetime import datetime
from urllib.request import pathname2url
import pygments.formatters.html
from pygments.lexers import guess_lexer, guess_lexer_for_filename
from _VnVReader import node_type_START, node_type_POINT, node_type_DONE, node_type_ITER, node_type_ROOT, node_type_LOG, \
    node_type_END
from flask import render_template

from app.models import VnV
import rendering as r
from rendering.readers import has_reader
from rendering.vnvdatavis.directives.jmes import render_vnv_template


class ProvFileWrapper:

    def __init__(self, pfile, description):
        self.file = pfile
        self.description = description

    def getName(self):
        return self.file.info.name

    def getUrl(self):
        return pathname2url(self.getName())

    def getSize(self):
        return self.file.info.size

    def getTimeStamp(self):
        return self.file.info.timestamp

    def modified(self):
        return self.file.modified()

    def reader(self):
        return self.file.reader

    def text(self):
        return self.file.text

    def package(self):
        return self.file.package

    def comm(self):
        return self.file.comm

    def readable(self):
        return has_reader(self.file.reader)

    def getDescription(self):
        return self.description


class ProvWrapper:

    def __init__(self, vnvprov, templates):
        self.prov = vnvprov
        self.templates = templates

    def getD(self, provfile):
        return render_template(self.templates.get_file_description(provfile.package, provfile.name))

    def get_executable(self):
        return ProvFileWrapper(self.prov.executable, "")

    def get_command_line(self):
        return self.prov.commandLine

    def get_libraries(self):
        return [ProvFileWrapper(a, "") for a in self.prov.libraries]

    def get_outputs(self):
        return [ProvFileWrapper(a, self.getD(a)) for a in self.prov.outputFiles]

    def get_inputs(self):
        return [ProvFileWrapper(a, self.getD(a)) for a in self.prov.inputFiles]

    def get_input(self):
        return ProvFileWrapper(self.prov.inputFile, "")

    def get_timestamp(self):
        t = self.prov.time_in_seconds_since_epoch
        return datetime.fromtimestamp(t).strftime('%Y-%m-%d %H:%M:%S')

    def render_input_file(self):
        lex = pygments.lexers.get_lexer_by_name("json")
        form = pygments.formatters.html.HtmlFormatter(linenos=True, style="colorful", noclasses=True)
        return pygments.highlight(self.get_input().text(), lex, form);


class CommObj:

    def __init__(self, comm_map, node_map, world_size):
        self.comm_map = comm_map
        self.node_map = node_map
        self.world_size = world_size

    def keys(self):
        return self.comm_map.keys();


class CommRender:
    def __init__(self, comm, commObj):
        self.comm = comm
        self.commObj = commObj
        self.html = "TODO"

    def getData(self):
        res = {}
        m = self.getMatSize()
        for node, procs in self.commObj.node_map.items():
            for proc in procs:
                x, y = self.getIndex(proc, m)
                if y in res:
                    res[y]["data"].append(
                        {"x": x, "y": int(node) * (1 if self.inComm(proc) else -1), "r": proc, "o": self.inComm(proc)})
                else:
                    res[y] = {
                        "name": f"Row {y}",
                        "row": y,
                        "data": [{"x": x, "y": int(node) * (1 if self.inComm(proc) else -1), "r": proc,
                                  "o": self.inComm(proc)}]
                    }
        print(res)
        return json.dumps(sorted(res.values(), key=lambda x: x["row"]))

    def inComm(self, proc):
        return proc in self.commObj.comm_map[self.comm]

    def getIndex(self, rank, matSize):
        return int(rank / matSize), rank % matSize

    def getMatSize(self):
        i = 1;
        while self.commObj.world_size > i * i:
            i += 1
        return i


class LogRender:
    def __init__(self, data, commObj, templates):
        self.data = data
        self.commObj = commObj
        self.templates = templates

    def get_comm(self):
        return CommRender(self.data.getComm(), self.commObj)

    def getMessage(self):
        return self.data.getMessage()

    def getTime(self):
        return self.data.getTime()

    def getStage(self):
        return self.data.getStage()


class UnitTestRender:

    def __init__(self, unitTest, commObj, templates):
        self.commObj = commObj
        self.data = unitTest
        self.templates = templates

    def getHtml(self):
        t = self.templates.get_html_file_name("UnitTests", self.data.getPackage(), self.data.getName())
        return render_vnv_template(t, data=self.data.getData())


class TestRender:

    def __init__(self, data, commObj, templates, template_override=None):
        self.data = data
        self.commObj = commObj
        self.templates = templates
        self.template_override = template_override

    def getHtml(self):
        if self.template_override is None:
            t = self.templates.get_html_file_name("Tests", self.data.getPackage(), self.data.getName())
            return render_vnv_template(t, data=self.data.getData())

        return render_vnv_template(self.template_override, data=self.data.getData())

    def getName(self):
        return self.data.getName()

    def getId(self):
        return self.data.getId()

    def getPackage(self):
        return self.data.getPackage()

    def getLogs(self):
        return [LogRender(a, self.commObj, self.templates) for a in self.data.getLogs()]


class PackageRender:
    def __init__(self, package, data, templates):
        self.data = data
        self.package = package
        self.templates = templates

    def getHtml(self):
        t = self.templates.get_package(self.package)
        a = render_vnv_template(t, data=self.data.getData())
        if len(a) > 0:
            return a
        else:
            return "<h4> No Package Information Available </h>"

    def getLogs(self):
        return [LogRender(a, self.commObj, self.templates) for a in self.data.getLogs()]


class SourceFile:
    def __init__(self, file, line=None):
        self.file = file
        self.line = line

    def getFilename(self):
        return self.file

    def getLine(self):
        return self.line

    def hasLine(self):
        return self.line is not None

    def getLineList(self):
        return [self.getLine()] if self.hasLine() else []

    def render(self):
        try:
            with open(self.file, 'r') as f:
                d = f.read()
                lex = guess_lexer_for_filename(self.file, d, stripnl=False)
                form = pygments.formatters.html.HtmlFormatter(linenos=True, hl_lines=self.getLineList(),
                                                              style="colorful", cssclass="vnvhigh")
                result = pygments.highlight(d, lex, form);
        except Exception as e:
            result = "Could not load file with name " + self.file

        return result


class InjectionPointRender:

    def __init__(self, injection_point, templates, commObj):
        self.ip = injection_point.cast()
        self.templates = templates
        self.commObj = commObj

    def getCommRender(self):
        return CommRender(self.ip.getComm(), self.commObj)

    def getName(self):
        return self.ip.getName()

    def getPackage(self):
        return self.ip.getPackage()

    def getInternalTest(self):
        tempoverride = self.templates.get_html_file_name("InjectionPoints", self.ip.getPackage(), self.ip.getName())
        return TestRender(self.ip.getData(), self.commObj, self.templates,
                          template_override=tempoverride)

    def getAdditionalTests(self):
        return [TestRender(a.cast(), self.commObj, self.templates) for a in self.ip.getTests()]

    def getLogs(self):
        return [LogRender(a.cast(), self.commObj, self.templates) for a in self.ip.getLogs()]

    def getSourceMap(self):
        source = json.loads(self.ip.getSourceMap())  # map stageId -> [ filename, line]
        return {a: SourceFile(b[0], b[1]) for a, b in source.items()}


class VnVFile:
    COUNTER = 0

    FILES = {}

    def __init__(self, name, filename, reader, template_dir, icon="icon-box"):
        self.name = name;
        self.filename = filename
        self.reader = reader
        self.icon = icon
        self.id_ = VnVFile.get_id()
        self.notifications = []

        self.wrapper = VnV.Read(filename, reader)
        self.root = self.wrapper.get()
        vnvspec = json.loads(self.root.getVnVSpec().get())
        self.template_dir = template_dir
        self.templates = r.build(template_dir, vnvspec)

    def __del__(self):
        os.removedirs(self.template_dir)

    def getCommRender(self, id):
        return CommRender(id, self.getCommObj())

    def get_world_size(self):
        if not hasattr(self, 'world_size'):
            self.world_size = self.root.getCommInfoNode().getWorldSize()
        return self.world_size

    def getPackages(self):
        return [{"name": a} for a in self.root.getPackages().fetchkeys()]

    def getFirstPackage(self):
        a = self.getPackages()
        if len(a) > 0:
            return self.render_package(a[0]["name"])
        return ""

    def getGlobalCommMap(self):
        return self.root.getCommInfoNode().getCommMap().toJsonStr(False)

    def renderDefaultComm(self):

        j = json.loads(self.getGlobalCommMap())
        if "nodes" in j and len(j["nodes"]) > 0 and "id" in j["nodes"][0]:
            i = j["nodes"][0]["id"]
            try:
                a = render_template("files/comm.html", commrender=self.getCommRender(str(i)))
                return a
            except Exception as e:
                print(e)
        return ""

    class RankInfo:

        def __init__(self, commInfoNode):
            self.size = commInfoNode.getWorldSize()

            nmap = json.loads(commInfoNode.getNodeMap())
            self.nodes = len(nmap)
            self.min_rpn = len(nmap[min(nmap.keys(), key=lambda x: len(nmap[x]))])
            self.max_rpn = len(nmap[max(nmap.keys(), key=lambda x: len(nmap[x]))])
            self.version = commInfoNode.getVersion()
            self.unique_comms = len(json.loads(commInfoNode.getCommMap().getComms()))

        def shortversion(self, length):
            return self.version[0: min(len(self.version), length)] + "..."

    def getRankInfo(self):

        return VnVFile.RankInfo(self.root.getCommInfoNode())

    def getDefaultComm(self):
        j = json.loads(self.getGlobalCommMap())
        if "nodes" in j and len(j["nodes"]) > 0 and "id" in j["nodes"][0]:
            return j["nodes"][0]["id"]

    def list_unit_test_packages(self):
        return [{"name": a.getPackage + ":" + a.getName()} for a in self.root.getUnitTests()]

    def hasUnitTests(self):
        return len(self.root.getUnitTests())

    def render_unit_test(self, id=None):
        if (id is None and self.hasUnitTests()):
            return render_template("viewers/unittest.html",
                                   unitrender=UnitTestRender(self.root.getUnitTests()[0], self.getCommObj(),
                                                             self.templates))
        for i in self.root.getUnitTests():
            if i.getId() == id:
                return render_template("viewers/unittest.html",
                                       unitrender=UnitTestRender(i, self.getCommObj(),
                                                                 self.templates))

        return ""

    def get_comm_map(self):
        x = self.root.getCommInfoNode().getCommMap();
        s = x.getComms()
        return json.loads(s)

    def get_prov(self):
        return ProvWrapper(self.root.getInfoNode().getProv(), self.templates)

    def get_node_map(self):
        x = self.root.getCommInfoNode().getNodeMap()
        return json.loads(x)

    def getCommObj(self):
        if not hasattr(self, "commObj"):
            self.commObj = CommObj(self.get_comm_map(), self.get_node_map(), self.get_world_size())
        return self.commObj

    def list_communicators(self):
        return self.getCommObj().keys()

    def get_injection_point(self, id):
        return self.root.findById(id)

    def list_injection_points(self, proc):
        """Return a nested heirarchy of injection points and logs for this comm."""
        return self.root.single_proc_heirarchy(proc)

    def get_introduction(self):
        return render_template(self.templates.get_introduction())

    def get_conclusion(self):
        return render_template(self.templates.get_conclusion())

    def render_ip(self, id):
        if id == VnVFile.INJECTION_INTRO:
            return self.get_introduction()
        elif id == VnVFile.INJECTION_CONC:
            return self.get_conclusion()
        ip = self.get_injection_point(id)
        return InjectionPointRender(ip, self.templates, self.getCommObj())

    def render_package(self, package):
        packageTestObject = self.root.getPackage(package)
        return PackageRender(package, packageTestObject, self.templates).getHtml()

    def reset_proc_iter(self):
        self.proc_iter = None

    def set_proc_iter(self, proc=0, only=None):
        self.proc_iter_config = {"id": int(proc), "only": True if only else False, "comm": False}
        self.proc_iter = self.root.getWalker("VNV", "proc", json.dumps(self.proc_iter_config))
        self.currX = -1
        self.currY = -1
        return True

    def set_comm_iter(self, comm, only=None):
        self.proc_iter_config = {"id": int(comm), "only": True if only else False, "comm": True}
        self.proc_iter = self.root.getWalker("VNV", "proc", json.dumps(self.proc_iter_config))
        self.currX = -1
        self.currY = -1
        return True

    node_type_map = {
        node_type_POINT: [1, -1],
        node_type_START: [1, 0],
        node_type_DONE: [0, 0],
        node_type_ITER: [1, -1],
        node_type_ROOT: [1, 0],
        node_type_LOG: [0, 0],
        node_type_END: [0, -1]
    }
    INJECTION_INTRO = -100
    INJECTION_CONC = -101

    def proc_iter_next(self, count=10):
        if (self.currX > 50):
            return []
        res = []
        if self.currX == -1:
            res.append({"x": 0, "y": 0, "id": VnVFile.INJECTION_INTRO, "time": 0})
            self.currX = 0
            self.currY = 0

        i = 0
        while i < count:

            n = self.proc_iter.next()
            if n is not None:

                if n.type == node_type_ITER:
                    continue

                self.currX += 1
                self.currY += VnVFile.node_type_map[n.type][0]

                if n.type == node_type_DONE:
                    res.append(
                        {"x": self.currX, "y": self.currY, "id": VnVFile.INJECTION_CONC, "done": True, "time": n.time})
                    break
                else:
                    res.append({"x": self.currX, "y": self.currY, "id": n.item.getId(), "time": n.time})

                self.currY += VnVFile.node_type_map[n.type][1]
                i += 1
            else:
                break

        return res

    def lock(self):
        self.root.lock();
    def release(self):
        self.root.release()

    @staticmethod
    def get_id():
        VnVFile.COUNTER += 1
        return VnVFile.COUNTER

    @staticmethod
    def add(name, filename, reader, template_dir):
        f = VnVFile(name, filename, reader, template_dir)
        VnVFile.FILES[f.id_] = f
        return f

    @staticmethod
    def removeById(fileId):
        VnVFile.FILES.pop(fileId)

    class FileLockWrapper:
        def __init__(self, file):
            self.file = file

        def __enter__(self):
            self.file.lock()
            return self.file

        def __exit__(self, type, value, traceback):
            self.file.release()

    @staticmethod
    def find(id_):
        if id_ in VnVFile.FILES:
            return VnVFile.FileLockWrapper(VnVFile.FILES[id_])
        raise FileNotFoundError
