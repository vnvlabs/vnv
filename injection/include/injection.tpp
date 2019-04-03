
 
template <typename T> 
T* IVVTest::carefull_cast(int stage, std::string parameterName, NTV &parameters ) {

        // First, make sure "parameterName" is a test parameter and, if it is, get its type
        auto test_parameter = m_parameters.find(parameterName);
        if ( test_parameter == m_parameters.end() ) {
            throw " This is not a test parameter ";
        }

        //Next, get the transform. If one exists, it is returned, else return "parameterName,DefaultTransform">
        std::pair<std::string, IVVTransform*>  trans = m_config.getStage(stage).getTransform(parameterName);
      
        //Next, get the parameter from the injection point parameters that the transform says we want. 
        auto ip_parameter = parameters.find(trans.first);       
        if ( ip_parameter == parameters.end() ) {
            throw "A injection point parameter with the transform name does not exist" ; 
        }

        // Transform the injection point parameter into the test parameter 
        // Transforms should check the types 
        void * tptr = trans.second->Transform(ip_parameter->second, test_parameter->second);        
        
        // Finally, cast it to the correct type; 
        return (T*) tptr; 
}

template<typename T> 
void InjectionPoint::ntv_unpacker(int stageValue, NTV &ntv, int count, T parameter) {
      

      auto s = stages.find(stageValue);
      if ( s != stages.end() ) {

          if ( count >= s->second.m_params.size() )
            return;      
      
          ntv.insert(std::make_pair(s->second.m_params[count].first, std::make_pair(s->second.m_params[count].second, (void*) parameter))); 
      
         if ( count + 1 < s->second.m_params.size() ) 
            throw "Not enough parameters supplied to this injection point";      
      }
}

template<typename T, typename ...Args> 
void InjectionPoint::ntv_unpacker(int stageValue, NTV &ntv, int count, T parameter, Args...args) {
      
      auto s = stages.find(stageValue);
      if ( s != stages.end() ) {
  
          if ( count >= s->second.m_params.size() )
              return;      

          ntv.insert(std::make_pair(s->second.m_params[count].first, std::make_pair(s->second.m_params[count].second, (void*) parameter)));
          ntv_unpacker(stageValue, ntv, ++count, args...);
      }
}

template<typename ...Args> 
void InjectionPoint::runTests(int stageValue ,Args...args) {
      
      auto s = stages.find(stageValue);
      if ( s != stages.end() ) {
           // Write BeginStep Injection point scope 
           AdiosWrapper *wrapper = VV::adiosWrapper;
           NTV ntv, converted_ntv; // map of name -- > { type, void* } 
           ntv_unpacker(stageValue, ntv,0,args...); // Unpack the args and package them as an ntv:   
          
           // Call the method to write this injection point to file.  
           wrapper->startInjectionPoint(getScope(),stageValue, m_markdown);
           for ( auto it : m_tests ) {
              it->_runTest(stageValue,ntv);
           }     
           wrapper->endInjectionPoint(getScope(),stageValue, m_markdown);
       }  
}

template<typename ...Args>
InjectionPoint::InjectionPoint(std::string scope, int stage, std::string filename, int lineNumber, std::string desc, Args...args ) : 
    m_scope(scope)
{
  stages.insert(std::make_pair(stage, InjectionPointStage(filename,lineNumber, desc, args...)));
}

template< typename ...Args> 
void InjectionPoint::addStage(int stageIndex, std::string filename, int lineNumber, std::string desc, Args...args) {

  auto s = stages.find(stageIndex);
  if ( s == stages.end() ) {
    stages.insert(std::make_pair(stageIndex, InjectionPointStage(filename,lineNumber, desc, args...)));    
  } else if ( s->second.isEmpty() ) {
      s->second.setFromEmpty(filename, lineNumber, desc,args...);
  } 
}


template<typename ...Args>
InjectionPointStage::InjectionPointStage(std::string filename, int lineNumber, std::string desc, Args...args ) : 
    m_filename(filename),
    m_lineNumber(lineNumber) ,
    m_desc(desc)
{
  argsToVec(m_params,args...);    
}

template<typename ...Args>
void InjectionPointStage::setFromEmpty(std::string filename, int lineNumber, std::string desc, Args...args ) {
    
    m_filename = filename;
    m_lineNumber = lineNumber;
    m_desc = desc;
    m_empty = false;  
    argsToVec(m_params,args...);    
}

 
template<typename ...Args>
void InjectionPointStage::argsToVec(plist &p, std::string first, std::string second, Args...args) {

    p.push_back( std::make_pair(second,first) ); // args come in pairs (type - name )
    argsToVec(p,args...);
}

template<typename ...Args>
InjectionPointRegistrar::InjectionPointRegistrar(std::string scope,  // the name of the injection point  
                                                 int injectionIndex,   // the index of the injection point in the loop ( -1 == start , -2 == end, 0 == single, >0 = index between start and end) 
                                                 std::string filename, // the filename of the injection point in the loop 
                                                 int line,             // the line where the injection point was registered 
                                                 std::string desc,     // the json file where the injection point description is located. 
                                              Args...args) {

   /// Need to figure out 
    
   auto it = getMap()->find(scope); 
   if ( it == getMap()->end()) {
      getMap()->insert( std::make_pair(scope, InjectionPoint(scope, injectionIndex, filename, line, desc, args... ) ) );
   } else 
      it->second.addStage(injectionIndex, filename, line, desc, args...);
}


template<typename ...Args>
void VV::injectionPoint(int injectionIndex, std::string scope, std::string function, Args... args) {
    if ( runTests ) {
        auto ipd =  InjectionPointBaseFactory::getDescription(scope);
        ipd.runTests(injectionIndex, args...);
    }
};
    

 
