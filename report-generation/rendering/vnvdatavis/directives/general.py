import json
import os
import re

import docutils.nodes
from docutils.nodes import SkipNode
from docutils.parsers.rst import directives
from flask import render_template
from sphinx.directives import optional_int
from sphinx.directives.code import CodeBlock
from sphinx.errors import ExtensionError
from sphinx.util import nodes
from sphinx.util.docutils import SphinxDirective
import pygments
from pygments.lexers.data import JsonLexer
from pygments.formatters.html import HtmlFormatter

from rendering.vnvdatavis.directives import vnv_roles, vnv_directives, vnv_nodes, vnv_js_assets, vnv_css_assets
from rendering.vnvdatavis.directives.jmes import jmes_jinja_query_str, jmes_jinga_stat, DataClass, \
    jmes_jinja_codeblock, jmes_jinja_query, get_target_node

class VnVChartNode(docutils.nodes.General, docutils.nodes.Element):

    @staticmethod
    def visit_node(visitor, node):
        visitor.body.append(node["html"])
        raise SkipNode

    @staticmethod
    def depart_node(visitor, node):
        pass

VnVChartNode.NODE_VISITORS = {
        'html': (VnVChartNode.visit_node, VnVChartNode.depart_node)
}


def jmes_text_node(text, meth):
    result = meth(text)
    return [VnVChartNode(html=result)]


def jmes_role(name, rawtext, text, lineno, inliner, options={}, content=[]):
    return jmes_text_node(text, jmes_jinja_query_str), []


vnv_roles["vnv"] = jmes_role


################ ADD A BUNCH OF ROLES TO TAKE STATISTICS OF JMES RESTULT ##########################

def get_stats_role(stats_function):
    def role(name, rawtext, text, lineno, inliner, options={}, content=[]):
        r = jmes_jinga_stat(text, stats_function)
        return [docutils.nodes.Text(r, r)], []

    return role


for f in DataClass.statsMethods:
    vnv_roles[f"vnv-{f}"] = get_stats_role(f)


class JmesStringDirective(SphinxDirective):
    required_arguments = 1
    optional_arguments = 0

    final_argument_whitespace = True
    option_spec = {}
    has_content = False

    def run(self):
        return jmes_text_node(" ".join(self.arguments), jmes_jinja_query_str)


vnv_directives["vnv-print"] = JmesStringDirective


class JsonCodeBlockDirective(SphinxDirective):
    has_content = False
    required_arguments = 1
    optional_arguments = 0
    final_argument_whitespace = True
    option_spec = {}

    def run(self):
        return jmes_text_node(" ".join(self.arguments), jmes_jinja_codeblock)


vnv_directives["vnv-code"] = JsonCodeBlockDirective


########################## CHARTS/TABLES/TREES ################################################



class JsonChartDirective(SphinxDirective):
    required_arguments = 0
    optional_arguments = 0
    file_argument_whitespace = True
    has_content = True
    options_spec = {
        "height": optional_int,
        "width": optional_int
    }

    def getScript(self): return self.script_template

    def getContent(self):
        return re.sub('{{(.*?)}}', lambda x: jmes_jinja_query(x.group(1)), "\n".join(self.content))


    def getHtml(self, id_):
        return self.getScript().format(
            id_=id_,
            height=self.options.get("height", 400),
            width=self.options.get("width", 400),
            config=self.getContent()
        )

    def run(self):
        target, target_id = get_target_node(self)
        block = VnVChartNode(html=self.getHtml(target_id))
        return [target, block]


class PlotlyChartDirective(JsonChartDirective):
    script_template = '''
            <div id="{id_}" style="width:"{width}"; height:"{height}"></div>
            <script>
              const obj = JSON.parse('{config}')
              Plotly.newPlot('{id_}',obj['data'],obj['layout']);
            </script>
            '''


class ApexChartDirective(JsonChartDirective):
    script_template = '''
          <div id="{id_}" class='vnv-table' width="{width}" height="{height}"></div>
          <script>
            const obj = JSON.parse('{config}')
            var chart = new ApexCharts(document.querySelector("#{id_}"), obj);
            chart.render();
          </script>
        '''


