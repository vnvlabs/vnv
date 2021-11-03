import json
import os
import shutil

import sphinx
from flask import render_template

from app.models import VnV
from app.models.VnVFile import VnVFile
from app.rendering import setup_build_directory, get_conf_template

'''
Pipeline should be a directory containing a file called 

config.json with the form:

{
        "files" : [
                ["VNV1","json_file","../vv-output"]
        ]
}

We will render a file called "index.rst" 
~       


'''


class PipelineBuild:
    def __init__(self, src_dir, id_):
        self.src_dir = src_dir
        self.file = id_

    def get_content(self):
        return os.path.join("pipeline_renders", self.src_dir, "_build", "html", "index.html")

def build_pipeline(src_dir, id_, pipeline_dir):
    if os.path.exists(src_dir):
        shutil.rmtree(src_dir)

    shutil.copytree(pipeline_dir, src_dir)
    os.makedirs(os.path.join(src_dir, "_build"))
    os.makedirs(os.path.join(src_dir, "_static"))
    os.makedirs(os.path.join(src_dir, "_templates"))

    # Write a configuration file in the source directory.
    with open(os.path.join(src_dir, "conf.py"), 'w') as w:
        w.write(get_conf_template(id_))

    params = ["-M", "html", src_dir, os.path.join(src_dir, "_build")]
    sphinx.cmd.build.make_main(params)
    return PipelineBuild(os.path.basename(src_dir), id_)

class VnVPipeline:

    COUNTER = 0
    FILES = {}

    def __init__(self, name, filename, template_root, pipeline_template_root, icon="icon-box"):
        self.name = name
        self.filename = filename
        self.icon = icon
        self.template_root = template_root
        self.pipeline_template_root = pipeline_template_root
        self.id_ = VnVPipeline.get_id()
        self.template_dir = os.path.join(pipeline_template_root, str(self.id_))

        self.notifications = []
        self.files = []
        self._read(filename)

    def _read(self,filename):
        config_file = os.path.join(filename, "config.json")

        self.templates = build_pipeline(self.template_dir, self.id_, filename)

        if os.path.exists(config_file):
            with open(config_file,'r') as f:
                config = json.load(f)
                for file in config.get("files",[]):
                    self.files.append(VnVFile.add(
                        file[0],os.path.join(self.filename,file[2]),file[1],self.template_root))

            for f in self.files:
                f.pipeline = True


    def render(self):
        return render_template(self.templates.get_content(), data=self)

    def getDataChildren(self):
        ret =  []
        for i in self.files:
            ret.append({
                "text" : i.name,
                "li_attr" : {
                    "fileId" : i.id_,
                    "nodeId" : "#",
                },
                "children" : True
            })
        return ret

    @staticmethod
    def get_id():
        VnVPipeline.COUNTER += 1
        return VnVPipeline.COUNTER

    @staticmethod
    def add(name, filename, template_root, pipeline_template_root):
        f = VnVPipeline(name, filename, template_root, pipeline_template_root)
        VnVPipeline.FILES[f.id_] = f
        return f

    @staticmethod
    def removeById(fileId):
        pipeline = VnVPipeline(fileId)
        for file in pipeline.files:
            VnVFile.removeById(file.id_)
        VnVPipeline.FILES.pop(fileId)

    @staticmethod
    def find(id_):
        if id_ in VnVPipeline.FILES:
            return VnVPipeline.FILES[id_]
        raise FileNotFoundError
