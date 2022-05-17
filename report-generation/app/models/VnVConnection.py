import glob
import json
import os
import shutil
import subprocess
import tempfile
import stat
import time
import uuid
from datetime import datetime
from pathlib import Path
import shlex
import paramiko
from ansi2html import Ansi2HTMLConverter
from app.models.RemoteFileInfo import get_file_name
from datetime import datetime


class VnVJob:
    def __init__(self, id, name, script, metadata, ctx):
        self.id = id
        self.name = name
        self.ctx = ctx
        self.script_ = script
        self.metadata = metadata
        self.time = datetime.now();

    def getName(self):
        return self.name if self.name is not None else self.id

    def getId(self):
        return self.id

    def getCtx(self):
        return self.ctx

    def running(self):
        a = self.getCtx().running()
        return a

    def script(self):
        return self.script_

    def metadata(self):
        return self.metadata

    def dispDate(self):
        return self.time.strftime("%m/%d/%Y at %H:%M:%S")

    def stdout(self):
        return Ansi2HTMLConverter().convert(self.getCtx().stdout())

    def exitcode(self):
        return self.getCtx().exitcode()


class VnVConnection:
    INFO_FILE = get_file_name()
    INFO_FILE_PATH = "__vnv_fetch__.py"

    def __init__(self, domain=None, port=None, username=None):
        self.transport = None
        self.username_ = username
        self.domain_ = domain
        self.port_ = port
        self.pythonpath = "python3"
        self.cache = {}
        self.running_sessions = {}

    def toJson(self):
        return {
            "username": self.username_,
            "domain": self.domain_,
            "port": self.port_,
        }

    def getPath(self, filename, exten=None):
        t = tempfile.gettempdir()
        uu = os.path.join(t, uuid.uuid4().hex)

        if exten is None:
            ext = filename.split(".")
            if len(ext) > 1:
                uu += "." + ext[-1]
        else:
            uu += "." + exten
        return uu

    def local(self):
        return False

    def connect(self, username, domain, port, password):
        connect = not self.connected()
        if self.username_ != username:
            self.username_ = username
            connect = True
        if self.domain_ != domain:
            self.domain_ = domain
            connect = True
        if self.port != port:
            self.port_ = port
            connect = True

        if connect:
            try:
                self.transport = paramiko.Transport((domain, port))
                self.transport.connect(None, username, password)
                self.sftp().put(VnVConnection.INFO_FILE, VnVConnection.INFO_FILE_PATH)
            except Exception as e:
                print(e)
                self.transport = None

        if self.connected():
            return True
        return False

    def disconnect(self):
        if self.transport is not None:
            self.transport.close()
            self.transport = None

    def port(self):
        return self.port_

    def username(self):
        return self.username_

    def domain(self):
        return self.domain_

    def connected(self):
        if self.transport is None:
            return False
        try:
            self.transport.send_ignore()
            return True
        except EOFError as e:
            return False

    def sftp(self):
        return paramiko.SFTPClient.from_transport(self.transport)

    class SessionContext:
        nbytes = 4096

        def __init__(self, session):
            self.session = session
            self.stdout_data = None

        def running(self):
            return not self.session.exit_status_ready()

        def stdout(self):
            if not self.running():
                if self.stdout_data is None:
                    self.stdout_data = []

                    while self.session.recv_ready():
                        self.stdoutdata.append(self.session.recv(self.nbytes).decode("utf-8"))

                return "".join(self.stdoutdata)
            return ""

        def exitcode(self):
            if not self.running():
                return self.session.recv_exit_status()
            return -1

        def cancel(self):
            self.session.close()

    GLOBY = """python -c 'import glob; import json; print(json.dumps(glob.glob("{path}")))'"""

    def autocomplete(self, pref):
        try:
            return json.loads(self.execute(self.GLOBY.format(path=pref)))  # Not implemented yet
        except:
            return []

    def execute(self, command, asy=False, name=None, fullscript=None, metadata=None):
        nbytes = 4096
        stdout_data = []
        stderr_data = []
        session = self.transport.open_channel(kind='session')
        session.exec_command(command)

        # Block until finished
        if not asy:
            while not session.exit_status_ready():
                pass

            while session.recv_ready():
                stdout_data.append(session.recv(nbytes).decode("utf-8"))

            session.recv_exit_status()
            return "".join(stdout_data)
        else:
            uid = uuid.uuid4().hex
            self.running_sessions[uid] = VnVJob(uid, name, command if fullscript is None else fullscript,
                                                metadata=metadata, ctx=VnVConnection.SessionContext(session))
            return uid

    def get_jobs(self):
        return [v for k, v in self.running_sessions.items()]

    def delete_job(self, jobId):
        self.running_sessions.pop(jobId)

    def cancel_job(self, jobId):
        self.running_sessions[jobId].getCtx().cancel()

    def execute_script(self, script, asy=True, name=None, metadata=None):

        path = self.write(script, None)
        self.execute("chmod u+x " + path)
        self.execute(path, asy, name=name, fullscript=Ansi2HTMLConverter().convert(script), meta=metadata)

    def getInfo(self, path):

        if path in self.cache:
            return self.cache[path]

        nbytes = 4096
        stdout_data = []
        stderr_data = []
        session = self.transport.open_channel(kind='session')
        session.exec_command(f"{self.pythonpath} {VnVConnection.INFO_FILE_PATH} {path}")

        # Block until finished
        while not session.exit_status_ready():
            pass

        while session.recv_ready():
            stdout_data.append(session.recv(nbytes).decode("utf-8"))

        session.recv_exit_status()

        try:
            a = json.loads(''.join(stdout_data))
            self.cache[path] = a
            return a
        except Exception as e:
            print(e)

    def describe(self):
        return f"{self.username_}@{self.domain_}:{self.port_}"

    def download(self, remote):
        info = self.getInfo(remote)
        if "download" in info:
            return info["download"]

        p = self.getPath(remote)
        self.sftp().get(remote, p)
        info["download"] = p
        return p

    def upload(self, remote, local):
        self.sftp().put(local, remote)

    def write(self, txt, path):
        if path is None:
            path = self.execute("mktemp")

        sf = self.sftp()
        f = sf.file(path, 'w', -1)
        f.write(txt)
        f.flush()
        sf.close()
        return path

    def destroy(self):
        if self.sftp: self.sftp.close()
        if self.transport: self.transport.close()

    def exists(self, path):
        return self.getInfo(path)["exists"]

    def is_dir(self, path):
        return self.getInfo(path)["isdir"]

    def info(self, path):
        a = self.getInfo(path)
        return a["abspath"], a["dir"], a["name"], a["ext"], a["size"], a["lastMod"], a["lastModStr"]

    def children(self, abspath):
        return self.getInfo(abspath)["children"]

    def root(self):
        return self.getInfo("~")["root"]

    def home(self):
        return self.getInfo("~")["home"]

    def crumb(self, path):
        return self.getInfo(path)["crumb"]


