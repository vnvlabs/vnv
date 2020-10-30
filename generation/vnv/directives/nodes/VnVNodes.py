#!/usr/bin/env python
# -*- coding: utf-8 -*-
import json
from io import StringIO

import sphinx
from docutils.parsers.rst import Directive, directives, languages
from docutils.statemachine import ViewList
from sphinx.util import docutils, nodes
from docutils.writers.html4css1 import HTMLTranslator
from ..utils.FakeDict import FakeDict, process
from ...generate import configs
from ...vnv import VnVReader as OR

vnv_files = {}


def __read_from_file(reader, filename):
    return OR.Read(filename, reader, "{}")


def read_vnv_file(reader, filename):
    # __read_from_file returns a VnV::ReaderWrapper. This is a wrapper
    # around a rootNode. The wrapper contains a c++ shared pointer to the
    # rootNode. That means the rootNode is FREED when the wrapper is destroyed.
    # So, we must store the wrapper for as long as we need the rootNode.
    r = __read_from_file(reader, filename)
    vnv_files[filename] = r
    return r


def get_node_by_id(env, idr: int):
    # Note, the ids are fixed for the given execution of VnVReader
    # but can change from run to run. Basically, the id increases by
    # one each time a new node is added. So, if the order in which files
    # are loaded changes, so does the ids.
    if hasattr(env, "vnv_current_reader") and env.vnv_current_reader:
        rn = env.vnv_current_reader.get()

        print(idr, rn.hasId(idr))
        if rn.hasId(idr):
            return rn.findById(idr)

        # Look for the id in the saved nodes if any
        if hasattr(env, "vnv_saved_readers"):
            for i in env.vnv_saved_readers:
                if i.get().hasId(idr):
                    return i.get().findById(idr)

    raise RuntimeError("Unrecognized Id")


def getRestructuredText(node):
    # This should return the restructured text for each node.
    # This can be configured,
    return configs.getGenerator().getRestructuredText(node)


class VnVLogDirective(Directive):
    required_arguments = 0
    optional_arguments = 0
    final_argument_whitespace = False
    has_content = True
    option_spec = {
        "package": directives.unchanged,
        "level": directives.unchanged,
        "stage": directives.unchanged
    }

    emap = {
        "Error": "error",
        "Warning": "warning",
        "Info": "important",
        "Debug": "caution",
    }

    def run(self):
        mess = "\n".join(self.content)
        pack = self.options.get("package", "<unknown-package>")
        level = self.options.get("level", "Error")
        stage = self.options.get("stage", "<unknown-stage>")

        print(languages.get_language(self.state.document.settings.language_code))
        message = "{package}[{stage}]: {message}".format(
            package=pack, stage=stage, message=mess).splitlines()
        stream = StringIO()
        stream.write(
            "\n.. {type}::\n\n".format(
                type=self.emap.get(
                    level, "hint")))
        for i in message:
            stream.write("    {mess}".format(mess=i))

        node = docutils.nodes.Element()
        result = ViewList(stream.getvalue().splitlines(), source="")
        self.state.nested_parse(result, 0, node)
        #sphinx.util.nodes.nested_parse_with_titles(self.state, result, node)
        return [node]


# Include a vnv files root node.
class VnVIncludeDirective(Directive):
    required_arguments = 2
    optional_arguments = 0
    final_argument_whitespace = True
    has_content = False
    idCount = 0
    option_spec = {
        "save": directives.unchanged,
        "title": directives.unchanged,
        "commmap" : directives.positive_int
    }

    def run(self):
        # First, set the current node for the Injection point.
        env = self.state.document.settings.env

        reader = self.arguments[0]
        filename = " ".join(self.arguments[1:])

        env.vnv_current_reader = OR.Read(filename, reader, "{}")

        # If save was requested, then add this to our current reader.
        if "save" in self.options:
            if not hasattr(env, "vnv_saved_readers"):
                env.vnv_saved_readers = {}
            env.vnv_saved_readers[self.options["save"]] = [
                env.vnv_current_reader, filename, reader]

        if "title" in self.options:
            configs.getGenerator().setTitle(self.options['title'])
        if "commmap" in self.options:
            commMap = env.vnv_current_reader.get().getCommMap()
            worldSize = env.vnv_current_reader.get().getWorldSize()
            configs.getGenerator().setCommMap(2)

        # Second, replace myself with a directive for defining the node. .
        unparsedRestructuredText = "\n\n.. vnv-node:: {id}\n\n".format(
            id=env.vnv_current_reader.get().getId())
        node = docutils.nodes.paragraph()
        result = ViewList(unparsedRestructuredText.splitlines(), source="")
        sphinx.util.nodes.nested_parse_with_titles(self.state, result, node)

        env.vnv_current_reader = None
        env.vnv_current_node = None

        return [node]


class VnVForgetSavedDirective(Directive):
    required_arguments = 1
    optional_arguments = 0
    final_argument_whitespace = True
    has_content = False
    idCount = 0
    option_spec = {
    }

    def run(self):
        id_ = " ".join(self.arguments)
        env = self.state.document.settings.env
        if hasattr(env, "vnv_saved_readers") and id_ in env.vnv_saved_readers:
            if env.vnv_saved_readers[id_] == env.vnv_current_reader:
                env.vnv_current_reader = None
                env.vnv_current_node = None
            del env.vnv_saved_readers[id]
        return []


