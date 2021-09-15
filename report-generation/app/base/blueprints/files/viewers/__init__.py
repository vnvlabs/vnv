from flask import Blueprint, request, render_template, jsonify

from app.base.utils.utils import render_error
from app.models import VnVFile

blueprint = Blueprint(
    'viewers',
    __name__,
    url_prefix='/viewers',
    template_folder='templates'
)


def template_globals(globs):
    globs["viewers"] = ["processor", "communicator", "graph"]


@blueprint.route('/render/<int:id_>')
def render(id_):
    try:
        with VnVFile.VnVFile.find(id_) as file:
            return render_template("viewers/view.html", file=file)

    except Exception as e:
        return render_error(501, "Error Loading File")


@blueprint.route('/package/<int:id_>')
def package(id_):
    try:
        with VnVFile.VnVFile.find(id_) as file:
            pack = request.args.get("p")
            return file.render_package(pack)
    except Exception as e:
        return render_error(501, "Error Loading File")


@blueprint.route('/unit/<int:id_>')
def unit(id_):
    try:

        pack = request.args.get("uid", type=int)
        with VnVFile.VnVFile.find(id_) as file:
            return file.render_unit_test(pack)

    except Exception as e:
        return render_error(501, "Error Loading File")


@blueprint.route('/ip/<int:id_>')
def ip(id_):
    try:
        with VnVFile.VnVFile.find(id_) as file:

            injection = request.args.get('ipid', type=int)
            iprender = file.render_ip(injection)

            if isinstance(iprender, str):
                return iprender

            resp = render_template(
                "viewers/injectionPoint.html",
                iprender=iprender)
            file.release()
            return resp

    except Exception as e:
        return render_error(501, "Error Loading File")


@blueprint.route("next")
def next():
    id_ = request.args.get("id_", type=int)
    count = request.args.get("count", 100, type=int)
    try:
        with VnVFile.VnVFile.find(id_) as file:
            data = file.proc_iter_next(count)
            return jsonify(data), 200
    except BaseException:
        return render_error(501, "Error Loading File")
