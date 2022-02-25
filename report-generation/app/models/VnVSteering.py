import glob
import json
import os
import time

import jsonschema
from flask import parse__template, make_response

from app.models import VnV


class SteerRender:

    def __init__(self, fileId, id_, message, expiry):
        self.id_ = id_
        self.fileId = fileId
        self.schema = message
        self.expiry = expiry


class VnVSteering:
    COUNTER = 0

    FILES = {}

    def __init__(self, name, filename, icon="icon-box"):
        self.name = name
        self.filename = filename
        self.icon = icon
        self.id_ = VnVSteering.get_id()
        self.notifications = []

    def list_pending(self):
        pending = glob.glob(
            os.path.join(
                self.filename,
                "__response__",
                "*.pending"))
        pend = []
        for i in pending:
            f = open(i, 'r')
            message = json.load(f)
            expiry = message["expires"]
            if int(time.time()) > expiry:
                f.close()
                os.remove(i)
            else:
                pend.append(SteerRender(self.id_, i[len(os.path.join(
                    self.filename, "__response__")) + 1:-8], message["schema"], expiry))
            f.close()

        return pend

    def render(self, id_):
        return self.get(
            os.path.join(
                self.filename,
                "__response__",
                id_ + ".pending"))

    def get(self, filename):
        with open(filename, 'r') as f:
            message = json.load(f)
            expiry = message["expires"]
            return SteerRender(self.id_, filename[len(os.path.join(
                self.filename, "__response__")) + 1:-8], message["schema"], expiry)

    def getFirstPending(self):
        l = self.list_pending()
        if len(l) > 0:
            return render_template('steering/render.html', steer=l[0])
        return "<div>No Requests Available at this time </div>"

    def respond(self, id_, message):
        r = os.path.join(self.filename, "__response__", id_)
        try:
            mess = json.loads(message)
            steer = self.render(id_)
            jsonschema.validate(mess, steer.schema)

        except Exception as e:
            return make_response("error", 501)
        with open(r + ".responding", 'w') as f:
            f.write(message)
        os.rename(r + ".responding", r + ".complete")
        os.remove(r + ".pending")

    @staticmethod
    def get_id():
        VnVSteering.COUNTER += 1
        return VnVSteering.COUNTER

    @staticmethod
    def add(name, filename):
        f = VnVSteering(name, filename)
        VnVSteering.FILES[f.id_] = f
        return f

    @staticmethod
    def removeById(fileId):
        VnVSteering.FILES.pop(fileId)

    @staticmethod
    def find(id_):
        if id_ in VnVSteering.FILES:
            return VnVSteering.FILES[id_]
        raise FileNotFoundError
