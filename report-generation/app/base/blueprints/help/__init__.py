# -*- encoding: utf-8 -*-
"""
Copyright (c) 2019 - present AppSeed.us
"""

from flask import Blueprint, render_template, request, make_response, jsonify

blueprint = Blueprint(
    'help',
    __name__,
    template_folder='templates',
    url_prefix="/help"
)


@blueprint.route('/topic')
def help():
    try:
        key=request.args.get("key","error")
        return render_template("help/" + key + ".html")
    except Exception as e:
        return render_template("help/error.html")


def template_globals(globs):
    pass
