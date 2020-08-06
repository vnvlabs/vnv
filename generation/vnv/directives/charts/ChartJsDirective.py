#!/usr/bin/env python
# -*- coding: utf-8 -*-
import jmespath
import json as jsonLoader
import os
import re
from docutils.parsers.rst import Directive
from docutils import nodes
from docutils.parsers.rst import directives

from ..utils.FakeDict import FakeDict, JmesTerm, process
from ...generate.configs import js_dir, add_js_files
from ...generate.HtmlToPng import getIFrame


class chart(nodes.General, nodes.Element):
    children = ()

    local_attributes = ('backrefs', 'content')


class VnVChartDirective(Directive):
    required_arguments = 0
    optional_arguments = 0
    file_argument_whitespace = True
    has_content = True
    idCount = 0

    option_spec = FakeDict(**{
        "width": directives.unchanged,
        "height": directives.unchanged
    })

    def getNewId(self):
        VnVChartDirective.idCount += 1
        return "vnv-chart-{}".format(VnVChartDirective.idCount)

    irs = "/" + js_dir + "iframeResizer.contentWindow.min.js"
    script_name = "Chart.min.js"
    css_name = "Chart.min.css"

    script_template = '''
     <div id="{id}_container" width="{width}" height="{height}">
     <canvas id="{id}"></canvas>
     </div>
     <script>
     const json = '{json}'
     const obj = JSON.parse(json)
     var ctx = document.getElementById('{id}');
     var myChart = new Chart(ctx, obj);
     </script>
     '''

    def getHtml(self, jsonObj):
        id_ = self.getNewId()
        return self.script_template.format(
            id=id_,
            json=jsonLoader.dumps(jsonObj),
            width=self.options.get("width", "100%"),
            height=self.options.get("height", "100%")
        )

    def getCss(self):
        if len(self.css_name) > 0:
            s = "../" + self.css_name
            return '''<link href="{css_name}" rel="stylesheet">'''.format(
                css_name=s)
        return ""

    def getScript(self):
        s = "../" + self.script_name

        return '''
          {css}
          <script src='{script_name}'></script>
          <script src='{iframe_resizer}'></script>

        '''.format(script_name=s, iframe_resizer=self.irs, css=self.getCss())

    def getIFrame(self, jsonObj):
        html = self.getHtml(jsonObj)
        return getIFrame(html,
                         header=self.getScript(),
                         width=self.options.get("width", "100%"))

    def run(self):
        env = self.state.document.settings.env
        if not hasattr(env, "vnv_current_node"):
            env.vnv_current_node = None
        cont = process(
            self.options,
            env.vnv_current_node,
            "".join(
                self.content))
        try:
            print("THIS", cont)
            cc = jsonLoader.loads(cont)

            html = self.getIFrame(cc)
            return [nodes.raw('', html, format="html")]
        except Exception as e:
            print(cont)
            raise RuntimeError("Could not parse json")


class VnVGoogleChartDirective(VnVChartDirective):
    script_template = '''
     <div id="{id}" style="width:"{width}"; height:"{height}"></div>
     <script>
       const json = '{json}'
       const obj = JSON.parse(json)
       obj['containerId'] = '{id}'

       google.charts.load('current');
       google.charts.setOnLoadCallback(drawVisualization);
       function drawVisualization() {{
        var wrapper = new google.visualization.ChartWrapper(obj);
        wrapper.draw();
       }}
     </script>

     '''
    script_name = "gcharts.min.js"
    css_name = ""

    def getHtml(self, jsonObj):
        return self.script_template.format(
            id=self.getNewId(),
            json=jsonLoader.dumps(jsonObj),
            width=self.options.get("width", "100%"),
            height=self.options.get("height", "400px")
        )


