

from mpi4py import MPI
import numpy as np
import adios2
import yaml
import markdown

comm = MPI.COMM_WORLD
rank = comm.Get_rank()
size = comm.Get_size()

yamlFilesParsed = []  
injectionPointYamlDict = {}

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

def get_content_header(start):
    return "<div class='col-md-9' style=\"font-size:13px;\">" if start else "</div>"

def get_section(divId, header, start ):
    s = "<div class=\"panel panel-default\">"
    s = s + "<div class=\"panel-heading\">"
    s = s + "<h4 class=\"panel-title\" data-toggle=\"collapse\" data-target=\"#" + divId + "\">"
    s = s + header + "</h4> </div> <div id=" + divId + " class=\"panel-collapse collapse\"> <div class=\"panel-body\">"
    
    return s if start else "</div></div></div>"


def getSectionsFromYaml(divId, yamlDict, close=True) :
    header = yamlDict.get("title","Untitled")
    content = yamlDict.get("content","")
    sections = yamlDict.get("sections",[])

    indexs = ""
    if len(sections) > 0 or not close :
        indexs = getParentListItem(divId, header, True)
    else :
        indexs = getLinkedListItem(divId, header) 

    s = get_section(divId, header, True)

    s = s + markdown.markdown(content) 
    for n,section in enumerate(sections):
        print("\n\n\n\n\n", section, "\n\n\n\n")        
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
        
def getInjectionPointStageContent(stage,ida,yamlDict,start):
    if start:
        stageDict = yamlDict.get("stage_" + str(stage))
        return getSectionsFromYaml(getDivID() , stageDict, False) 
    else:
        return get_section("","",False) , getParentListItem("","",False)
    
def getTestContent(stage, ida, yamlDict):
    header = yamlDict.get("title","Untitled")
    content = yamlDict.get("content","")
    stageId = "stage_" + str(stage)
    stageInfo = yamlDict.get(stageId,None)
    return getSectionsFromYaml(getDivID(), stageInfo)

    


if( rank == 0 ):
    # with-as will call adios2.close on fh at the end
    # if only one rank is active pass MPI.COMM_SELF
    

    html_index = get_index_header(True)
    html_content = get_content_header(True) 

    with adios2.open("./outfile_defualt.bp", "r", MPI.COMM_SELF) as fh:
        
        current_stage = [""]
        current_id = [100000]
        
        padding  = 0
        for fh_step in fh:
            step = fh_step.current_step()
            ss = fh_step.read("stage")
           
            type_ = fh_step.read_string("type")[0]
            if type_ == "introduction" :
                
                introHtml = loadYaml("sampleIPFile.yaml", "introduction") 
                sec,inds = getSectionsFromYaml("Introduction", introHtml)
                html_index = html_index + inds 
                html_content = html_content + sec 
            
            elif type_ == "StartIP" :
                ida = fh_step.read_string("identifier")[0]
                ss = fh_step.read("stage")[0]
                
                introHtml = loadYaml("sampleIPFile.yaml", ida) 
                print(ida,introHtml)
                
                ### This indicates that this is the start of an injection point
                if ss == 0 or ss == -1 : 
                    sec, inds = getInjectionPointContent(ss, ida, introHtml,True) 
                    html_index = html_index + inds 
                    html_content = html_content + sec 
                
                sec,inds = getInjectionPointStageContent(ss,ida,introHtml,True)
                html_index = html_index + inds 
                html_content = html_content + sec 

            elif type_ == "StartTest" :
                ida = fh_step.read_string("identifier")[0]
                ss = fh_step.read("stage")[0]
                introHtml = loadYaml("sampleIPFile.yaml", ida) 
                
                print(ida) 
                
                # Add the test to the index 
                sec,inds = getTestContent(ss,ida,introHtml)
                html_index = html_index + inds 
                html_content = html_content + sec 
            
            elif type_ == "EndIP" :
                ida = fh_step.read_string("identifier")[0]
                ss = fh_step.read("stage")
                introHtml = loadYaml("sampleIPFile.yaml", ida) 
                
                #End this stage for the injection point 
                sec,inds = getInjectionPointStageContent(ss,ida,introHtml,False)
                html_index = html_index + inds 
                html_content = html_content + sec 
               

                #End the injection Point because this is the last stage. 
                if ( ss > 9000 or ss == -1 ): 
                    sec, inds = getInjectionPointContent("", "", introHtml,False) 
                    html_index = html_index + inds 
                    html_content = html_content + sec 

            elif type_ == "conclusion":
                introHtml = loadYaml("sampleIPFile.yaml", "conclusion") 
                sec,inds = getSectionsFromYaml("Conclusion", introHtml)
                html_index = html_index + inds 
                html_content = html_content + sec 
                
    html_index = html_index + get_index_header(False)
    html_content = html_content + get_content_header(False)

    print(html_index)

    print ("\n\n\n\n")
    print(html_content)

