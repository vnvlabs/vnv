
from app.rendering.vnvdatavis.directives.charts import ApexChartDirective
from app.rendering.vnvdatavis.directives.jmes import jmes_jinja_percentage

class ApexGaugeDirective(ApexChartDirective):
    required_arguments = 0
    optional_arguments = 0
    file_argument_whitespace = True
    has_content = False
    options_spec = {
        'min': str,
        'max': str,
        'curr': str,
        'title': str
    }

    def labels(self):
        return self.options.get("title","VnV Gauge Chart")
    def g(self):
        mi = self.options.get("min", "`0`")
        cu = self.options.get("curr", "`50`")
        ma = self.options.get("max", "`100`");
        return jmes_jinja_percentage(cu,mi,ma)
    def getContent(self):
        return f'''
         {{
          series: [{self.g()}],
          chart: {{
            height: 350,
            type: 'radialBar',
          }},
          plotOptions: {{
            radialBar: {{
              hollow: {{
                size: '70%',
              }}
            }},
          }},
          labels: ["{self.labels()}"],
        }}
        '''

def setup(sapp):
    sapp.add_directive("vnv-gauge", ApexGaugeDirective)
