# -*- encoding: utf-8 -*-
import glob
import json
import uuid

from flask import Blueprint, render_template, request, make_response, jsonify
from werkzeug.utils import redirect

from . import blueprints
from .utils.mongo import list_mongo_collections

from werkzeug.security import generate_password_hash, check_password_hash

blueprint = Blueprint(
    'base',
    __name__,
    url_prefix='',
    template_folder='templates'
)

AUTHENTICATE = False
PASSWORD = generate_password_hash("password")
COOKIE_PASS = uuid.uuid4().hex

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
        if not check_password_hash(PASSWORD,request.form["__token__"]):
            return make_response("Authorization Failed", 401)

    elif request.endpoint and request.endpoint != "base.login" and 'static' not in request.endpoint and not login_valid :
        return render_template('login.html', next=request.url)


blueprint.register_blueprint(
    blueprints.plugins.blueprint,
    url_prefix="/plugins")
blueprint.register_blueprint(blueprints.files.blueprint, url_prefix="/files")
blueprint.register_blueprint(
    blueprints.inputfiles.blueprint,
    url_prefix="/inputfiles")

blueprint.register_blueprint(
    blueprints.pipelines.blueprint,
    url_prefix="/pipelines")


blueprint.register_blueprint(
    blueprints.notifications.blueprint,
    url_prefix="/notifications")

blueprint.register_blueprint(
    blueprints.directives.blueprint,
    url_prefix="/directives")

@blueprint.route('/')
def default_route():
    return render_template('index.html', segment='index')

@blueprint.route('/login', methods=["POST"])
def login():

    if not AUTHENTICATE:
        return make_response(redirect("/"))

    if check_password_hash(PASSWORD,request.form.get("password")):
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


@blueprint.route("/")
def home():
    return render_template("index.html", segment="index")

def available_ports() :
    return range(14000,14010)



context_map = {
    "json_file" : lambda x : glob.glob(x + "*"),
    "adios_file": lambda x: glob.glob(x + "*"),
    "pipeline": lambda x: glob.glob(x + "*"),
    "json_http": lambda x : [str(a) for a in available_ports()],
    "json_socket": lambda x : [str(a) for a in available_ports()],
    "saved" : lambda x : list_mongo_collections()
}

def getContext(context,prefix):

    if context in context_map:
        return context_map[context](prefix)

    return glob.glob(prefix + "*")


@blueprint.route('/autocomplete')
def autocomplete():
    pref = request.args.get('prefix', '')

    context = request.args.get("context")
    if context :
        return make_response(jsonify(getContext(context,pref)),200)

    return make_response(jsonify(glob.glob(pref + "*")), 200)


def template_globals(d):
    blueprints.files.template_globals(d)
    blueprints.plugins.template_globals(d)
    blueprints.notifications.template_globals(d)
    blueprints.inputfiles.template_globals(d)
    blueprints.directives.template_globals(d)
    blueprints.pipelines.template_globals(d)


def faker():
    blueprints.files.faker()
    blueprints.pipelines.faker()
    blueprints.inputfiles.faker()
