

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

def loadYaml(filename, injectionPoint):
    if filename not in yamlFilesParsed: 
        with open(filename, 'r') as stream:
            try:
                injectionPointYamlDict.update(yaml.load(stream))
                yamlFilesParsed.append(filename)
            except yaml.YAMLError as exc:
                print(exc)
    print(injectionPointYamlDict)
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
    return getLinkedListItem(ida,"TESTS") + " " + getUL(True) if start else getUL(False) + getLI(False)

def getParentListItem(lid, text, start):
    return getLI(True) + getLink(lid, text) + getUL(True) if start else getUL(False) + getLI(False)

def get_content_header(start):
    return "<div class='col-md-9' style=\"font-size:100px;\">" if start else "</div>"

def get_section(divId, header, start ):
    s = "<div class=\"panel panel-default\">"
    s = s + "<div class=\"panel-heading\">"
    s = s + "<h4 class=\"panel-title\" data-toggle=\"collapse\" data-target=\"#" + divId + "collapseOne\">"
    s = s + header + "</h4> </div> <div id=" + divId + " class=\"panel-collapse collapse\"> <div class=\"panel-body\">"
    
    return s if start else "</div></div></div>"


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
                html_index = html_index + getLinkedListItem("TODO","Introduction")
                md = fh_step.read_string("markdown")[0]              
                
                introHtml = loadYaml("sampleIPFile.yaml", "introduction")
                
                html_content = html_content + markdown.markdown(introHtml)
                
            elif type_ == "StartIP" :
                ida = fh_step.read_string("identifier")[0]
                ss = fh_step.read("stage")[0]
                    
                if (ss == 0)  : 
                    #This indicates we have a new injection point, so do a sublist
                    html_index = html_index + getParentListItem("TODO",ida, True) ## This is a new Injection Point header
                                
                # Add a sub heading for this stage 
                html_index = html_index + getParentListItem("TODO", "Stage " + str(ss) , True)
                                
                #d = loadYaml(fh_step.read_string("markdown")[0], fh_step.read_string("identifier")[0]) 
                
            elif type_ == "StartTest" :
                ida = fh_step.read_string("identifier")[0]
                ss = fh_step.read("stage")[0]
                # Add the test to the index 
                html_index += getLinkedListItem("TODO",ida) 
            
            elif type_ == "EndIP" :
                ida = fh_step.read_string("identifier")[0]
                ss = fh_step.read("stage")
                
                #End the tests for this injection point stage 
                html_index = html_index + getParentListItem("","",False) 
                
                if ( ss > 9000 ): 
                    #End the injection point heading for this injection point index 
                    html_index = html_index + getParentListItem("","",False) 

            elif type_ == "conclusion":
                html_index = html_index + getLinkedListItem("TODO","Conclusion")
                introHtml = loadYaml("sampleIPFile.yaml", "conclusion") 
                html_content = html_content + get_section("conclusion","Conclusion",True)
                html_content += markdown.markdown(introHtml)
                html_content += get_section("","",False)
                
    html_index = html_index + get_index_header(False)
    html_content = html_content + get_content_header(False)

    print(html_index)

    print ("\n\n\n\n")
    print(html_content)