class VnVPlotlyChartDirective(VnVChartDirective):
    script_name = "plotly.min.js"
    css_name = ""
    script_template = '''
     <div id="{id}" style="width:"{width}"; height:"{height}"></div>
     <script>
       const data = '{json}'
       const obj = JSON.parse(data)
       Plotly.newPlot('{id}',obj['data'],obj['layout']);
     </script>

     '''

    def getHtml(self, jsonObj):
        return self.script_template.format(
            id=self.getNewId(),
            json=jsonLoader.dumps(jsonObj),
            width=self.options.get("width", "100%"),
            height=self.options.get("height", "400px")
        )


def convert_to_bool(mess):
    if mess is None or mess in ["False", "false", "0", "f", "F"]:
        return False
    elif mess in ["True", "true", "1", "t", "T"]:
        return True
    raise RuntimeError("Non boolean options requested")


def cust_com(strf):
    return directives.choice(strf, ("dark", "light"))


class VnVJsonViewerDirective(VnVChartDirective):
    option_spec = FakeDict(**{
        "width": directives.unchanged,
        "height": directives.unchanged,
        "open": int,
        "hoverPreviewEnabled": convert_to_bool,
        "hoverPreviewArrayCount": directives.positive_int,
        "hoverPreviewFieldCount": directives.positive_int,
        "theme": cust_com,
        "animateOpen": convert_to_bool,
        "animateClose": convert_to_bool,
        "useToJSON": convert_to_bool,
        "style": directives.unchanged
    })

    script_name = "json-formatter.js"
    css_name = ""

    script_template = '''
    <div id="{id}" class='vnv_jsonviewer' width="{width}" height="{height}" style='{style}'></div>
    <script>
       var open = {open};
       var data = JSON.parse('{data}');
       var config = JSON.parse('{config}');
       var table = new JSONFormatter(data,open, config);
       document.getElementById('{id}').appendChild(table.render());
    </script> '''

    def getHtml(self, jsonObj):
        return self.script_template.format(
            id=self.getNewId(), config=jsonLoader.dumps(
                {
                    k: v for k, v in self.options.items() if not isinstance(
                        v, JmesTerm)}), data=jsonLoader.dumps(jsonObj), open=self.options.get(
                "open", 1), height=self.options.get(
                "height", "400px"), width=self.options.get(
                            "width", "100%"), style=self.options.get(
                                "style", ""))


class VnVTableViewerDirective(VnVChartDirective):
    option_spec = FakeDict(**{
        "width": directives.unchanged,
        "height": directives.unchanged,
    })

    script_name = "tabulator.min.js"
    css_name = "tabulator.min.css"

    script_template = '''
        <div id="{id}" class='vnv-table' width="{width}" height="{height}"></div>
        <script>
            const obj = JSON.parse('{config}')
            var table = new Tabulator("#{id}", obj);
        </script>
    '''

    def getHtml(self, jsonObj):
        return self.script_template.format(
            id=self.getNewId(),
            height=self.options.get("height", 400),
            width=self.options.get("width", 400),
            config=jsonLoader.dumps(jsonObj)
        )


def on_environment_ready(app):
    # Make sure the javascript is available.
    src_dir = os.path.join(os.path.abspath(os.path.dirname(__file__)), "data")
    js_file = [
        "iframeResizer.contentWindow.min.js",
        "Chart.min.js",
        "Chart.min.css",
        "gcharts.min.js",
        "plotly.min.js",
        "json-formatter.js",
        "tabulator.min.js",
        "tabulator.min.css"]
    add_js_files(app.outdir, js_file, src_dir)

    app.config.html_static_path.append(src_dir)
    app.add_js_file("iframeResizer.min.js")


def setup(app):
    app.add_node(chart)
    app.add_directive("vnv-chart", VnVChartDirective)  # Charts.js chart
    app.add_directive("vnv-gchart", VnVGoogleChartDirective)  # Google chart
    app.add_directive("vnv-pchart", VnVPlotlyChartDirective)  # Plotly chart
    app.add_directive("vnv-jchart", VnVJsonViewerDirective)  # Raw Json Viewer
    app.add_directive("vnv-tchart", VnVTableViewerDirective)  # Table Viewer

    app.connect("builder-inited", on_environment_ready)
