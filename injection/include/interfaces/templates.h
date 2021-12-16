#define TESTING 0
/************************************************************
 * 
 *  Lots of template parameters match them . 
 *
 *   VNV_TEMPLATE_5(a,a1,b,b1,c,c1,d,d1,e,e1, {
 *        printf("TGFSDG");		
 *   })
 *  
 *
 *   if (auto five = GetPtr(a, a1)) { 
 *     if (auto four = GetPtr(b, b1)) { 
 *       if (auto three = GetPtr(c, c1)) { 
 *         if (auto two = GetPtr(d, d1)) { 
 *           if (auto one = GetPtr(e, e1)) { 
 *             { 
 *               printf("TGFSDG");
 *             } 
 *           }
 *         }
 *       }
 *     }
 *   }
 * 
 */

#define VNV_TEMPLATE_(VAR, NAME, PARAM,CALLBACK) \
  if (auto VAR = GetPtr(NAME, PARAM)) { \
     CALLBACK \
  }	  

#define VNV_TEMPLATE_1(NAME, PARAM, CALLBACK) \
    VNV_TEMPLATE_(one, NAME, PARAM, CALLBACK)

#define VNV_TEMPLATE_2(NAME, PARAM,...)\
   VNV_TEMPLATE_(two, NAME, PARAM, VNV_TEMPLATE_1(__VA_ARGS__))

#define VNV_TEMPLATE_3(NAME, PARAM,...) \
   VNV_TEMPLATE_(three, NAME, PARAM, VNV_TEMPLATE_2(__VA_ARGS__))
   	
#define VNV_TEMPLATE_4(NAME,PARAM,...) \
   VNV_TEMPLATE_(four, NAME, PARAM, VNV_TEMPLATE_3(__VA_ARGS__))

#define VNV_TEMPLATE_5(NAME, PARAM,...) \
   VNV_TEMPLATE_(five, NAME, PARAM, VNV_TEMPLATE_4(__VA_ARGS__))

#if TESTING
VNV_TEMPLATE_5(a,a1,b,b1,c,c1,d,d1,e,e1, {
   printf("TGFSDG");		
})
#endif

/************************************************************
 * 
 *
 *  One  parameters iterated over a number of times. 
 *
 *  I.e. TEMPALTE_1_2({printf("Hello");}, {printf{"THERE"}, "a", double, float, int, double);
 *  
 *  if (auto one = GetPtr("a", double)) { { printf("Hello"); } } 
 *  else if (auto one = GetPtr("a", float)) { { printf("Hello"); } } 
 *  else if (auto one = GetPtr("a", int)) { { printf("Hello"); } } 
 *  else if (auto one = GetPtr("a", double)) { { printf("Hello"); } }
 *  else {{ printf("THERE"); }}
 * 
 */

#define VNV_TEMPLATE_1_1(CALLBACK, ELSE, NAME, PARAM )\
   VNV_TEMPLATE_1(NAME, PARAM, CALLBACK)\
   else {ELSE}

#define VNV_TEMPLATE_1_2(CALLBACK, ELSE, NAME, PARAM, ... )\
   VNV_TEMPLATE_1(NAME, PARAM, CALLBACK)\
   else VNV_TEMPLATE_1_1(CALLBACK,  ELSE, NAME, __VA_ARGS__) 

#define VNV_TEMPLATE_1_3(CALLBACK, ELSE,  NAME, PARAM, ... )\
   VNV_TEMPLATE_1(NAME, PARAM, CALLBACK)\
   else VNV_TEMPLATE_1_2(CALLBACK, ELSE,  NAME, __VA_ARGS__) 

#define VNV_TEMPLATE_1_4(CALLBACK, ELSE,  NAME, PARAM, ... )\
   VNV_TEMPLATE_1(NAME, PARAM, CALLBACK)\
   else VNV_TEMPLATE_1_3(CALLBACK, ELSE,  NAME, __VA_ARGS__) 
   
#if TESTING
VNV_TEMPLATE_1_4({
  printf("Hello");
}, {
  printf("THERE");
 }, 
"a", double, float, int, double);
#endif

/************************************************************
 * 
 *  Two parameters iterated over a number of times. 
 *
 *  I.e. TEMPALTE_2_2({printf("Hello");}, {printf{"THERE"}, "a","b", double, float, int, double);
 *  if (auto two = GetPtr("a", double)) { 
 *    if (auto one = GetPtr("b", float)) { {printf("Hello");} }
 *  } else if (auto two = GetPtr("a", int)) { 
 *    if (auto one = GetPtr("b", double)) { {printf("Hello");} }
 *   } else {{printf{"THERE"}};
 * 
 */

#define VNV_TEMPLATE_2_1(CALLBACK, ELSE, NAME, NAME1, PARAM , PARAM1 )\
   VNV_TEMPLATE_2(NAME, PARAM, NAME1, PARAM1,  CALLBACK)\
   else {ELSE}

#define VNV_TEMPLATE_2_2(CALLBACK, ELSE, NAME, NAME1, PARAM, PARAM1, ... )\
   VNV_TEMPLATE_2(NAME, PARAM, NAME1, PARAM1, CALLBACK)\
   else VNV_TEMPLATE_2_1(CALLBACK,  ELSE, NAME, NAME1, __VA_ARGS__) 

#define VNV_TEMPLATE_2_3(CALLBACK, ELSE, NAME, NAME1, PARAM, PARAM1 ,... )\
   VNV_TEMPLATE_2(NAME, PARAM, NAME1, PARAM1, CALLBACK)\
   else VNV_TEMPLATE_2_2(CALLBACK,  ELSE, NAME, NAME1, __VA_ARGS__) 

#define VNV_TEMPLATE_2_4(CALLBACK, ELSE, NAME, NAME1, PARAM, PARAM1, ... )\
   VNV_TEMPLATE_2(NAME, PARAM, NAME1, PARAM1, CALLBACK)\
   else VNV_TEMPLATE_2_3(CALLBACK,  ELSE, NAME, NAME1, __VA_ARGS__) 

#if TESTING
VNV_TEMPLATE_2_2({printf("Hello");}, {printf{"THERE"}, "a","b", double, float, int, double);
#endif

/************************************************************************/

