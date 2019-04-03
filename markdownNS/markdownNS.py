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

class VVParser :
    options = {}
    def __init__(self,options):
        self.options = options

    def parse(self) :
        print "Base Class Does not Override the parse() method in VVParser"
        raise NotImplementedError

class plotJs(VVParser):

    def parse(self) :
        et = etree.Element("plotjs")
        et.text = "Hello"
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
    pass
GLOBAL_PARSER_DICT["spy"] = spy



def getVVParser(name, options) :
    klass = GLOBAL_PARSER_DICT.get(name) 
    if klass is None:
        return None
    else :
        return klass(options)

class VVTestPattern(InlineProcessor):
    MDNS_RE=r'\[VV::(.*?)=({.*?})\]'
    
    def __init__(self):
        super(VVTestPattern,self).__init__(MDNS_RE)
    
    def handleMatch(self,m, data ):        
        optionsDict = json.loads(m.group(2))
        par = getVVParser(m.group(1), optionsDict)
        if par is None:
            return None,None,None # Not one of mine (odd that another one might exist though?)
        return par.parse(), m.start(0), m.end(0) 

class MarkdownNS(Extension):
    
    def __init__(self, *args, **kwargs):

        self.config = {
            'adios' : [None, "adios data object"] , 
            'static' : [None, "static data object"]
        }
        super(MarkdownNS,self).__init__(*args,**kwargs)
    
    def extendMarkdown(self,md,md_globals):
       pattern = VVTestPattern()
       md.inlinePatterns['vv'] = pattern

def makeExtension(*args, **kwargs):
    return MarkdownNS(*args, **kwargs)
