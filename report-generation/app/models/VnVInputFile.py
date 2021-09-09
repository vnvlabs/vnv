import json
import os
import subprocess

from flask import render_template

from app.models import VnV
import rendering as r



class VnVInputFile:
    COUNTER = 0

    FILES = {}

    def __init__(self, name, filename, defVal, icon="icon-box"):
        self.name = name;
        self.filename = filename
        self.icon = icon
        self.id_ = VnVInputFile.get_id()
        self.notifications = []


        self.spec = VnV.LoadSpec(self.filename)
        if defVal is not None and os.path.exists(defVal):
            with open(defVal,'r') as f:
                self.value = json.load(f)
        else:
            self.value = VnV.getVnVConfigFile()



    def val(self):
        return json.dumps(self.value)

    def schema(self):
        return json.dumps(self.spec)

    @staticmethod
    def get_id():
        VnVInputFile.COUNTER += 1
        return VnVInputFile.COUNTER

    @staticmethod
    def add(name, filename, defVal):
        f = VnVInputFile(name, filename, defVal)
        VnVInputFile.FILES[f.id_] = f
        return f

    @staticmethod
    def removeById(fileId):
        VnVInputFile.FILES.pop(fileId)

    @staticmethod
    def find(id_):
        if id_ in VnVInputFile.FILES:
            return VnVInputFile.FILES[id_]
        raise FileNotFoundError
