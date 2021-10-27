import json

from sphinx.errors import ExtensionError

from app.rendering.vnvdatavis.directives.charts import JsonChartDirective, VnVChartNode, ApexChartDirective
from app.rendering.vnvdatavis.directives.jmes import jmes_jinja_query, get_target_node, jmes_jinja_query_json, \
    jmes_check, jmes_jinja_zip


def jmes_expression(x):
    return x


def lineareabar(x):
    if x in ['line', 'area', 'bar', 'column']:
        return x
    raise ExtensionError("Invalid type")


class ApexLineChartDirective(ApexChartDirective):
    required_arguments = 0
    optional_arguments = 0
    file_argument_whitespace = True
    has_content = False
    option_spec = {
        'yaxis': str,
        'xaxis': str,
        'title': str,
        'label': str,
        "type": lineareabar
    }

    def getContent(self):
        return f'''
        {{
            "series": [{{
                "name": "{self.options.get('label', "Series")}",
                "type": "{self.options.get("type", "area")}",
                "data": {jmes_jinja_query_json(self.options.get('yaxis', '`[1,1,2,3,5]`'))}
            }}],
            "chart": {{
                "width" : "100%",
                "height" : "100%",
                "type":"{self.options.get("type", "area")}",
                "zoom": {{
                    "enabled": true 
                }}
            }},
            "dataLabels": {{
                "enabled": false
            }},
            "stroke": {{
                "curve": "smooth"
            }},
            "plotOptions": {{
                "bar": {{
                    "columnWidth": "50%"
                }}
            }},
            "title": {{
                "text": "{self.options.get('title', 'Line Chart')}",
                "align": "left"
            }},
            "grid": {{
                "row": {{
                    "colors": ["#f3f3f3", "transparent"],
                    "opacity": 0.5
                }}
            }},
            "xaxis": {{
                "categories": {jmes_jinja_query_json(self.options.get('xaxis', '`["a","b","c","d","e"]`'))}
            }}
        }}
        '''


def json_jmes_array(a):
    aa = json.loads(a)
    if not isinstance(aa, list):
        raise ExtensionError("Not a json array")
    for i in aa:
        if not isinstance(i, str) or not jmes_check(i):
            raise ExtensionError("Invalid Jmes String")
    return aa


def json_array(a):
    aa = json.loads(a)
    if not isinstance(aa, list):
        raise ExtensionError("Not a json array")
    for i in aa:
        if not isinstance(i, str):
            raise ExtensionError("Invalid Jmes String")
    return aa


def jmes_check_local(text):
    if jmes_check(text):
        return text
    raise ExtensionError("Invalid Jmes")


class ApexMultiLineChartDirective(ApexChartDirective):
    required_arguments = 0
    optional_arguments = 0
    file_argument_whitespace = True
    has_content = False
    option_spec = {
        'yaxis': json_jmes_array,
        'xaxis': jmes_check_local,
        'type': json_array,
        'labels': json_array,
        'title': str,
    }

    def getSeries(self):
        series = "["
        yax = self.options.get("yaxis", [])
        labels = self.options.get("labels", [f'Series {i}' for i in range(0, len(yax))])
        type = self.options.get("type", ['line' for i in range(0, len(yax))])
        for n, y in enumerate(yax):
            series += f'{{"name" : "{labels[n]}" , "data" : {jmes_jinja_query_json(y)} , "type" : "{type[n]}" }}'
            series += "," if (n + 1 < len(yax)) else "]"
        return series

    def getCatagories(self):
        return jmes_jinja_query_json(self.options.get("xaxis", '`[]`'))

    def getContent(self):
        return f'''
        {{
            "series": {self.getSeries()},
            "chart": {{
               "width" : "100%",
               "height" : "100%",
               "type": "line",
               "zoom": {{
                    "enabled": true 
                }}
            }},
            "dataLabels": {{
                "enabled": false
            }},
            "stroke": {{
                "curve": "smooth"
            }},
            "plotOptions": {{
                "bar": {{
                    "columnWidth": "50%"
                }}
            }},
            "title": {{
                "text": "{self.options.get('title', 'Multi Line Chart')}",
                "align": "left"
            }},
            "xaxis": {{
                "categories": {self.getCatagories()}
            }}
        }}
        '''



class ApexTimeSeriesChartDirective(ApexChartDirective):
    required_arguments = 0
    optional_arguments = 0
    file_argument_whitespace = True
    has_content = False
    option_spec = {
        'times': str,
        'data': str,
        'label': str,
        'title': str,
    }

    def getData(self):
        return jmes_jinja_zip({"x": self.options.get("times", '`[]`'), "y": self.options.get("data", "`[]`")})

    def getContent(self):
        return f'''
        {{
          "series": [
            {{
                "name": "{self.options.get("label", "Series 1")}",
                "data": {self.getData()}
            }}
          ],
          "chart": {{
            "id": "chart2",
            "width" : "100%",
            "height" : "100%",    
            "type": "line",
            "toolbar": {{
                "autoSelected": "pan",
                "show": false
            }}
          }},
          "colors": ["#546E7A"],  
          "stroke": {{
            "width": 3
          }},
          "dataLabels": {{
            "enabled": false
          }},
          "fill": {{
            "opacity": 1
          }},
            "markers": {{
            "size": 0
          }},
          "xaxis": {{
            "type": "datetime"
          }} 
        }}
        '''

class ApexScatterChartDirective(ApexChartDirective):
    required_arguments = 0
    optional_arguments = 0
    file_argument_whitespace = True
    has_content = False
    option_spec = {
        'xdata': str,
        'ydata': str,
        'label': str,
        'title': str,
    }

    def getData(self):
        return jmes_jinja_zip({"x": self.options.get("xdata", '`[]`'), "y": self.options.get("ydata", "`[]`")})
    def getContent(self):
        return f'''
        {{
          "series": [
            {{
                "name": "{self.options.get("label", "Series 1")}",
                "data": {self.getData()}
            }}
          ],
          "chart": {{
                 "width" : "100%",
                "height" : "100%",  
             "type" : "scatter", 
             "zoom": {{
                 "enabled": true,
                 "type": "xy"
             }}
          }},
          "xaxis": {{
              "tickAmount": 10
          }},
          "yaxis": {{
            "tickAmount": 7
          }}
       }}
       '''



def setup(sapp):
    sapp.add_directive("vnv-line", ApexLineChartDirective)
    sapp.add_directive("vnv-multi-line", ApexMultiLineChartDirective)
    sapp.add_directive("vnv-time-series", ApexTimeSeriesChartDirective)
    sapp.add_directive("vnv-scatter", ApexScatterChartDirective)

