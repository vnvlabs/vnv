import json
import os
import uuid

import jsonschema
from jsonschema.exceptions import ErrorTree, ValidationError, SchemaError

from app.base.utils import mongo
from app.models import VnV
from app.models.VnVConnection import VnVLocalConnection, VnVConnection, connectionFromJson



def get_current_path(newVal, row, col):
    i = 0
    while i < len(newVal):
        c = newVal[i]
        if c in map:
           looking = map[c]




def get_row_and_column(path, newVal, a):
    try:
        if len(path) == 0:
            return 1, 1
        p = a
        while len(path) > 1:
            p = p[path.popleft()]
        p[path.pop()] = "91123212"
        aa = json.dumps(a, separators=(',', ':')).find("91123212")

        s = newVal.split("\n")
        newlines = 0
        currcol = 0
        inquotes = False
        for i in newVal:
            sub = True
            currcol += 1
            if i == "\n":  # new line so reset col count and currcol
                sub = False
                newlines += 1
                currcol = 0
            elif i == " " and not inquotes:
                sub = False
            elif i == "\"" and (i == 0 or newVal[i - 1] != "\\"):
                inquotes = not inquotes
            if sub: aa -= 1
            if aa == 0: return newlines, currcol;
    except:
        pass

    return 1, 1


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
                return ["green", "Valid"]
            else:
                return ["#d54287", "Application does not exist"]
        else:
            return ["red", "Connection is not open"]

    def getSpecDumpCommand(self):
        return self.specDump.replace("${application}", self.filename)

    def loadSpec(self):
        self.spec = self.connection.execute(self.getSpecDumpCommand())

    def schema(self):
        return self.spec

    def describe(self):
        return f'{self.connection.describe()}://{self.filename}'

    def execTemplate(self):
        return self.exec

    def validateSpec(self, newVal):
        try:
            s = json.loads(newVal)
            jsonschema.Draft7Validator.check_schema(s)
            return []
        except SchemaError as v:
            r, c = get_row_and_column(v.path, newVal, s)
            return [{"row": r, "column": c, "text": v.message, "type": 'warning', "source": 'vnv'}]
        except Exception as e:
            return [{"row": 1, "column": 1, "text": str(e), "type": 'warning', "source": 'vnv'}]

    def validateInput(self, newVal):
        try:
            s = json.loads(self.spec)
            a = json.loads(newVal)
            errs = jsonschema.validate(a, schema=s)
            return []
        except ValidationError as v:
            r, c = get_row_and_column(v.path, newVal, a)
            return [{"row": r, "column": c, "text": v.message, "type": 'warning', "source": 'vnv'}]
        except Exception as e:
            return [{"row": 1, "column": 1, "text": str(e), "type": 'warning', "source": 'vnv'}]

    # TODO Implement this.
    EXECUTION_SCHEMA = {
        "type": "object",
        "properties": {
            "run": {"type": "boolean"}
        },
        "additionalProperties": False
    }

    def validateExecution(self, newVal):
        try:
            a = json.loads(newVal)
            errs = jsonschema.validate(a, schema=VnVInputFile.EXECUTION_SCHEMA)
            return []
        except ValidationError as v:
            r, c = get_row_and_column(v.path, newVal, a)
            return [{"row": r, "column": c, "text": v.message, "type": 'warning', "source": 'vnv'}]
        except Exception as e:
            return [{"row": 1, "column": 1, "text": str(e), "type": 'warning', "source": 'vnv'}]

    def autocomplete_input(self, row, col, pre, val):
        return []

    def autocomplete_exec(self, row, col, pre, val):
        return []

    def autocomplete_spec(self, row, col, pre, val):
        return []

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
