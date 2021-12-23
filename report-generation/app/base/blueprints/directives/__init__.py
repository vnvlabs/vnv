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

from app.models.VnVFile import VnVFile
from app.rendering.vnvdatavis.directives.dataclass import DataClass
from app.rendering.vnvdatavis.directives.plotly import plotly_post_process
from ...utils.utils import render_error
import os

blueprint = Blueprint(
    'directives',
    __name__,
    template_folder='templates'
)

UPDATE_DIR = os.path.join(os.getcwd(), "updates")

@blueprint.route('/updates/<updateId>/<int:fileid>/<int:dataid>', methods=["GET"])
def chartupdates(updateId, fileid, dataid):
    try:
        with VnVFile.find(fileid) as file:
            data = file.getById(dataid).cast()
            with open(os.path.join(UPDATE_DIR, updateId + ".html"), 'r') as w:
                if "plotly" in request.args:
                    config = plotly_post_process(w.read(), data=DataClass(data, dataid, fileid))
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
            with VnVFile.find(d[0]) as vnv:
                data = vnv.getById(d[1]).cast()
                q = base64.urlsafe_b64decode(d[2].encode("utf-8"))
                resp.append([data.getopen(), DataClass(data, d[1], d[0]).mquery(d[3], q.decode('ascii'))])

        return make_response(jsonify(resp), 200)
    except Exception as e:
        return render_error(501, "Error Loading File")


def template_globals(globs):
    pass
