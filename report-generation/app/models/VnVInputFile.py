import json
import os
import uuid

from app.base.utils import mongo
from app.models import VnV
from app.models.VnVConnection import VnVLocalConnection, VnVConnection, connectionFromJson


class VnVInputFile:
    COUNTER = 0

    FILES = {}

    def __init__(self, name):
        self.name = name
        self.filename = "path/to/application"
        self.icon = "icon-box"
        self.id_ = VnVInputFile.get_id()
        self.notifications = []
        self.connection = VnVLocalConnection()

        self.loadfile = ""
        self.value = json.dumps(VnV.getVnVConfigFile(), indent=4)
        self.spec = "{}"
        self.specDump = "${application} --vnv-dump 1"
        self.exec = "{}"
        self.execFile = ""

    def toJson(self):
        a = {}
        a["name"] = self.name
        a["filename"] = self.filename
        a["icon"] = self.icon
        a["connection"] = self.connection.toJson()
        a["loadfile"] = self.loadfile
        a["value"] = self.value
        a["spec"] = self.spec
        a["specDump"] = self.specDump
        a["exec"] = self.exec
        a["execFile"] = self.execFile
        return a

    @staticmethod
    def fromJson(a):
        r = VnVInputFile(a["name"])
        r.filename = a["filename"]
        r.icon = a["icon"]
        r.connection = connectionFromJson(a["connection"])
        r.loadfile = a["loadfile"]
        r.value = a["value"]
        r.spec = a["spec"]
        r.specDump = a["specDump"]
        r.exec = a["exec"]
        r.execFile = a["execFile"]
        return r

    def setConnection(self, hostname, username, password, port):
        self.connection = VnVConnection(hostname, username, password, port)

    def setConnection(self):
        self.connection = VnVLocalConnection()

    def getFileStatus(self):
        if self.connection.connected():
            if self.connection.exists(self.filename):
                return ["success","Valid"]
            else:
                return ["warning", "Application does not exist"]
        else:
            return ["error", "Connection is not open"]

    def getSpecDumpCommand(self):
        return self.specDump.replace("${application}",self.filename)

    def loadSpec(self):
        self.spec = self.connection.execute(self.getSpecDumpCommand())

    def schema(self):
        return self.spec

    def describe(self):
        return f'{self.connection.describe()}://{self.filename}'

    def execTemplate(self):
        return self.exec

    @staticmethod
    def get_id():
        VnVInputFile.COUNTER += 1
        return VnVInputFile.COUNTER

    @staticmethod
    def add(name):

        a = mongo.loadInputFile(name)
        if a is not None:
            f = VnVInputFile.fromJson(a)
        else:
            f = VnVInputFile(name)

        VnVInputFile.FILES[f.id_] = f
        return f

    @staticmethod
    def removeById(fileId):
        VnVInputFile.FILES.pop(fileId)

    @staticmethod
    def find(id_):
        if id_ in VnVInputFile.FILES:
           return VnVInputFile.FileLockWrapper(VnVInputFile.FILES[id_])
        raise FileNotFoundError

    class FileLockWrapper:
        def __init__(self, file):
            self.file = file

        def __enter__(self):
            return self.file

        def __exit__(self, type, value, traceback):
            mongo.persistInputFile(self.file)
