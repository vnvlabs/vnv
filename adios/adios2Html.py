

from mpi4py import MPI
import numpy as np
import adios2
import yaml
from markdown.util import etree

import markdownNS 

comm = MPI.COMM_WORLD
rank = comm.Get_rank()
size = comm.Get_size()

yamlFilesParsed = []  
injectionPointYamlDict = {}

JS_INCLUDES = set()
JS_SCRIPT = []

divId = 0

def getDivID():
    global divId 
    divId = divId + 1
    return "vv_div" + str(divId)

def loadYaml(filename, injectionPoint):
    if filename not in yamlFilesParsed: 
        with open(filename, 'r') as stream:
            try:
                injectionPointYamlDict.update(yaml.load(stream))
                yamlFilesParsed.append(filename)
            except yaml.YAMLError as exc:
                print(exc)
    return injectionPointYamlDict.get(injectionPoint)

def pad(x):
    return " ".join(["\t"] * x )

def get_index_header(start):
    return "<ul id=\"tree3\" style='height: 100vh;overflow-y: auto; overflow-x: scroll;'>" if start else "</ul>"

def getUL(start):
    return "<ul>" if start else "</ul>"
def getLI(start):
    return "<li>" if start else "</li>"
def getLink(linkId, text):
    return "<a href=\"#" + linkId + "\">" + text + "</a>"
def getListItem(text):
    return getLI(True) + text + getLI(False)

def getLinkedListItem(lid, text):
    return getListItem(getLink(lid,text))

def getTestsList(ida, start):
    return getLinkedListItem(getDivID(),"TESTS") + " " + getUL(True) if start else getUL(False) + getLI(False)

def getParentListItem(lid, text, start):
    return getLI(True) + getLink(getDivID(), text) + getUL(True) if start else getUL(False) + getLI(False)

def get_section(divId, header, start ):
    s = "<div class=\"panel-group\"> <div class=\"panel panel-default \">"
    s = s + "<div class=\"panel-heading\">"
    s = s +     "<h4 class=\"panel-title\"><a class=\"accordion-toggle\" data-parent=\"master-" + divId + "\" data-toggle=\"collapse\" href=\"#" + divId + "\" >"
    s = s + header + "</a></h4> </div> <div id=" + divId + " class=\"panel-collapse collapse in \"> <div class=\"panel-body\">"
    
    return s if start else "</div></div></div></div>"


def getSectionsFromYaml(divId, yamlDict, close=True) :
    print(yamlDict)
    header = yamlDict.get("title","Untitled")
    content = yamlDict.get("content","")
    sections = yamlDict.get("sections",[])

    indexs = ""
    if len(sections) > 0 or not close :
        indexs = getParentListItem(divId, header, True)
    else :
        indexs = getLinkedListItem(divId, header) 

    s = get_section(divId, header, True)

    text, requiredJs, customJs = markdownNS.getMarkdown(content, {})


    ## Add the div information. 
    s = s + text 
    JS_INCLUDES.update(requiredJs)
    for i in customJs:
        JS_SCRIPT.append(i)

    for n,section in enumerate(sections):
        
        sec,ind = getSectionsFromYaml(divId + "_" + str(n) , section)
        s = s + sec;
        indexs = indexs + ind
    
    if close:
        s = s + get_section("","",False)
    
        if len(sections) > 0:
            indexs += getParentListItem("","",False)
 
    return s, indexs 

def getInjectionPointContent(stage, ida, yamlDict, start):
    
    if start:
        return getSectionsFromYaml(getDivID(), yamlDict, False) 
    else:
        return get_section("","",False), getParentListItem("","",False)
        
def getTestsHeading(start):
      return getParentListItem(getDivID(), "TESTS", start) 

def getChildrenHeading(start):
      return getParentListItem(getDivID(), "Children", start)

def getInjectionPointStageContent(stage,ida,yamlDict,start):
    if start:
        stageDict = yamlDict.get("stage_" + str(stage))
        return getSectionsFromYaml(getDivID() , stageDict, False) 
    else:
        return get_section("","",False) , getParentListItem("","",False)
    
def getTestContent(yamlDict):
    print("\n\n\n\n" , yamlDict)
    return getSectionsFromYaml(getDivID(), yamlDict)
    
