# -*- encoding: utf-8 -*-
"""
Copyright (c) 2019 - present AppSeed.us
"""
import base64
import json
import os

import flask
import jinja2
from flask import Blueprint, render_template, make_response, request, jsonify, render_template_string
from jinja2 import FileSystemLoader

from app import Directory
from app.models.VnVFile import VnVFile
from app.rendering.vnvdatavis.directives.dataclass import DataClass
from app.rendering.vnvdatavis.directives.plotly import plotly_post_process
from ...utils.utils import render_error
import os

blueprint = Blueprint(
    'temporary',
    __name__,
    template_folder=Directory.TEMP_TEMPLATE_DIR
)


@blueprint.route('/files/<uid>', methods=["GET"])
def getfile(uid):
    return flask.send_from_directory(Directory.STATIC_FILES_DIR, uid)


def template_globals(globs):
    pass
