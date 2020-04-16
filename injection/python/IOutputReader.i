

%module VnVReader

//%include <std_shared_ptr.i>
//%shared_ptr(VnV::Reader::IRootNode)
//%shared_ptr(VnV::Reader::ITreeGenerator)
//%shared_ptr(VnV::Reader::IReader)
 %{
 #include "interfaces/IOutputReader.h"
%}

%include <exception.i>

%exception {
    try {
        $action
    }
    catch (VnV::VnVExceptionBase e) {
        SWIG_exception(SWIG_RuntimeError, e.what());
    } catch (...) {
        SWIG_exception(SWIG_RuntimeError, "Unknown exception");
    }
}

%include "std_string.i"
%include "std_vector.i"
%include "std_map.i"
%include "interfaces/IOutputReader.h"

namespace std {
    %template(charVector) vector<char*>;
}


%pythoncode %{
class classIterator : 
         def __init__(self, obj):
           self.obj = obj
           self.iterData = []
           self.iterCounter = 0
           for name in dir(obj):
             if name.startswith("get"):
               self.iterData.append(name[3:]) 
        
         def __next__(self):
           if (self.iterCounter) < len(self.iterData):
              key = self.iterData[self.iterCounter]
              self.iterCounter += 1 
              return [key,self.obj[key]]
           else:
              raise StopIteration

class mapclassIterator:
    def __init__(self,obj):
         self.obj = obj
         self.iterCounter = 0
         self.iterData = self.obj.keys();

    def __next__(self):
         if (self.iterCounter < len(self.iterData)):
           key = self.iterData[self.iterCounter]
           self.iterCounter += 1
           return [key,self.obj[key]]
         else:
           raise StopIteration

class listclassIterator : 
         def __init__(self, obj):
           self.obj = obj
           self.iterCounter = 0
        
         def __next__(self):
           if (self.iterCounter) < len(self.obj):
              return self.obj[self.iterCounter]
           else:
              raise StopIteration

dataBaseCastMap = {
    DataBase.DataType_Bool : "AsBoolNode",
    DataBase.DataType_Integer : "AsIntegerNode",
    DataBase.DataType_Float : "AsFloatNode",
    DataBase.DataType_Double : "AsDoubleNode",
    DataBase.DataType_String : "AsStringNode",
    DataBase.DataType_Long : "AsLongNode",
    DataBase.DataType_Array : "AsArrayNode",
    DataBase.DataType_Map : "AsMapNode",
    DataBase.DataType_Log : "AsLogNode",
    DataBase.DataType_Documentation : "AsDocumentationNode",
    DataBase.DataType_InjectionPoint : "AsInjectionPointNode",
    DataBase.DataType_Info : "AsInfoNode",
    DataBase.DataType_Test : "AsTestNode",
    DataBase.DataType_UnitTest : "AsUnitTestNode",
}

def Initialize(args, config):
   VnVInit(charVector(args),config)
def Finalize():
   VnVFinalize()

def castDataBase(obj) :
    return getattr(obj, "get" + dataBaseCastMap[obj.getType()])()

%}

%define PY_GETATTR(Typename)
%extend Typename {
  %pythoncode %{

      def __str__(self):
          return self.toString();
      
      def __getitem__(self,key):
         res = getattr(self,"get"+key)
         if res is not None:
                return res()
         raise KeyError("not a valid key")

      def __getType__(self):
         return "dict"
      
      def __len__(self):
         count = 0
         for name in dir(self):
                if name.startswith('get'):
                     count+=1
         return count
      
      def __iter__(self):
        return classIterator(self)
      
      def values(self):
         res = []
         for name in dir(self):
           if name.startswith('get'):
              res.append(getattr(self,name)())
         return res
      
      def keys(self):
         res = []
         for name in dir(self):
            if name.startswith('get'):
               res.append(name[3:])
         return res
      
      def __contains__(self,item):
         return hasattr(self,"get" + item)      
      
   %}
}
%enddef

// Make these have iterators and getattr:

PY_GETATTR(VnV::Reader::IRootNode)
PY_GETATTR(VnV::Reader::IUnitTestNode)
PY_GETATTR(VnV::Reader::IDocumentationNode)
PY_GETATTR(VnV::Reader::ILogNode)
PY_GETATTR(VnV::Reader::IInjectionPointNode)
PY_GETATTR(VnV::Reader::ITestNode)
PY_GETATTR(VnV::Reader::IInfoNode)
PY_GETATTR(VnV::Reader::IDoubleNode)
PY_GETATTR(VnV::Reader::IIntegerNode)
PY_GETATTR(VnV::Reader::IFloatNode)
PY_GETATTR(VnV::Reader::IStringNode)
PY_GETATTR(VnV::Reader::ILongNode)
PY_GETATTR(VnV::Reader::IBoolNode)


%define PY_GETATTRLIST(Typename)
%extend Typename {
  %pythoncode %{
      def __getitem__(self,key):
        if isinstance(key,int) and key < self.size() :
           return castDataBase(self.get(key))
        raise KeyError("not a valid key")
      
      def __len__(self):
        return self.size(); 
      
      def __iter__(self):
        return listclassIterator(self)
     
      def __getType__(self):
        return "list"      
      
      def __str__(self):
        return self.toString();
      
   %}
}
%enddef

PY_GETATTRLIST(VnV::Reader::IArrayNode)

%define PY_GETATTRMAP(Typename)
%extend Typename {
    %pythoncode %{
        def __getitem__(self,key):
            if isinstance(key,string) and self.contains(key):
                return castDataBase(self.get(key))
            raise KeyError("not a valid key")

        def __len__(self):
            return self.size();

        def __iter__(self):
            return mapclassIterator(self)

        def __getType__(self):
            return "dict"

        def values(self):
            res = []
            for name in self.keys():
                res.append(castDataBase(self.get(name)))
            return res

        def keys(self):
             res = []
             for name in self.fetchkeys():
                res.append(name)
             return res

        def __contains__(self,item):
             return self.contains(item)

        def __str__(self):
            return self.toString();
    %}
}
%enddef
PY_GETATTRMAP(VnV::Reader::IMapNode)