class VnVLocalConnection:

    def __init__(self):
        self.connected_ = True
        self.running_procs = {}

    def toJson(self):
        return {}

    def describe(self):
        return "localhost"

    def local(self):
        return True

    def port(self):
        return "22"

    def username(self):
        return ""

    def domain(self):
        return ""

    def connect(self, username, domain, port, password):
        self.connected_ = True
        return True

    def disconnect(self):
        self.connected_ = False

    def connected(self):
        return self.connected_

    def autocomplete(self, pref):
        return glob.glob(pref + "*")

    class SessionContext:

        def __init__(self, session):
            self.session = session
            self.stdout_data = None

        def running(self):
            return self.session.poll() is None

        def cancel(self):
            self.session.kill()

        def stdout(self):
            if not self.running():
                if self.stdout_data is None:
                    self.stdout_data = self.session.communicate()[0].decode("utf-8")
                return self.stdout_data
            return ""

        def exitcode(self):
            if not self.running():
                return self.session.returncode
            return -1

    def execute(self, command, asy=False, name=None, fullscript=None, metadata=None, env={}):
        try:
            result = subprocess.Popen(shlex.split(command), stdout=subprocess.PIPE, env=env)
            if not asy:
                return result.communicate()[0].decode("utf-8")
            else:
                uid = uuid.uuid4().hex
                self.running_procs[uid] = VnVJob(uid, name, command if fullscript is None else fullscript,
                                                 metadata, VnVLocalConnection.SessionContext(result))
                return uid
        except Exception as e:
            raise Exception("Failed to execute command: " + str(e))

    def execute_script(self, script , work_dir=None, deps={}, asy=True, name=None, metadata=None):
        path = self.write(script, None)
        st = os.stat(path)
        os.chmod(path, st.st_mode | stat.S_IEXEC)
        return self.execute("sh " + path, asy, name=name, fullscript=Ansi2HTMLConverter().convert(script), metadata=metadata)

    def get_jobs(self):
        return [v for k, v in self.running_procs.items()]

    def delete_job(self, jobId):
        self.running_procs.pop(jobId)

    def cancel_job(self, jobId):
        self.running_procs[jobId].getCtx().cancel()

    def exists(self, path):
        return os.path.exists(os.path.abspath(path))

    def is_dir(self, path):
        return os.path.isdir(os.path.abspath(path))

    def info(self, path):
        abspath = os.path.abspath(path)
        dir = os.path.dirname(abspath)
        name = os.path.basename(abspath)
        if os.path.exists(abspath):
            size = os.lstat(abspath).st_size if len(abspath) else 0
            lastMod = os.lstat(abspath).st_mtime if len(abspath) else 0
            lastModStr = (datetime.fromtimestamp(lastMod).strftime('%Y-%m-%d %H:%M:%S')) if len(abspath) else ""
        else:
            size = 0
            lastMod = 0
            lastModStr = ""

        ext = "directory" if os.path.exists(abspath) and Path(abspath).is_dir() else os.path.splitext(abspath)[1]
        return abspath, dir, name, ext, size, lastMod, lastModStr

    def write(self, txt, path):
        if path is None:
            path = self.execute("mktemp").rstrip().lstrip()

        with open(path, 'w') as f:
            f.write(txt)

        return path

    def home(self):
        return os.path.expanduser("~")

    def root(self):
        return os.path.abspath(os.sep)

    def children(self, abspath):
        return [os.path.join(abspath, i) for i in os.listdir(abspath)]

    def download(self, remote):
        return remote

    def upload(self, remote, local):
        shutil.copy(local, remote)

    def crumb(self, dir):
        c = os.path.normpath(dir).split(os.path.sep)
        cc = os.path.abspath(os.sep)
        loc = []
        for i in c:
            if len(i):
                cc = os.path.join(cc, i)
                loc.append(cc)
        return loc


def connectionFromJson(j):
    if j:
        return VnVConnection(domain=j["domain"], username=j["username"], port=j["port"])
    return VnVLocalConnection()


class MainConnection:
    MAIN_CONNECTION = VnVLocalConnection()


def SetMainConnection(local, username, domain, password, port):
    MainConnection.MAIN_CONNECTION.disconnect()

    if local:
        MainConnection.MAIN_CONNECTION = VnVLocalConnection()
    else:
        MainConnection.MAIN_CONNECTION = VnVConnection()

    return MainConnection.MAIN_CONNECTION.connect(username, domain, int(port), password)


def SetFileConnection(file, local, username, domain, password, port):
    file.connection.disconnect()

    if local:
        file.connection = VnVLocalConnection()
    else:
        file.connection = VnVConnection()

    return file.connection.connect(username, domain, int(port), password)


def MAIN_CONNECTION():
    return MainConnection.MAIN_CONNECTION
