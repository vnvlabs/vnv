# -*- encoding: utf-8 -*-
import glob
import json
import os
import shutil
import uuid

from flask import Blueprint, render_template, request, make_response, jsonify, send_file
from werkzeug.utils import redirect

from . import blueprints
from .blueprints.files import get_file_template_root
from .utils.mongo import list_mongo_collections

from werkzeug.security import generate_password_hash, check_password_hash
from app.Directory import VNV_DIR_PATH

from .. import Directory
from ..models.VnVFile import VnVFile
from ..models.VnVInputFile import VnVInputFile

blueprint = Blueprint(
    'base',
    __name__,
    url_prefix='',
    template_folder='templates'
)

AUTHENTICATE = False
PASSWORD = generate_password_hash("password")
COOKIE_PASS = uuid.uuid4().hex


def config(conf):
    global PASSWORD
    PASSWORD = generate_password_hash(conf.passw)
    global AUTHENTICATE
    AUTHENTICATE = conf.auth


def GET_COOKIE_TOKEN():
    return COOKIE_PASS


def verify_cookie(cook):
    if cook is not None and cook == COOKIE_PASS:
        return True
    return False


@blueprint.before_request
def check_valid_login():
    if not AUTHENTICATE:
        return

    login_valid = verify_cookie(request.cookies.get("vnv-login"))
    if request.form.get("__token__") is not None:
        if not check_password_hash(PASSWORD, request.form["__token__"]):
            return make_response("Authorization Failed", 401)

    elif request.endpoint and request.endpoint != "base.login" and 'static' not in request.endpoint and not login_valid:
        return render_template('login.html', next=request.url)


blueprint.register_blueprint(blueprints.files.blueprint, url_prefix="/files")

blueprint.register_blueprint(blueprints.tempfiles.blueprint, url_prefix="/temp")
blueprint.register_blueprint(blueprints.help.blueprint, url_prefix="/help")

blueprint.register_blueprint(
    blueprints.inputfiles.blueprint,
    url_prefix="/inputfiles")

blueprint.register_blueprint(
    blueprints.notifications.blueprint,
    url_prefix="/notifications")

blueprint.register_blueprint(
    blueprints.directives.blueprint,
    url_prefix="/directives")


@blueprint.route('/')
def default_route():
    return render_template('index.html', segment='index')


@blueprint.route('/avatar/<username>')
def avatar_route(username):
    return send_file("static/assets/images/user/AVATARS.png")


@blueprint.route('/login', methods=["POST"])
def login():
    if not AUTHENTICATE:
        return make_response(redirect("/"))

    if check_password_hash(PASSWORD, request.form.get("password")):
        response = make_response(redirect("/"))
        response.set_cookie('vnv-login', GET_COOKIE_TOKEN())
        return response
    return render_template("login.html", error=True)


@blueprint.route('/logout', methods=["GET"])
def logout():
    global COOKIE_PASS
    COOKIE_PASS = uuid.uuid4().hex
    response = make_response(redirect("/"))
    response.set_cookie('vnv-login', "", expires=0)

    return response


@blueprint.route("icon")
def icon():
    p = request.args.get("package")
    return send_file("static/assets/images/close.png")


@blueprint.route("/")
def home():
    return render_template("index.html", segment="index")


def available_ports():
    return range(14000, 14010)


context_map = {
    "json_file": lambda x: glob.glob(x + "*"),
    "adios_file": lambda x: glob.glob(x + "*"),
    "json_http": lambda x: [str(a) for a in available_ports()],
    "json_socket": lambda x: [str(a) for a in available_ports()],
    "saved": lambda x: list_mongo_collections()
}


def getContext(context, prefix):
    if context in context_map:
        return context_map[context](prefix)

    return glob.glob(prefix + "*")


@blueprint.route('/autocomplete')
def autocomplete():
    pref = request.args.get('prefix', '')

    context = request.args.get("context")
    if context:
        return make_response(jsonify(getContext(context, pref)), 200)

    return make_response(jsonify(glob.glob(pref + "*")), 200)

LOGO_SMALL = os.path.join(VNV_DIR_PATH , "static/assets/images/logo.png")
LOGO_LARGE = os.path.join(VNV_DIR_PATH ,"static/assets/images/logo_large.png")
LOGO_ICON =  os.path.join(VNV_DIR_PATH, "static/assets/images/favicon.ico")
COPYRIGHT_LINK="mailto:boneill@rnet-tech.com"
COPYRIGHT_MESSAGE="RNET Technologies Inc. 2022"

def template_globals(d):
    d["COPYRIGHT_LINK"] = COPYRIGHT_LINK
    d["COPYRIGHT_MESSAGE"] = COPYRIGHT_MESSAGE

    blueprints.files.template_globals(d)
    blueprints.notifications.template_globals(d)
    blueprints.inputfiles.template_globals(d)
    blueprints.directives.template_globals(d)

def updateBranding(config, pd):
    logo = config.get("logo",{})
    if "small" in logo and os.path.exists(os.path.join(pd,logo["small"])):
        shutil.copy(os.path.join(pd,logo["small"]),LOGO_SMALL )
    if "large" in logo and os.path.exists(os.path.join(pd, logo["large"])):
        shutil.copy(os.path.join(pd,logo["large"]),LOGO_LARGE )
    if "icon" in logo and os.path.exists(os.path.join(pd, logo["icon"])):
        shutil.copy(os.path.join(pd,logo["icon"]),LOGO_ICON )

    copy = config.get("copyright",{})
    if "message" in copy:
        global COPYRIGHT_MESSAGE
        COPYRIGHT_MESSAGE = copy["message"]
        print("Update Copyright Message", COPYRIGHT_MESSAGE)

    if "link" in copy:
        global COPYRIGHT_LINK
        COPYRIGHT_LINK = copy["link"]
        print("Update Copyright Link", COPYRIGHT_LINK)


def load_default_data(loadIt):
    if not loadIt: return

    a = os.getenv("VNV_DEFAULT_REPORTS")
    if a is not None:

        for file in a.split(":"):
            try:
                with open(file, 'r') as w:
                    pd = os.path.dirname(file);
                    config = json.load(w)

                    for key, value in config.get("executables",{}).items():
                        blueprints.inputfiles.vnv_executables[key] = [ os.path.join(pd,value["filename"]), value.get("description", "") ]

                    for key,value in config.get("plugins",{}).items():
                        blueprints.inputfiles.vnv_plugins[key] = value

                    blueprints.files.load_defaults(config.get("reports",{}))

                    updateBranding(config.get("branding",{}),pd)

            except Exception as e:
                print(e)
                pass

    blueprints.inputfiles.vnv_executables["Custom"] = ["", "Custom Application"]