class IP: 
    
    def __init__(self):
        self.index = ""
        self.content = ""
        self.testIndex = ""
        self.contentIndex = ""
        self.tests = False
        self.children = False
        self.closed = True

    def open(self, fh_step):
        ida = fh_step.read_string("identifier")[0]
        ss = fh_step.read("stage")[0]        
        introHtml = loadYaml("sampleIPFile.yaml", ida) 
        self.content,self.index = getInjectionPointContent(ss, ida, introHtml,True)     
        self.closed = False 
        self.stage = ss

    def setStage(self, s) :
        self.stage = s

    def addTest(self, fh_step): 
        
        if self.stage < 9000:
            return 
        if not self.tests:
            self.test_index = getTestsHeading(True)
            self.test_content = get_section(getDivID(), "Tests",True)
            self.tests = True

        ida = fh_step.read_string("identifier")[0]        
        introHtml = loadYaml("sampleIPFile.yaml", ida) 
        
        sec,inds = getTestContent(introHtml)
        self.test_content = self.test_content + sec
        self.test_index = self.test_index + inds

    def addChild(self, injectionPoint):
        
        if not injectionPoint.closed :
            print("Wooops")

        if not self.children:
            self.children_index = getChildrenHeading(True)
            self.children_content = get_section(getDivID(),"Children",True)
            self.children = True

        self.children_content = self.children_content + injectionPoint.content 
        self.children_index = self.children_index + injectionPoint.index

    def close(self):

        if self.stage < 9000:
            return False

        if (self.tests):
            self.test_content = self.test_content + get_section("","",False)
            self.test_index = self.test_index + getTestsHeading(False)
            self.content = self.content + self.test_content;
            self.index = self.index + self.test_index ; 
        if (self.children):
            self.children_index = self.children_index + getChildrenHeading(False)
            self.children_content = self.children_content + get_section("","",False)
            self.content = self.content + self.children_content;
            self.index = self.index + self.children_index ; 
        
        sec, inds = getInjectionPointContent("","",{},False)
        self.index = self.index + inds
        self.content = self.content + sec 
        self.closed = True
        return True


if( rank == 0 ):
    # with-as will call adios2.close on fh at the end
    # if only one rank is active pass MPI.COMM_SELF
    

    html_index = get_index_header(True)
    html_content = "<div class=\"panel-group\">"


    with adios2.open("./outfile_defualt.bp", "r", MPI.COMM_SELF) as fh:
        
        cIp = [] 

        for fh_step in fh:
            step = fh_step.current_step()
            ss = fh_step.read("stage")[0]
           
            type_ = fh_step.read_string("type")[0]
            if type_ == "introduction" :
                introHtml = loadYaml("sampleIPFile.yaml", "introduction") 
                sec,inds = getSectionsFromYaml("Introduction", introHtml)
                html_index = html_index + inds 
                html_content = html_content + sec 
            
            elif type_ == "StartIP" :
                
                ida = fh_step.read_string("identifier")[0]        
                
                ss = fh_step.read("stage")[0]
                if ss == 0 or ss == -1 : 
                    cIp.append(IP())
                    cIp[-1].open(fh_step)
                else:
                    cIp[-1].setStage(ss)
                
                print("starting injection Point", ida, ss, len(cIp) ) 

            elif type_ == "StartTest" :
                cIp[-1].addTest(fh_step)

            elif type_ == "EndIP" :
                ida = fh_step.read_string("identifier")[0]        
                ss = fh_step.read("stage")[0]                
                print("ending injection Point", ss, ida, len(cIp) ) 
                if ( cIp[-1].close() ) :

                    if ( len(cIp) == 1 ) :
                        html_index = html_index + cIp[-1].index
                        html_content = html_content + cIp[-1].content
                        cIp = []
                    else:
                        cIp[-2].addChild(cIp[-1]) 
                        del cIp[-1]
    
            elif type_ == "conclusion":
                introHtml = loadYaml("sampleIPFile.yaml", "conclusion") 
                sec,inds = getSectionsFromYaml("Conclusion", introHtml)
                html_index = html_index + inds 
                html_content = html_content + sec 
                
    html_index = html_index + get_index_header(False)
    html_content = html_content + "</div>"


    html_scripts = ""
    for i in JS_INCLUDES :
        html_scripts = html_scripts + etree.tostring(i, method='html').decode()

    for i in JS_SCRIPT :
        html_scripts = html_scripts + etree.tostring(i, method = 'html' ).decode()

    html_styles = ""
    html_title = "<title> This is the title </title>"
    filedata = ""
    with open('web/index.template', 'r') as file :
         filedata = file.read()
         
         filedata = filedata.replace("#####TITLE#####", html_title)
         filedata = filedata.replace("#####STYLES#####", html_styles )
         filedata = filedata.replace("#####TREE#####", html_index )
         filedata = filedata.replace("#####CONTENT#####", html_content )
         filedata = filedata.replace("#####JS_INCLUDES#####", html_scripts )
     # Write the file out again
    with open('web/index.html', 'w') as file:
        file.write(filedata)    
