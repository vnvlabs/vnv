

%module VnVReader

//%include <std_shared_ptr.i>
//%shared_ptr(VnV::Nodes::IRootNode)
//%shared_ptr(VnV::Nodes::ITreeGenerator)
//%shared_ptr(VnV::Nodes::IReader)
 %{
 #include "python/PythonInterface.h"
%}

%include <exception.i>

//%exception {
//    try {
//        $action
//    }
//    catch (VnV::VnVExceptionBase e) {
//        SWIG_exception(SWIG_RuntimeError, e.what());
//    } catch (std::exception e ) {
//        SWIG_exception(SWIG_RuntimeError, e.what());
//    }
//}

%include "std_string.i"
%include "std_vector.i"
%include "std_map.i"
%include "interfaces/nodes/Nodes.h"
%include "python/PythonInterface.h"

namespace std {
    %template(charVector) vector<string>;
}


%pythoncode %{

import json
import numpy as np

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
           return key
         else:
           raise StopIteration

class listclassIterator :
         def __init__(self, obj):
           self.obj = obj
           self.iterCounter = 0

         def __next__(self):
           if (self.iterCounter) < len(self.obj):
                self.iterCounter += 1
                return self.obj[self.iterCounter - 1]

           else:
              raise StopIteration

class shapeClassIterator:
         def __init__(self, obj):
           self.obj = obj
           self.iterCounter = 0
           self.shape = json.loads(self.obj.getShapeJson())
           if len(self.shape)==0:
               raise TypeError("Cannot iterate a Scalar Shape class")
         

         def __next__(self):

           if (self.iterCounter) < self.shape[-1]:
                self.iterCounter += 1
                return self.obj[self.iterCounter - 1]

           else:
              raise StopIteration


dataBaseCastMap = {
    DataBase.DataType_Bool : "AsBoolNode",
    DataBase.DataType_Integer : "AsIntegerNode",
    DataBase.DataType_Float : "AsFloatNode",
    DataBase.DataType_Double : "AsDoubleNode",
    DataBase.DataType_String : "AsStringNode",
    DataBase.DataType_Long : "AsLongNode",
    DataBase.DataType_Shape : "AsShapeNode",
    DataBase.DataType_Json : "AsJsonNode",
    

    DataBase.DataType_Array : "AsArrayNode",
    DataBase.DataType_Map : "AsMapNode",
    DataBase.DataType_Log : "AsLogNode",
    DataBase.DataType_InjectionPoint : "AsInjectionPointNode",
    DataBase.DataType_Info : "AsInfoNode",
    DataBase.DataType_CommInfo : "AsCommInfoNode",
    DataBase.DataType_Test : "AsTestNode",
    DataBase.DataType_UnitTest : "AsUnitTestNode",
    DataBase.DataType_Data : "AsDataNode",
    DataBase.DataType_Root : "AsRootNode"
}

type2Str = {
    DataBase.DataType_Bool : "Bool",
    DataBase.DataType_Integer : "Integer",
    DataBase.DataType_Float : "Float",
    DataBase.DataType_Double : "DoubleNode",
    DataBase.DataType_String : "StringNode",
    DataBase.DataType_Long : "Long",
    DataBase.DataType_Array : "Array",
    DataBase.DataType_Map : "Map",
    DataBase.DataType_Log : "Log",
    DataBase.DataType_InjectionPoint : "InjectionPoint",
    DataBase.DataType_Info : "Info",
    DataBase.DataType_Info : "CommInfo",
    DataBase.DataType_Test : "Test",
    DataBase.DataType_Shape : "Shape",
    DataBase.DataType_UnitTest : "UnitTest",
    DataBase.DataType_Data : "DataNode",
    DataBase.DataType_Root : "RootNode"
}

vnv_initialized = False
vnv_finalized = False

def Initialize(args, config):
   global vnv_initialized
   if vnv_initialized:
      raise RuntimeError("Cannot initialize twice")

   r = charVector()
   r.push_back(__file__)
   for i in args:

     if isinstance(i,str):
       r.push_back(i)

   if isinstance(config,dict):
      VnVInit_Str(r, json.dumps(config))
   else:
      VnVInit(r,config)
   vnv_initialized = True

def Finalize():
    global vnv_finalized

    if not vnv_finalized and vnv_initialized:
        VnVFinalize()
        vnv_finalized = True

def Read(filename,reader,config):
   if not vnv_initialized:
       raise RuntimeError("Not initialized")
   if vnv_finalized:
       raise RuntimeError("Already finalized")

   if reader is None:
       name, ext = os.path.splitext(filename)
       if ext is None or len(ext)==0:
            raise RuntimeError("Cannot extract reader information from file extension")
       return ReaderWrapper(filename,ext[1:],config)
   return ReaderWrapper(filename,reader,config)


def castDataBase(obj) :
    return getattr(obj, "get" + dataBaseCastMap[obj.getType()])()

%}


