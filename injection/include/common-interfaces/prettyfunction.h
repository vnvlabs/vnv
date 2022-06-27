#ifndef VNV_FUNCTION_SIG_H
#define VNV_FUNCTION_SIG_H

#if __cplusplus
#  define VNVEXTERNC extern "C"
#else
#  define VNVEXTERNC
#endif

//This code detects the compiler and sets the VNV_FUNCTION_SIG macro accordingly.
//Shamelessly copied from the boost library -- https://www.boost.org/doc/libs/1_77_0/boost/current_function.hpp

#if defined(__GNUC__) 

# define VNV_FUNCTION_SIG _VnV_function_sig( "GNU" , __PRETTY_FUNCTION__ )

#elif (defined(__MWERKS__) && (__MWERKS__ >= 0x3000)) 

# define VNV_FUNCTION_SIG _VnV_function_sig( "MWERKS" , __PRETTY_FUNCTION__  )

#elif (defined(__ICC) && (__ICC >= 600)) 

# define VNV_FUNCTION_SIG _VnV_function_sig( "ICC" , __PRETTY_FUNCTION__  )

#elif defined(__ghs__) 

# define VNV_FUNCTION_SIG _VnV_function_sig( "GHS" , __PRETTY_FUNCTION__  )

#elfif defined(__clang__)

# define VNV_FUNCTION_SIG _VnV_function_sig( "Clang" , __PRETTY_FUNCTION__  )

#elif defined(__DMC__) && (__DMC__ >= 0x810)

# define VNV_FUNCTION_SIG _VnV_function_sig( "DMC" , __PRETTY_FUNCTION__  )

#elif defined(__FUNCSIG__)

# define VNV_FUNCTION_SIG _VnV_function_sig( "MS" , __PRETTY_FUNCTION__  )

#else

# define VNV_FUNCTION_SIG _VnV_function_sig( "NA", "" ) 

#endif

struct VnV_Function_Sig {
    const char* compiler;
    const char* signiture;
};

VNVEXTERNC struct VnV_Function_Sig _VnV_function_sig(const char* compiler, const char* sig);

#endif // #ifndef 