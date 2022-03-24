import json
import os.path

import flask
from flask import render_template_string
from sphinx.errors import ExtensionError

from .charts import JsonChartDirective
from collections.abc import MutableMapping

import math
import numpy as np
import json

try:
    plotly_schema
except:
    with open(os.path.join(os.path.dirname(__file__), "plot-schema.json")) as f:
        plotly_schema = json.load(f)["schema"]




def plotly_array(ff, arrayOk=False):
    def f(t):
        if arrayOk:
            try:
                a = json.loads(t)
                if isinstance(a, list):
                    return [ff(aa) for aa in a]
                else:
                    raise ExtensionError("Invalid option not an array")
            except:
                pass
        return ff(t)

    return f


def plotly_enumerated(values, arrayOk=False, **kwargs):
    def enumerate(t):
        for i in values:
            if isinstance(i, str):
                if t == i:
                    return i
            elif isinstance(i, bool):
                try:
                    a = plotly_boolean(False)(t)
                    if i == a:
                        return i
                except:
                    pass
            elif isinstance(i, float):
                try:
                    a = float(t)
                    if a == i:
                        return i
                except:
                    pass
            elif isinstance(i, int):
                try:
                    a = int(t)
                    if a == i:
                        return i
                except:
                    pass
            else:
                pass
        print("UNRECONIZED ENUM -- RETURNING ANYWAY BUT.... TODO make this an error")
        return i

    return plotly_array(enumerate, arrayOk)


def plotly_boolean(arrayOk=False, **kwargs):
    def plotly_boolean_(t):
        if t in ["1", "t", "T", "true", "True"]:
            return True
        elif t in ["0", "f", "F", "false", "False"]:
            return False
        raise ExtensionError(t + " is not valid boolean")

    return plotly_array(plotly_boolean_, arrayOk)


def plotly_number_internal(cls, min=None, max=None, arrayOk=False):
    def plotly_number_(t):
        try:
            a = cls(t)
            if min is not None and a < min:
                raise ExtensionError(t + " must be >= " + min)
            if max is not None and a > max:
                raise ExtensionError(t + " must be <= " + max)
            return a
        except:
            raise ExtensionError(t + " is not a number")

    return plotly_array(plotly_number_, arrayOk)


def plotly_number(min=None, max=None, arrayOk=False,**kwargs):
    return plotly_number_internal(float, min, max, arrayOk)


def plotly_integer(min=None, max=None, arrayOk=False, **kwargs):
    return plotly_number_internal(int, min, max, arrayOk)


def plotly_string(noBlank=False, values=None, arrayOk=False,**kwargs):
    def s(t):
        if values is not None and t not in values:
            raise ExtensionError(t + " not in " + values)
        if noBlank and len(t) == 0:
            raise ExtensionError(t + " can not be empty ")
        return t

    return plotly_array(s, arrayOk)


def plotly_color(arrayOk=False,**kwargs):
    def s(t):
        return t  # todo

    return plotly_array(s, arrayOk)


def plotly_colorlist(**kwargs):
    return plotly_color(True)


def plotly_colorscale(**kwargs):
    v = ["Greys", "YlGnBu", "Greens", "YlOrRd", "Bluered", "RdBu", "Reds", "Blues", "Picnic", "Rainbow",
         "Portland", "Jet", "Hot", "Blackbody", "Earth", "Electric", "Viridis", "Cividis"]
    return plotly_enumerated(values=v, arrayOk=False)


def plotly_angle(**kwargs):
    return plotly_number(min=-180, max=180, arrayOk=False)


def plotly_data_array(**kwargs):
    def func(t):
        try:
            a = json.loads(t)
            if isinstance(a, list):
                for i in a:
                    if isinstance(i,list):
                        plotly_data_array()(json.dumps(i))
                    elif isinstance(i, dict) :
                        raise ExtensionError("Not a data array")
                return a
        except:
            pass
        raise ExtensionError("Must be an array")
    return func


def plotly_convert(keys, value, trace, data):
    rendered = render_template_string(value, data=data)

    if trace == "layout":
        s = plotly_schema["layout"]["layoutAttributes"]
    elif trace == "config":
        s = plotly_schema["config"]
    else:
        s = plotly_schema["traces"][trace]["attributes"]

    for i in keys: s = s[i]

    m = f'plotly_{s["valType"]}'
    if m in globals():
        return globals()[m](**s)(rendered)
    return rendered


