import json
from markdown.extensions import Extension
from markdown.inlinepatterns import SimpleTagPattern
from markdown.inlinepatterns import InlineProcessor
from markdown.util import etree

# Simple tag pattern matches the 3rd group, with the first being the entire string 
# It replaces the 3rd group with <tag> (3rd) </tag> in output where tag is set in the constructro
DEL_RE = r'(--)(.*?)--'

MDNS_RE=r'\[VV::(.*?)=({.*?})\]'

GLOBAL_PARSER_DICT = {}


#### Plot 3d

class VVParser :
    options = {}
    adios = {}
    def __init__(self,options, adios ):
        self.options = options
        self.adios = adios
        
    def parse(self) :
        print "Base Class Does not Override the parse() method in VVParser"
        raise NotImplementedError

class plotJs(VVParser):

    def parse(self) :
        print self.adios
        et = etree.Element("div")
        et.set("class","container")
        etree.SubElement(et, 'canvas', id='mychart')
        s = etree.SubElement(et, 'script')
        s.text = "src=\"https://cdnjs.cloudflare.com/ajax/libs/Chart.js/2.7.2/Chart.js\""
        ss = etree.SubElement(et,'script')
        ss.text = " \
var data = [20000, 14000, 12000, 15000, 18000, 19000, 22000]; \
var labels =  [\"sunday\", \"monday\",\"tuesday\", \"wednesday\", \"thursday\", \"friday\", \"saturday\"];\
var ctx = document.getElementById(\"myChart\").getContext('2d');\
var myChart = new Chart(ctx, {\ type: 'line',\
                 data: {\
                     labels: labels,\
                     datasets: [{\
                         label: 'This week',\
                         data: data,\
                     }]\
                  },\
              });"\
              
        return et

GLOBAL_PARSER_DICT["plotJS"] = plotJs


class plot3D(VVParser):
    def parse(self) :
        et = etree.Element("plot3d")
        et.text = "Hello"
        return et

GLOBAL_PARSER_DICT["plot3D"] = plot3D

class matrixStatistics(VVParser):
    def parse(self) :
        et = etree.Element("matrixStatistics")
        et.text = "Hello"
        return et

GLOBAL_PARSER_DICT["matrixStatistics"] = matrixStatistics


class spy(VVParser) :
    def parse(self) :
        et = etree.Element("spy")
        et.text = "Hello"
        return et

GLOBAL_PARSER_DICT["spy"] = spy



def getVVParser(name, options, adios) :
    klass = GLOBAL_PARSER_DICT.get(name) 
    if klass is None:
        return None
    else :
        return klass(options,adios)

class VVTestPattern(InlineProcessor):
    MDNS_RE=r'\[VV::(.*?)=({.*?})\]'
    
    adios = None

    def __init__(self, adios):
        super(VVTestPattern,self).__init__(MDNS_RE)
        self.adios = adios
    
    def handleMatch(self,m, data ):        
        optionsDict = json.loads(m.group(2))
        par = getVVParser(m.group(1), optionsDict, self.adios )
        if par is None:
            return None,None,None # Not one of mine (odd that another one might exist though?)
        return par.parse(), m.start(0), m.end(0) 

class MarkdownNS(Extension):
    
    adios = None

    def __init__(self, adios, *args, **kwargs):
        self.adios = adios 
        super(MarkdownNS,self).__init__(*args,**kwargs)
    
    def extendMarkdown(self,md,md_globals):
       pattern = VVTestPattern(self.adios)
       md.inlinePatterns['vv'] = pattern

def makeExtension(*args, **kwargs):
    return MarkdownNS(*args, **kwargs)
