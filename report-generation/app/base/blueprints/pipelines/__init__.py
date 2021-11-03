# -*- encoding: utf-8 -*-
"""
Copyright (c) 2019 - present AppSeed.us
"""
import json
import os
from flask import Blueprint, make_response, jsonify
from flask import render_template, redirect, url_for, request

from app.models.VnVPipeline import VnVPipeline
from app.models.VnVInputFile import VnVInputFile
from ..files import get_file_template_root
from ...utils.utils import render_error

blueprint = Blueprint(
    'pipelines',
    __name__,
    template_folder='templates'
)

def get_pipeline_file_template_root():
    sdir = os.path.join(
        os.path.abspath(
            os.path.dirname(__file__)),
        blueprint.template_folder)
    return os.path.abspath(os.path.join(sdir, "pipeline_renders"))



@blueprint.route('/new', methods=["POST"])
def new():
    try:

        file = VnVPipeline.add(
            request.form["name"],
            request.form["def"],
            get_file_template_root(),
            get_pipeline_file_template_root()
        )
        return redirect(url_for("base.pipelines.view", id_=file.id_))
    except Exception as e:
        return render_error(501, "Error Loading File")


@blueprint.route('/delete/<int:id_>', methods=["POST"])
def delete(id_):
    VnVPipeline.removeById(id_)
    return "success", 200

@blueprint.route('/data/<int:id_>', methods=["GET"])
def data(id_):

    # One layer of children -- all others just issue children = ""
    node = request.args.get("id", "#")

    if node == "#":
        file = VnVPipeline.find(id_)
        return make_response(jsonify( file.getDataChildren() ), 200)

    else:
        return make_response(jsonify([]),200)


@blueprint.route('/view/<int:id_>')
def view(id_):
    try:
        file = VnVPipeline.find(id_)
        return render_template("pipelines/view.html", file=file)
    except Exception as e:
        print(e)
        return render_error(501, "Error Loading File")


def template_globals(globs):
    globs["pipelines"] = VnVPipeline.FILES

def faker():
    if os.path.exists("/home/ben/source/vv/vv-neams/build/examples/dummy/executables/pipeline"):
        VnVPipeline.add(
            "test pipe",
            "/home/ben/source/vv/vv-neams/build/examples/dummy/executables/pipeline",
            get_file_template_root(),
            get_pipeline_file_template_root()
        )