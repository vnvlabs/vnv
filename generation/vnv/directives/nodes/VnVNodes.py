#!/usr/bin/env python
# -*- coding: utf-8 -*-
import sphinx
from docutils.parsers.rst import Directive
from docutils.statemachine import ViewList
from sphinx.util import docutils

from ...generate import configs
from ...vnv import VnVReader as OR

vnv_files = {}


def __read_from_file(reader, filename):
    return OR.Read(filename, reader, {})


def read_vnv_file(reader, filename):
    r = __read_from_file(reader, filename)
    vnv_files[filename] = r
    return r


def get_node_by_id(idr):
    for rootNode in vnv_files.values():
        if rootNode.getRootNode().hasId(idr):
            return rootNode.getRootNode.findById(idr)
    raise RuntimeError("Unrecognized Id")


def getRestructuredText(node):
    # This should return the restructured text for each node.
    # This can be configured,
    return configs.getGenerator().getRestructuredText(node)


# Include a vnv files root node.
class VnVIncludeDirective(Directive):
    required_arguments = 2
    optional_arguments = 0
    final_argument_whitespace = True
    has_content = True
    idCount = 0;
    option_spec = {
    }

    def run(self):
        # First, set the current node for the Injection point.
        reader = self.arguments[0]
        filename = " ".join(self.arguments[1:])
        rootNode = read_vnv_file(reader, filename)

        env = self.state.document.settings.env
        rn = rootNode.getRootNode()

        # Second, replace myself with a directive for defining the node. .
        unparsedRestructuredText = ".. vnv-node:: {id}\n".format(id=rn.getId())
        node = docutils.nodes.paragraph()
        result = ViewList(unparsedRestructuredText.splitlines(), source="")
        sphinx.util.nodes.nested_parse_with_titles(self.state, result, node)
        return [node]


class VnVNodeDirective(Directive):
    required_arguments = 1
    optional_arguments = 0
    final_argument_whitespace = True
    has_content = True
    idCount = 0;
    option_spec = {
        "show", int
    }

    def run(self):
        # First, set the current node for the Injection point.
        env = self.state.document.settings.env
        env.vnv_current_node = configs.getNodeById("".join(self.arguments))

        # Second, ask the configs component to return the raw text for this node.
        unparsedRestructuredText = getRestructuredText(env.vnv_current_node)
        node = docutils.nodes.paragraph()
        result = ViewList(unparsedRestructuredText.splitlines(), source="")
        sphinx.util.nodes.nested_parse_with_titles(self.state, result, node)
        return [node]


def setup(app):
    app.add_directive("vnv-node", VnVNodeDirective)
    app.add_directive("vnv-include", VnVIncludeDirective)
