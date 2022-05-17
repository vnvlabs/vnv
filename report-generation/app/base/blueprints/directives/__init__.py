# -*- encoding: utf-8 -*-
"""
Copyright (c) 2019 - present AppSeed.us
"""
import base64
import json
import os

import jinja2
from flask import Blueprint, render_template, make_response, request, jsonify, render_template_string
from jinja2 import FileSystemLoader

from app import Directory
from app.models.VnVFile import VnVFile
from app.models.VnVInputFile import VnVInputFile
from app.rendering.vnvdatavis.directives.chartsjs import chartsjs_post_process
from app.rendering.vnvdatavis.directives.dataclass import DataClass
from app.rendering.vnvdatavis.directives.forr import VnVForDirective
from app.rendering.vnvdatavis.directives.iff import VnVIfDirective
from app.rendering.vnvdatavis.directives.include import post_process_include
from app.rendering.vnvdatavis.directives.plotly import plotly_post_process
from app.rendering.vnvdatavis.directives.plotly_animation import PlotlyAnimation
from app.rendering.vnvdatavis.directives.apex import apex_post_process
from ...utils.utils import render_error
import os

blueprint = Blueprint(
    'directives',
    __name__,
    template_folder='templates',
    url_prefix="/directives"
)

context_map = {
    "plotly" : plotly_post_process,
    "for" : VnVForDirective.post_process,
    "if" : VnVIfDirective.post_process,
    "animation" : PlotlyAnimation.post_process,
    "apex": apex_post_process,
    "jscharts" : chartsjs_post_process,
    "include" : post_process_include
}

@blueprint.route('/updates/<updateId>/<int:fileid>/<int:dataid>', methods=["GET"])
def chartupdates(updateId, fileid, dataid):
    try:
        #This is an input file (usually the description.
        if fileid == VnVInputFile.VNVINPUTFILEDEF:
           with VnVInputFile.find(dataid) as file:
               with open(os.path.join(Directory.UPDATE_DIR, updateId), 'r') as w:
                   context = request.args.get("context", "")
                   data = DataClass(file, file.getId(), VnVInputFile.VNVINPUTFILEDEF)

                   if context in context_map:
                       config = context_map[context](w.read(), data, file)
                   else:
                       config = render_template_string(w.read(), data=data)
           d = {"more": False, "config": config}
           return make_response(json.dumps(d), 200)

        with VnVFile.find(fileid) as file:
            data = file.getById(dataid).cast()
            with open(os.path.join(Directory.UPDATE_DIR, updateId), 'r') as w:
                context = request.args.get("context","")
                if context in context_map:
                    config = context_map[context](w.read(), DataClass(data, dataid, fileid), file )
                else:
                    config = render_template_string(w.read(), data=DataClass(data, dataid, fileid))

            d = {"more": data.getopen(), "config": config}
            return make_response(json.dumps(d), 200)

    except Exception as e:
        return render_error(501, "Error Loading File")


@blueprint.route('/roles', methods=["POST"])
def roleupdates():
    try:
        resp = []
        r = request
        data = request.get_json()
        for d in data:

            if d[0] == VnVInputFile.VNVINPUTFILEDEF:
                with VnVInputFile.find(d[1]) as file:
                    data = DataClass(file, file.getId(), VnVInputFile.VNVINPUTFILEDEF)
                    q = base64.urlsafe_b64decode(d[2].encode("utf-8"))
                    resp.append([False, data.mquery(d[3], q.decode('ascii'))])
            else:
                with VnVFile.find(d[0]) as vnv:
                    data = vnv.getById(d[1]).cast()
                    q = base64.urlsafe_b64decode(d[2].encode("utf-8"))
                    resp.append([data.getopen(), DataClass(data, d[1], d[0]).mquery(d[3], q.decode('ascii'))])

        return make_response(jsonify(resp), 200)
    except Exception as e:
        return render_error(501, "Error Loading File")

def template_globals(globs):
    pass