def plotly_post_process_raw(text, data, file, ext):
    # Extract all the trace definitions -- trace.x = scatter trace.y = line
    # Turn it into an object
    rdata = {}

    textj = json.loads(text) if isinstance(text,str) else text

    options = textj["options"]

    evalContent = textj["content"]
    try:
        extra_data=eval(evalContent,{"np":np,"math":math,"json":json, "numpy":np});
        if isinstance(extra_data,dict):
            options.update(extra_data)
    except:
        pass


    t = "trace."
    traces = {"layout": "layout",
              "config": "config"}
    errors = {}

    for k, v in options.items():
        if k[0:len(t)] == t:
            traces[k[len(t):]] = render_template_string(v,data=data)

    for k, v in options.items():
        a = k.split('.')
        if k in ext or a[0] == "trace":
            pass
        else:
            a = k.split('.')
            if len(a) <= 1:
                raise ExtensionError("invalid arguement " + k)

            elif a[0] not in traces:
                traces[a[0]] = options.get("defaultTrace", "scatter")

            dc = rdata
            for c in a[0:-1]:
                dc = dc.setdefault(c, {})
            try:
                dc[a[-1]] = plotly_convert(a[1:], v, traces[a[0]], data)
            except Exception as e:
                errors[k] = { "value" : v , "error" : str(e)}

    # Raw data is in the correct format, but it is not
    rawdata = {
        "layout": {},
        "config": {"responsive":True},
        "data": [],
        "errors" : errors
    }

    for k, v in rdata.items():
        if k == "layout":
            rawdata["layout"] = v
        elif k == "config":
            rawdata["config"] = v
        else:
            v.setdefault("type", traces.get(k,"scatter"))
            v.setdefault("name", k)
            rawdata["data"].append(v)
    return rawdata

def plotly_post_process(text, data, file):
    return json.dumps(plotly_post_process_raw(text,data,file, PlotlyDirec.external))


class PlotlyOptionsDict(MutableMapping):
    """A dictionary that calls a function when a requested key does not
    exist in the dictionary"""

    def __init__(self):
        self.store = {
            "height": str,
            "width": str,
            "defaultTrace": str
        }

    def __getitem__(self, key):
        if key in self.store:
            return self.store[key]
        return str

    def __setitem__(self, key, value):
        self.store[key] = value

    def __delitem__(self, key):
        del self.store[key]

    def __iter__(self):
        return iter(self.store)

    def __len__(self):
        return len(self.store)


class PlotlyChartDirective(JsonChartDirective):
    script_template = '''
            <div id="{id_}" style="width:100%; height:100%;"></div>
            <script>
            $(document).ready(function() {{
              const obj = {config}
              Plotly.newPlot('{id_}',obj['data'],obj['layout']);

              url = "/directives/updates/{uid}/{{{{data.getFile()}}}}/{{{{data.getAAId()}}}}"
              update_now(url, "{id_}", 1000, function(config) {{
                var xx = JSON.parse(config)
                Plotly.update('{id_}',xx['data'],xx['layout']);
              }})

            }})
            </script>
            '''

class PlotlyDirec(PlotlyChartDirective):
    required_arguments = 0
    optional_arguments = 0
    file_argument_whitespace = False
    has_content = True
    option_spec = PlotlyOptionsDict()
    external = ["width", "height", "defaultTrace"]

    postprocess = plotly_post_process

    ## We tag all the ids with the dataId because we only really generate the html once per test. So, if
    ## you have a test multiple times, we need to have the dataId different so they dont interfere.
    script_template = '''
                 <div id="{{{{data.getAAId()}}}}-{uid}" style="width:100%; height:100%; min-height:450px"></div>
                 <script>
                 $(document).ready(function() {{
                   url = "/directives/updates/{uid}/{{{{data.getFile()}}}}/{{{{data.getAAId()}}}}?context=plotly"
                   var load = [88,12]
                   
                   Plotly.newPlot('{{{{data.getAAId()}}}}-{uid}',[
                       {{values: load, 
                         text:'Loading', 
                         textposition:'inside', 
                         hole: 0.5, 
                         labels: ['Loaded','Remaining'],
                         type: 'pie'}}
                      ],
                      {{showlegend:false,
                        annotations: [
                           {{ 
                              font: {{ size: 20 }},
                              showarrow: false,
                              text: `${{load[0]}}%`,
                              x: 0.5,
                              y: 0.5
                           }}
                        ] 
                       }},
                       {{ }}
                   );
                   
                   update_now(url, "{{{{data.getAAId()}}}}-{uid}", 1000, function(config) {{
                     var xx = JSON.parse(config)
                     Plotly.react('{{{{data.getAAId()}}}}-{uid}',xx);     
                     Plotly.relayout('{{{{data.getAAId()}}}}-{uid}',{{}});                
                   }})
                   
                 }})
                 </script>
                 '''

    def register(self):
        return self.getContent()

    def getRawContent(self):
        a = {"options" : self.options, "content" : "\n".join(self.content)}
        return json.dumps(a)


def setup(sapp):
    sapp.add_directive("vnv-plotly-raw", PlotlyChartDirective)
    sapp.add_directive("vnv-plotly", PlotlyDirec)



'''

.. vnv-plotly::
   :trace.x: ......
   :trace.y: .....
   
   {
        "xd" : [ a for a in range(0,1000) ],
        "yd" : [ sin(a) for a  in range(0,1000) ]
   }

'''