%define PY_GETATTR(Typename)
%extend Typename {
  %pythoncode %{

      def __str__(self):
           return str(self.getValue())

      def __getitem__(self,key):
         if key == "metaData" or key == "MetaData":
            return json.loads(self.getMetaData().asJson())

         res = getattr(self,"get"+key)
         if res is not None:
            
            if hasattr(self,"isJson") and self.isJson():
              return json.loads(res())
            return res()

         print("Not a key {} {} ".format(key, self.__class__))
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
         for name in self.keys():
              res.append(self.__getitem__(name))
         return res

      def keys(self):
         res = []
         for name in dir(self):
            if name.startswith('get'):
               res.append(name[3:])
         return res

      def __contains__(self,item):
         res = hasattr(self,"get" + item)
         if not res and hasattr(self,"get", item.capitalize()):
                print("We dont have {i} but we do have {rr}".format(ii=item, rr=item.captialize()))
         return res

   %}
}
%enddef

// Make these have iterators and getattr:

PY_GETATTR(VnV::Nodes::IRootNode)
PY_GETATTR(VnV::Nodes::IUnitTestNode)
PY_GETATTR(VnV::Nodes::IDataNode)
PY_GETATTR(VnV::Nodes::ILogNode)
PY_GETATTR(VnV::Nodes::IInjectionPointNode)
PY_GETATTR(VnV::Nodes::ITestNode)
PY_GETATTR(VnV::Nodes::IInfoNode)
PY_GETATTR(VnV::Nodes::ICommInfoNode)



%define PY_GETATTRLIST(Typename)
%extend Typename {
  %pythoncode %{
      def __getitem__(self,key):
        if key == "metaData" or key == "MetaData":
            return json.loads(self.getMetaData().asJson())
         
        if isinstance(key,int) and abs(key) < self.size() :
          if (key < 0 ) :
             key = self.size() + key
          return castDataBase(self.get(key))

        elif isinstance(key,slice):
           sli = range(0,self.size())[slice]
           res = []
           for item in sli:
             res.append(self.__getitem__(item));
           return res

        elif isinstance(key,str):

           res = []
           for i in self:
             if i.getName() == key:
               res.append(i)
           if len(res) == 1:
              return res[0]
           elif len(res)>1:
              return res

        print("Not a key {} {} ".format(key, self.__class__))
        raise KeyError("not a valid key")

      def __len__(self):
        return self.size();

      def __iter__(self):
        return listclassIterator(self)

      def __getType__(self):
        return "list"

      def __str__(self):
              return str(self.getValue())
   %}
}
%enddef

PY_GETATTRLIST(VnV::Nodes::IArrayNode)

%define PY_GETATTRMAP(Typename)
%extend Typename {
    %pythoncode %{

        def __getitem__(self,key):
            if key == "metaData" or key == "MetaData":
              return json.loads(self.getMetaData().asJson())
         
            
            if isinstance(key,str) and self.contains(key):
                return castDataBase(self.get(key))
            print("Not a key {} {} ".format(key, self.__class__))
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
             return str(self.getValue())
    %}
}
%enddef
PY_GETATTRMAP(VnV::Nodes::IMapNode)


%define PY_GETATTRSHAPE(Typename)
%extend Typename {
  %pythoncode %{

      def recursive_pop(self, b):
        result = []
        for i in b:
          if isinstance(i,list):
             result.append(self.recursive_pop(i))
          else:
             result.append(self.getValue(i))
        return result

      def shape(self):
         if not hasattr(self, shape):
            self.shape = json.loads(self.getShapeJson());
            self.tot = np.prod(shape)  #total size of the list.
            self.npshape = np.reshape(range(0,self.tot), tuple(self.shape) ) #array with values as correct index.
         return self.shape, self.tot, self.npshape

         
      def __getitem__(self,key):
        if key == "metaData" or key == "MetaData":
            return json.loads(self.getMetaData().asJson()) 


        try:
         s,t,nps = self.shape()
         if (len(s) == 0) :
            raise TypeError("Scalar Shape has no __getitem__ method")
         
         
         b = nps[key].tolist() # Slice the array

         if (isinstance(b,list)):
            return np.ndarray(self.recursive_pop(b))
         else:
             return self.getValue(b)
        except:
             pass

        print("Not a key {} {} ".format(key, self.__class__))
        raise KeyError("not a valid key")

      def __len__(self):
         shape = json.loads(self.getShapeJson());
         if (len(shape) == 0) :
            raise TypeError("Scalar Object has no length")
         
         return shape[-1]

      def __iter__(self):
        return shapeclassIterator(self)

      def __getType__(self):
        return "list"

      def __str__(self):
          return str(self.getValue())
   %}
}
%enddef

PY_GETATTRSHAPE(VnV::Nodes::IDoubleNode)
PY_GETATTRSHAPE(VnV::Nodes::IIntegerNode)
PY_GETATTRSHAPE(VnV::Nodes::IFloatNode)
PY_GETATTRSHAPE(VnV::Nodes::IStringNode)
PY_GETATTRSHAPE(VnV::Nodes::ILongNode)
PY_GETATTRSHAPE(VnV::Nodes::IBoolNode)
PY_GETATTRSHAPE(VnV::Nodes::IJsonNode)
PY_GETATTRSHAPE(VnV::Nodes::IShapeNode)

