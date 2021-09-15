from app.models import VnV


class VnVPlugin:
    COUNTER = 0
    PLUGINS = {}

    def __init__(self, name, filename):
        self.id_ = VnVPlugin.get_id()
        self.name = name
        self.filename = filename
        VnV.LoadPlugin(self.name, self.filename)

    @staticmethod
    def get_id():
        VnVPlugin.COUNTER += 1
        return VnVPlugin.COUNTER

    @staticmethod
    def add(name, filename):
        f = VnVPlugin(name, filename)
        VnVPlugin.PLUGINS[f.id_] = f
        return f

    @staticmethod
    def removeById(fileId):
        VnVPlugin.PLUGINS.pop(fileId)

    @staticmethod
    def find(id_):
        if id_ in VnVPlugin.PLUGINS:
            return VnVPlugin.PLUGINS[id_]
        raise FileNotFoundError


def getPlugins():
    ret = {}
    for p in VnVPlugin.PLUGINS.values():
        ret[p.name] = p.filename
    return ret