class GChartChartDirective(JsonChartDirective):
    script_template = '''
            <div id="{id_}" style="width:"{width}"; height:"{height}"></div>
            <script>
              const json = '{config}'
              const obj = JSON.parse('{config}')
              obj['containerId'] = '{id_}'

              google.charts.load('current');
              google.charts.setOnLoadCallback(drawVisualization);
              function drawVisualization() {{
               var wrapper = new google.visualization.ChartWrapper(obj);
               wrapper.draw();
              }}
            </script>
        '''


class ChartJsChartDirective(JsonChartDirective):
    script_template = '''
           <div id="{id_}_container" width="{width}" height="{height}">
              <canvas id="{id_}"></canvas>
           </div>
           <script>
             const obj = JSON.parse('{config}')
             var ctx = document.getElementById('{id_}');
             var myChart = new Chart(ctx, obj);
           </script>
           '''


class TableChartDirective(JsonChartDirective):
    script_template = '''
         <div id="{id_}" class='vnv-table' width="{width}" height="{height}"></div>
         <script>
             const obj = JSON.parse('{config}')
             var table = new Tabulator("#{id_}", obj);
         </script>
         '''


class TreeChartDirective(JsonChartDirective):
    script_template = '''
        <div id="{id_}" class='vnv_jsonviewer' width="{width}" height="{height}"></div>
        <script>
           var data = JSON.parse('{config}');
           var tree = new JSONFormatter(data['data'], true ,data['config']);
           document.getElementById('{id_}').appendChild(table.render());
        </script> '''


vnv_nodes.append(VnVChartNode)
vnv_directives["vnv-apex"] = ApexChartDirective
vnv_directives["vnv-plotly"] = PlotlyChartDirective
vnv_directives["vnv-gchart"] = GChartChartDirective
vnv_directives["vnv-chart"] = ChartJsChartDirective
vnv_directives["vnv-table"] = TableChartDirective
vnv_directives["vnv-tree"] = TreeChartDirective

root = os.path.join("app", "static", "assets")
js_root = os.path.join(root, "js", "plugins")
css_root = os.path.join(root, "css", "plugins")

vnv_js_assets.append(os.path.join(js_root, "apexcharts.min.js"))
vnv_css_assets.append(os.path.join(css_root, "apexcharts.css"))
vnv_js_assets.append(os.path.join(js_root, "plotly.min.js"))
vnv_js_assets.append(os.path.join(js_root, "gcharts.min.js"))
vnv_js_assets.append(os.path.join(js_root, "Chart.min.js"))
vnv_css_assets.append(os.path.join(css_root, "Chart.min.css"))
vnv_js_assets.append(os.path.join(js_root, "tabulator.min.js"))
vnv_css_assets.append(os.path.join(css_root, "tabulator.min.css"))
vnv_js_assets.append(os.path.join(js_root, "json-formatter.umd.js"))
vnv_css_assets.append(os.path.join(css_root, "json-formatter.css"))


#####################  Image #######################

class JsonImageDirective(SphinxDirective):
    required_arguments = 1
    optional_arguments = 0
    file_argument_whitespace = True
    has_content = False
    options_spec = {
        "height": optional_int,
        "width": optional_int,
        "alt": directives.unchanged
    }

    script_tempate = '''
    <image  id="{id_} class="vnv_image" src="{src}" alt="{alt}" width="{width}" height="{height}"/>
    '''

    def getHtml(self, id_, content):
        return self.script_template.format(
            id_=id_,
            height=self.options.get("height", 400),
            width=self.options.get("width", ),
            src=content,
            alt=self.options.get("alt", "")
        )

    def run(self):
        j = jmes_jinja_query_str(" ".join(self.arguments))
        target, target_id = get_target_node(self)
        block = VnVChartNode(html=self.getHtml(target_id, j))
        return [target, block]


vnv_directives["vnv-image"] = JsonImageDirective


def setup(app):
    for node in vnv_nodes:
        app.add_node(node, **node.NODE_VISITORS)
    for key, value in vnv_roles.items():
        app.add_role(key, value)
    for key, value in vnv_directives.items():
        app.add_directive(key, value)
    for file in vnv_js_assets:
        app.add_js_file(file)
    for file in vnv_css_assets:
        app.add_css_file(file)
