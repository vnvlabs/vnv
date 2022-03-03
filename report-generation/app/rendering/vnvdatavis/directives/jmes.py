import json
import os
import re

import docutils.nodes

from sphinx.errors import ExtensionError

# Fake jmes
from . import fakejmes as jmespath



def get_target_node(directive):
    serial_no = directive.env.new_serialno("ccb")
    target_id = f"vnv-{serial_no}"
    targetnode = docutils.nodes.target('', '', ids=[target_id])
    return targetnode, target_id


def jmes_jinja_query(text):
    if jmespath.compile(text):
        return "{{ data.query('" + text + "') | safe }}"
    else:
        raise ExtensionError("Invalid jmes path query")

def jmes_jinja_if_query(text):
    if jmespath.compile(text):
        return "{%if data.query('" + text + "') %}"
    else:
        raise ExtensionError("Invalid jmes path query")


def jmes_jinja_query_str(text):
    if jmespath.compile(text):
        return "{{ data.query_str('" + text + "') | safe}}"
    else:
        raise ExtensionError("Invalid jmes path query")


def jmes_jinja_query_json(text):
    if jmespath.compile(text):
        return "{{ data.query_json('" + text + "') | safe}}"
    else:
        raise ExtensionError("Invalid jmes path query")

def jmes_jinja_query_join(text):
    if jmespath.compile(text):
        return "{{ data.query_join('" + text + "') | safe}}"
    else:
        raise ExtensionError("Invalid jmes path query")

def jmes_jinja_query_str_array(text):
    return "{{ data.query_str_array('" + text + "') | safe}}"


def jmes_check(text):
    return jmespath.compile(text)


def jmes_jinja_zip(param):
    for i in param:
        if not jmespath.compile(param[i]):
            raise ExtensionError("Invalid jmes path query")
    return f"{{{{ data.query_zip('{json.dumps(param)}') | safe }}}}"


def jmes_jinja_percentage(curr,min,max):
    if jmespath.compile(curr) and jmespath.compile(min) and jmespath.compile(max):
        return f"{{{{ data.query_percent('{curr}','{min}','{max}') | safe }}}}"
    else:
        raise ExtensionError("Invalid jmes path query")


def jmes_jinja_codeblock(text):
    if jmespath.compile(text):
        return "{{ data.codeblock('" + text + "')| safe}}"
    else:
        raise ExtensionError("Invalid jmes path query")


def jmes_jinga_stat(text, meth):
    if jmespath.compile(text):
        return f"{{{{ data.mquery('{meth}','{text}') | safe}}}}"
    else:
        raise ExtensionError("Invalid jmes path query")

def setup(sapp):
    global the_app
    the_app = sapp

