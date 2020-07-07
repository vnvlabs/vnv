#!/usr/bin/env python
# -*- coding: utf-8 -*-
import jmespath
import json as jsonLoader
import os
import shutil
import uuid
import csv

from docutils.parsers.rst import Directive
from docutils import nodes
from ..jmes import JmesDirective
from ...generate import configs


def get_new_id():
    VnVTableDirective.idCount += 1
    return "vnv_table_{}".format(VnVTableDirective.idCount)


class VnVTableDirective(Directive):
    required_arguments = 1
    optional_arguments = 1
    final_argument_whitespace = True
    has_content = True
    idCount = 0;
    option_spec = {
        "width", int,
        "height", int
    }

    script_template = '''
                   <div id="{id}" class='vnv-table' width="{width}" height="{height}"></div>
                   <script>
                     function {id}_func() {{
                       const obj = JSON.parse('{config}')
                       var table = new Tabulator("#{id}", obj);
                       table.setData({data})
                     }}
                     {id}_func()
                   </script>
                   '''
    def get_html(self, url):
        return self.script_template.format(
            id=get_new_id(),
            height=self.options.get("height", 400),
            data="'" + url + "'",
            width=self.options.get("width", 400),
            config=jsonLoader.dumps(self.content)
        )

    def run(self):
        env = self.state.document.settings.env
        file_type = self.arguments[0]
        file_name = "".join(self.arguments[1:])
        src_dir = os.path.join(configs.getApp().srcdir, os.path.dirname(env.docname))
        file_path = JmesDirective.getFilePath(file_name, env.vnv_current_node, src_dir)

        writer = None if file_type != "csv" else convert_csv_to_json

        url = JmesDirective.getUrl(file_path, configs.getApp().outdir, env, writer=writer)
        html = self.getHtml(url)
        return [nodes.raw('', html, format='html')]


def convert_csv_to_json(read_path, write_path):
    with open(read_path, 'r') as f:
        a = [row for row in csv.DictReader(f, skipinitialspace=True)]
        with open(write_path, 'w') as ff:
            ff.write(jsonLoader.dumps(a))


def on_environment_ready(app):
    # Make sure the javascript is available.
    js_dir = os.path.join(os.path.abspath(os.path.dirname(__file__)), "data")
    js_files = ["js/tabulator.min.js"]
    css_files = ["css/bootstrap/tabulator_bootstrap4.min.css", "css/bootstrap/tabulator_bootstrap4.min.css.map"]

    app.config.html_static_path.append(js_dir)
    for js_file in js_files:
        app.add_js_file(js_file)
    for css_file in css_files:
        app.add_css_file(css_file)


def setup(app):
    app.add_directive("vnv-table", VnVTableDirective)
    app.connect("builder-inited", on_environment_ready)
