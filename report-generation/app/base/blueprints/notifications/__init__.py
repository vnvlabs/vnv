# -*- encoding: utf-8 -*-
"""
Copyright (c) 2019 - present AppSeed.us
"""

from flask import Blueprint

from app.models.VnVNotification import VnVNotification

blueprint = Blueprint(
    'notifications',
    __name__,
    template_folder='templates',
    url_prefix="/notifications"
)


@blueprint.route('/read/<int:id_>', methods=["POST"])
def mark_read(id_):
    VnVNotification.mark_read(id_, True)
    return "success", 200


@blueprint.route('/read-all', methods=["POST"])
def mark_read_all():
    VnVNotification.read_all(True)
    return "success", 200


@blueprint.route('/delete/<int:id_>', methods=["POST"])
def clear(id_):
    VnVNotification.clear(id_)
    return "success", 200


@blueprint.route('/delete-all', methods=["POST"])
def clear_all():
    VnVNotification.clear_all()
    return "success", 200


def template_globals(globs):
    globs["notifications"] = VnVNotification.NOTIFICATIONS


def faker():
    VnVNotification.add("test", "This is a message")
    VnVNotification.add("test1", "This is a message")
    VnVNotification.add("test2", "This is a message")
    VnVNotification.add("test3", "This is a message")
