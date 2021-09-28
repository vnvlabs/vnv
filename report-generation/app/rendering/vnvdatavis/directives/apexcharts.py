import json
import re

from sphinx.directives import optional_int
from sphinx.util.docutils import SphinxDirective

from app.rendering.vnvdatavis.directives.general import JsonChartDirective, VnVChartNode, ApexChartDirective
from app.rendering.vnvdatavis.directives.jmes import jmes_jinja_query, get_target_node, jmes_jinja_query_json


def jmes_expression(x):
    return x


class ApexLineChartDirective(ApexChartDirective):
    required_arguments = 0
    optional_arguments = 0
    file_argument_whitespace = True
    has_content = False
    options_spec = {
        'data': jmes_jinja_query_json,
        'catagories': jmes_jinja_query_json,
        'title': str,
        'zoom': bool,
        'series': str
    }

    def getContent(self):
        return f'''
        {{
            "series": [{{
                "name": "{self.options.get('series',"Series")}",
                "data": {self.options.get('data','[]')}
            }}],
            "chart": {{
                "height": 350,
                "type": 'line',
                "zoom": {{
                    "enabled": {self.options.get('zoom',False)}
                }}
            }},
            "dataLabels": {{
                "enabled": False
            }},
            "stroke": {{
                "curve": 'straight'
            }},
            "title": {{
                "text": '{self.options.get('title','Line Chart')}',
                "align": 'left'
            }},
            "grid": {{
                "row": {{
                    "colors": ['#f3f3f3', 'transparent'],
                    "opacity": 0.5

                }},
            }},
            "xaxis": {{
                "categories": {self.options.get('catagories',"[]")},
            }}
        }}
        '''

def setup(sapp):
    sapp.add_directive("vnv-apex-line", ApexLineChartDirective)
