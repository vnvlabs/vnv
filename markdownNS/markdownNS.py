import json
import markdown 
from markdown.extensions import Extension
from markdown.inlinepatterns import SimpleTagPattern
from markdown.inlinepatterns import InlineProcessor
from markdown.util import etree

# Simple tag pattern matches the 3rd group, with the first being the entire string 
# It replaces the 3rd group with <tag> (3rd) </tag> in output where tag is set in the constructro
DEL_RE = r'(--)(.*?)--'

MDNS_RE=r'\[VV::(.*?)=({.*?})\]'

GLOBAL_PARSER_DICT = {}
REQUIRED_JS_SCRIPTS = set()
CUSTOM_JS_SCRIPTS = []

def getRequiredAndCustom():

    l = []
    for i in REQUIRED_JS_SCRIPTS:
        l.append(getScriptTag("", i))
    return l,CUSTOM_JS_SCRIPTS



def getScriptTag(text, src=None):
    e = etree.Element("script")
    if  src is not None: 
        e.set("src",src)
    e.text = text
    return e

def registerScript(text, src=None):
    GLOBAL_JS_SCRIPTS.add(getScriptTag(text,src))
def registerParser(text, k):
    GLOBAL_PARSER_DICT[text] = k


#### Plot 3d

DIVID = 0
def getDIVID() :
    global DIVID
    DIVID = DIVID + 1
    return "MD-NS-" + str(DIVID)

class VVParser :
    options = {}
    adios = {}
    def __init__(self,options, adios ):
        self.options = options
        self.adios = adios
        
    def parse(self) :
        print "Base Class Does not Override the parse() method in VVParser"
        raise NotImplementedError

    def requires(self):
        return []
    
class plotJs(VVParser):

    def parse(self) :
        print self.adios
        
        divId = getDIVID()

        # Build the element 
        et = etree.Element("div")
        et.set("class","container")
        etree.SubElement(et, 'canvas', id=divId)
        
        text = "renderExampleLineChart(%s)".format(divId)
        scriptTag = getScriptTag(text)
        
        return et, scriptTag
        # Add the script 
    
    # Get the javascript files that are required to run this thing
    def requires(self):
        return ["js/charts.js","js/jquery.min.js", "js/mycharts.js"]


registerParser("plotJs",plotJs)

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
        
        e,s = par.parse()
        
        REQUIRED_JS_SCRIPTS.update(par.requires())
        CUSTOM_JS_SCRIPTS.append(s)

        return e, m.start(0), m.end(0) 

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


def getMarkdown(string, adios):
    #Reset the required and custom scripts for this js. 
    REQUIRED_JS_SCRIPTS = set()
    CUSTOM_JS_SCRIPTS = []
    md = markdown.markdown(string, extensions=[MarkdownNS(adios)])
    rjs, cjs = getRequiredAndCustom()
    return md,rjs,cjs
