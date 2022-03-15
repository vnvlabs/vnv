import json
import os.path
import re

from docutils.nodes import SkipNode
from flask import render_template_string
from sphinx.errors import ExtensionError
from sphinx.util import nested_parse_with_titles, docutils

from .charts import JsonChartDirective, VnVChartNode
from collections.abc import MutableMapping

from .forr import VnVForDirective
from .jmes import get_target_node
from .plotly import PlotlyDirec, PlotlyOptionsDict, plotly_post_process, \
    plotly_post_process_raw

updatemenus = [{
    "x": 0,
    "y": 0,
    "yanchor": "top",
    "xanchor": "left",
    "showactive": False,
    "direction": 'left',
    "type": 'buttons',
    "pad": {"t": 87, "r": 20},
    "buttons": [{
        "method": 'animate',
        "args": [None, {
            "mode": 'immediate',
            "fromcurrent": True,
            "transition": {"duration": 300},
            "frame": {"duration": 500, "redraw": True}
        }],
        "label": 'Play'
    }, {
        "method": 'animate',
        "args": [[None], {
            "mode": 'immediate',
            "transition": {"duration": 0},
            "frame": {"duration": 0, "redraw": True}
        }],
        "label": 'Pause'
    }]
}]


def getSlider(steps, prefix="Index"):
    return [{
        "pad": {"l": 130, "t": 55},
        "currentvalue": {
            "visible": True,
            "prefix": prefix + ": ",
            "xanchor": 'right',
            "font": {"size": 20, "color": '#666'}
        },
        "steps": [
            {"method": 'animate',
             "label": str(i),
             "args": [[str(i)], {
                 "mode": 'immediate',
                 "transition": {"duration": 500},
                 "frame": {"duration": 500, "redraw": True},
             }]
             } for i, v in steps.items()]
    }]


def getFrames(steps):
    return [
        {
            "name": str(k),
            "data": v.get("data", {}),
            "layout": v.get("layout", {})
        } for k, v in steps.items()
    ]


class VnVAnimatedNode(docutils.nodes.General, docutils.nodes.Element):

    @staticmethod
    def visit_node(visitor, node):
        visitor.body.append(node["html"])
        raise SkipNode

    @staticmethod
    def depart_node(visitor, node):
        pass


VnVAnimatedNode.NODE_VISITORS = {
    'html': (VnVAnimatedNode.visit_node, VnVAnimatedNode.depart_node)
}


class PlotlyAnimation(PlotlyDirec):
    required_arguments = 0
    optional_arguments = 0
    file_argument_whitespace = False
    has_content = True
    external = ["width", "height", "defaultTrace", "start", "end", "step", "values", "prefix", "labels"]

    script_template = '''
                     <div id="{{{{data.getAAId()}}}}-{uid}" style="width:100%; height:100%; min-height:500px"></div>
                     <script>
                     $(document).ready(function() {{
                       url = "/directives/updates/{uid}/{{{{data.getFile()}}}}/{{{{data.getAAId()}}}}?context=animation"
                       var load = [88,12]
                       Plotly.newPlot('{{{{data.getAAId()}}}}-{uid}',[{{values: load, text:'Loading', textposition:'inside', hole: 0.5, labels: 
                       ['Loaded','Remaining'], type: 'pie'}}],{{showlegend:false,
                       annotations: [{{font: {{size: 20}},showarrow: false, text: `${{load[0]}}%`,x: 0.5,y: 0.5}}] }},{{ }});
                       update_now(url, "{{{{data.getAAId()}}}}-{uid}", 1000, function(config) {{
                         var xx = JSON.parse(config)
                         Plotly.react('{{{{data.getAAId()}}}}-{uid}',xx);                     
                       }})
                     }})
                     </script>
                     '''

    @staticmethod
    def dict_pop(frame: dict, ref: dict):
        r = {}
        for k, v in frame.items():
            if k in ref:
                if isinstance(v, dict) and k in ref and isinstance(ref[k], dict):
                    p = PlotlyAnimation.dict_pop(v, ref[k])
                    if len(p):
                        r[k] = p
                elif v != ref[k]:
                    r[k] = v
            else:
                r[k] = v
        return r

    @staticmethod
    def post_process(text, data, file):
        a = json.loads(text)
        r, v = VnVForDirective.extract_range(a["range"], data)

        if len(r) == 0:
            return plotly_post_process(a["text"], data, file)

        pcontent = VnVForDirective.substitute(r, v, a["text"])

        if len(pcontent) == 1:
            return json.dumps(plotly_post_process_raw(pcontent[0],data,file))

        try:
            labels = VnVForDirective.render_var(a["range"]["labels"],data, json.loads)
        except:
            labels = []

        def get_label(n):
            return str(labels[n]) if n < len(labels) else str(r[n])


        processedContent = { get_label(n) : plotly_post_process_raw({"options" : json.loads(i), "content" : "{}"}, data, file, PlotlyAnimation.external) for n, i in

                            enumerate(pcontent)}

        first_pass = processedContent[get_label(0)]
        first_pass["layout"]["updatemenus"] = updatemenus
        first_pass["layout"]["sliders"] = getSlider(processedContent,
                                         prefix=VnVForDirective.render_var(a["range"]["prefix"], data))

        frames = getFrames(processedContent)
        first_pass["frames"] = [frames[0]] + [PlotlyAnimation.dict_pop(frame, first_pass) for frame in frames[1:]]
        return json.dumps(first_pass)

    # def run(self):
    #    uid = self.updateRegistration()
    #   target, target_id = get_target_node(self)
    #    block = VnVChartNode(uid=uid, html=self.getHtml(target_id,uid))
    #   return [target, block]

    def register(self):
        return self.getContent()

    def getRawContent(self):
        if "values" in self.options:
            return json.dumps({"text": json.dumps(self.options),
                               "range": {"prefix": self.options.get("prefix", "Index"),
                                         "labels": self.options.get("labels"),
                                         "values": self.options["values"]}})
        else:
            return json.dumps({"text": json.dumps(self.options), "range": {
                "start": self.options.get("start", "0"),
                "end": self.options.get("end", "0"),
                "step": self.options.get("step", "1"),
                "prefix": self.options.get("prefix", "Index"),
                "labels" : self.options.get("labels")
            }})


def setup(sapp):
    sapp.add_node(VnVAnimatedNode, **VnVAnimatedNode.NODE_VISITORS)
    sapp.add_directive("vnv-animation", PlotlyAnimation)
