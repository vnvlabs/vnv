
 
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
    

 