# Set the current reader based on the result of a previous save point.
# Set the current node to be the root node.
class VnVSetReaderDirective(Directive):
    required_arguments = 1
    optional_arguments = 0
    final_argument_whitespace = True
    has_content = False
    idCount = 0
    option_spec = {
    }

    def run(self):
        id_ = " ".join(self.arguments)
        env = self.state.document.settings.env
        if hasattr(env, "vnv_saved_readers") and id_ in env.vnv_saved_readers:
            env.vnv_current_reader = env.vnv_saved_readers[id_][0]
            env.vnv_current_node = env.vnv_current_reader.get()
        return []


class VnVNodeDirective(Directive):
    required_arguments = 1
    optional_arguments = 0
    final_argument_whitespace = True
    has_content = True
    idCount = 0
    option_spec = {
        "show", int
    }

    def run(self):
        # First, set the current node for the Injection point.
        env = self.state.document.settings.env

        id = int("".join(self.arguments))
        rn = env.vnv_current_reader.get()

        env.vnv_current_node = get_node_by_id(env, id)

        # Second, ask the configs component to return the raw text for this
        # node.
        rawRestructuredText = getRestructuredText(env.vnv_current_node)
        node = docutils.nodes.paragraph()
        result = ViewList(rawRestructuredText.splitlines(), source="")
        sphinx.util.nodes.nested_parse_with_titles(self.state, result, node)
        return [node]


'''
  Process directive allows vnv data elements to supply restructed text. This
  is useed in the printf test to allow users to inject restructured text into
  testing points through the input file.

  This directive supports a process that the vnv-toolkit is designed to eliminate
  (hard coded i/o), but we support it none the less. If you find yourself using this
  directive, maybe consider refactoring so data elements are injected into static
  strings, rather than writing strings as data.
'''


class VnVProcessDirective(Directive):
    required_arguments = 0
    optional_arguments = 0
    final_argument_whitespace = False
    option_spec = FakeDict()

    def run(self):
        env = self.state.document.settings.env
        node = env.vnv_current_node
        cont = "\n".join(self.content)
        text = process(self.options, node, cont, formatter=str, def_val="")
        snode = docutils.nodes.paragraph()
        sphinx.util.nodes.nested_parse_with_titles(self.state, text, snode)
        return [snode]



# Set the env.vnv_current_node explicitly. This is to allow including sub-templates
# without calling vnv-node (which always sets the current node to itself. This can
# also be used to reset the node after calling children,
class VnVIdDirective(Directive):
    required_arguments = 1
    optional_arguments = 0
    final_argument_whitespace = True
    option_spec = {}
    has_content = True

    def run(self):
        env = self.state.document.settings.env
        idR = int("".join(self.arguments))
        env.vnv_current_node = get_node_by_id(env, idR)
        return []


# TODO GET CORRECT RESULTS FROM MAPNODE.
results_chart_temp = '''
.. vnv-tchart::

   {json}

'''


def chart_tem(data, dataTree = False):
    return {"data": data,
            "layout": "fitColumns",
            "tooltips": True,
            "dataTree" : dataTree,
            "columns": [{"title": "Name",
                         "field": "name"},
                        {"title": "Pass/Fail",
                         "field": "value",
                         "width": 90,
                         "hozAlign": "center",
                         "formatter": "tickCross",
                         "sorter": "boolean"}]}



def getResultsTableForData(data,dataTree):
     dd = chart_tem(data,dataTree)
     temp = results_chart_temp.format(json=json.dumps(dd))
     return "\n\n{}\n\n".format(temp)

class VnVResultsDirective(Directive):
    required_arguments = 0
    optional_arguments = 0
    final_argument_whitespace = False
    option_spec = {}
    has_content = False

    def run(self):
        env = self.state.document.settings.env
        n = OR.castDataBase(env.vnv_current_node)
        if hasattr(n, "getResults"):
            j = []
            for jj in n.getResults():
                nn = OR.castDataBase(n.getResults()[jj])
                j.append({"name": nn.getName(), "value": nn.getValue()})

            dd = chart_tem(j)
            temp = results_chart_temp.format(json=json.dumps(chart_tem(j)))

            print(temp)
            # Second, replace myself with a directive for defining the node. .
            unparsedRestructuredText = "\n\n{}\n\n".format(temp)
            node = docutils.nodes.paragraph()
            result = ViewList(unparsedRestructuredText.splitlines(), source="")
            sphinx.util.nodes.nested_parse_with_titles(
                self.state, result, node)
            return [node]
        print(n)
        raise RuntimeError(
            "Cannot Call vnv-results-table on node with no results field.")


def setup(app):
    # Read a file and set it to current.
    app.add_directive("vnv-read", VnVIncludeDirective)
    # Forget a saved file. Set current to None if it
    app.add_directive("vnv-forget", VnVForgetSavedDirective)
    # Set the current reader and current node.
    app.add_directive("vnv-set-reader", VnVSetReaderDirective)

    app.add_directive("vnv-node", VnVNodeDirective)  # Write a node out
    app.add_directive("vnv-set-id", VnVIdDirective)  # Set the current node
    # Set the current reader and current node.
    app.add_directive("vnv-process", VnVProcessDirective)
    app.add_directive("vnv-log", VnVLogDirective)  # Write a VnV Log
    app.add_directive("vnv-unit-test-results", VnVResultsDirective)
