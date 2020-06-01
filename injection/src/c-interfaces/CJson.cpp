/**
  @file CJson.cpp
**/

#include "c-interfaces/CJson.h"
#include <iostream>
#include "base/OptionsParserStore.h"
using nlohmann::json;

#define CJSON_OK 0
#define CJSON_INVALID_DATA 1
#define CJSON_NOT_AN_ARRAY 2
#define CJSON_ARRAY_INDEX_OUT_OF_BOUNDS 22
#define CJSON_NOT_AN_OBJECT 3
#define CJSON_OBJECT_KEY_DOES_NOT_EXIST 33
#define CJSON_NOT_A_STRING 4
#define CJSON_NOT_A_NUMBER 5
#define CJSON_NOT_A_INT 6
#define CJSON_NOT_A_UNSIGNED_INT 7
#define CJSON_NOT_A_FLOAT 8
#define CJSON_NOT_A_BOOL 9

void VnV_CJson_getErrorMessage(int code, char** message) {
   *message = (char*) malloc(100*sizeof(char));
    switch(code){
      case CJSON_OK: strcpy(*message, "Valid"); break;
      case CJSON_INVALID_DATA: strcpy(*message, "data object is null"); break;
      case CJSON_NOT_AN_ARRAY: strcpy(*message, "json object is not an array"); break;
      case CJSON_NOT_A_INT: strcpy(*message, "json object is not an integer"); break;
      case CJSON_NOT_A_BOOL: strcpy(*message, "json object is not a bool"); break;
      case CJSON_NOT_A_FLOAT: strcpy(*message, "json object is not a float"); break;
      case CJSON_NOT_A_UNSIGNED_INT: strcpy(*message, "json object is not an unsigned int"); break;
      case CJSON_NOT_AN_OBJECT: strcpy(*message, "json object is not a object"); break;
      case CJSON_NOT_A_NUMBER: strcpy(*message, "json object is not a number"); break;
      case CJSON_NOT_A_STRING: strcpy(*message, "json object is not a string"); break;
      case CJSON_OBJECT_KEY_DOES_NOT_EXIST: strcpy(*message, "key does not exist in json object"); break;
      case CJSON_ARRAY_INDEX_OUT_OF_BOUNDS: strcpy(*message, "array index out of bounds for json array"); break;
    }
}


bool VnV_CJson_check(c_json json, int *err) {
    *err = (json.data != nullptr) ? 0 : CJSON_INVALID_DATA;
    return *err == CJSON_OK;
}

json* VnV_CJson_castJson(c_json jsonRaw) {
    return (json*) jsonRaw.data;
}

void VnV_CJson_is_object(c_json json, bool *result, int* err) {
    if (VnV_CJson_check(json, err)) {
       *result =  VnV_CJson_castJson(json)->is_object();
    }
}

void VnV_CJson_is_null(c_json json, bool *result, int* err) {
    if (VnV_CJson_check(json,err))
       *result = VnV_CJson_castJson(json)->is_null();
}

void VnV_CJson_is_array(c_json json, bool *result, int* err) {
    if (VnV_CJson_check(json,err))
         *result = VnV_CJson_castJson(json)->is_array();
}

void VnV_CJson_is_number(c_json json, bool *result, int* err) {
    if (VnV_CJson_check(json,err))
        *result = VnV_CJson_castJson(json)->is_number();
}
void VnV_CJson_is_string(c_json json, bool *result, int* err) {
    if (VnV_CJson_check(json, err))
        *result = VnV_CJson_castJson(json)->is_string();
}

void VnV_CJson_is_boolean(c_json json, bool *result, int* err) {
    if (VnV_CJson_check(json,err))
        *result = VnV_CJson_castJson(json)->is_boolean();
}

void VnV_CJson_is_number_float(c_json json, bool *result, int *err) {
    if (VnV_CJson_check(json,err))
        *result = VnV_CJson_castJson(json)->is_number_float();
}

void VnV_CJson_is_number_unsigned(c_json json, bool *result, int *err) {
    if (VnV_CJson_check(json,err))
        *result = VnV_CJson_castJson(json)->is_number_unsigned();
}

void VnV_CJson_is_number_integer(c_json json, bool *result, int *err) {
    if (VnV_CJson_check(json,err))
        *result = VnV_CJson_castJson(json)->is_number_integer();
}

void VnV_CJson_getArraySize(c_json json, unsigned long *result, int *err) {
    bool yes;
    VnV_CJson_is_array(json,&yes,err);
    if (*err == CJSON_OK) {
        if (yes){
          *result =  VnV_CJson_castJson(json)->size();
        } else {
            *err = CJSON_NOT_AN_ARRAY;
        }
    }
}

void VnV_CJson_getArrayIndex(c_json json, unsigned int index, c_json *result, int *err) {
    unsigned long size;
    VnV_CJson_getArraySize(json,&size,err);
    if (*err == CJSON_OK) {
        if (index < size) {
           *result = {&(VnV_CJson_castJson(json)->at(index))};
        } else {
            *err = CJSON_ARRAY_INDEX_OUT_OF_BOUNDS;
        }
    }
}
void VnV_CJson_hasKey(c_json json, char* key, bool *result, int *err) {
    bool yes;
    VnV_CJson_is_object(json,&yes, err);
    if (*err == CJSON_OK) {
        if (yes) {
            *result = VnV_CJson_castJson(json)->contains(key);
        } else {
            *err = CJSON_NOT_AN_OBJECT;
        }
    }
}

void VnV_CJson_getKeyValue(c_json json, char* key, c_json* result, int *err) {
    bool yes;
    VnV_CJson_hasKey(json, key, &yes, err);
    if (*err == CJSON_OK) {
        if (yes) {
            nlohmann::json *j = VnV_CJson_castJson(json);
            auto it = j->find(key);
            *result = {&(it.value())};
        } else {
            *err = CJSON_OBJECT_KEY_DOES_NOT_EXIST;
        }
    }
}



void VnV_CJson_getAllKeys(c_json json, char***keys, unsigned long* size, int *err) {

    bool yes;
    VnV_CJson_is_object(json,&yes, err);
    if (*err == CJSON_OK) {
        if (yes) {
            nlohmann::json *j = VnV_CJson_castJson(json);
            *size = j->size();
            char** k = (char**) malloc(sizeof(char*)*(*size));
            int count = 0;
            for ( auto it : j->items()) {
                k[count] = (char*) malloc(sizeof(char)*(it.key().length()+2));
                strcpy(k[count++], it.key().c_str());
            }
            *keys = k;
        } else {
            *err = CJSON_NOT_AN_OBJECT;
        }
    }
}

void VnV_CJson_freeAllKeys(char***keys, int* size, int *err)  {
    for (int i = 0; i < *size; i++) {
        free((*keys)[i]);
    }
    free(*keys);
    *err = CJSON_OK;
}

// C Interface
void VnV_CJson_getString(c_json json, char** result, int *err) {
    bool yes;
    VnV_CJson_is_string(json,&yes,err);
    if (*err == CJSON_OK) {
        if (yes) {
            std::string s = VnV_CJson_castJson(json)->get<std::string>();
            *result = (char*) malloc(sizeof(char)*(s.length()+1));
            strcpy(*result, s.c_str());
        } else {
            *err = CJSON_NOT_A_STRING;
        }
    }
}

void VnV_CJson_freeString(c_json json, char** ptr, int *err) {
   free(*ptr);
   *err = CJSON_OK;
}


void VnV_CJson_getInt(c_json json, int* result, int *err) {
    bool yes;
    VnV_CJson_is_number_integer(json,&yes,err);
    if (*err == CJSON_OK) {
        if (yes) {
            *result = VnV_CJson_castJson(json)->get<int>();
        } else {
            *err = CJSON_NOT_A_INT;
        }
    }
}

void VnV_CJson_getUnsignedInt(c_json json, unsigned int* result, int *err) {

    bool yes;
    VnV_CJson_is_number_unsigned(json,&yes,err);
    if (*err == CJSON_OK) {
        if (yes) {
            *result = VnV_CJson_castJson(json)->get<unsigned int>();
        } else {
            *err = CJSON_NOT_A_UNSIGNED_INT;
        }
    }
}

void VnV_CJson_getFloat(c_json json, float* result, int *err) {

    bool yes;
    VnV_CJson_is_number_float(json,&yes,err);
    if (*err == CJSON_OK) {
        if (yes) {
            *result = VnV_CJson_castJson(json)->get<float>();
        } else {
            *err = CJSON_NOT_A_FLOAT;
        }
    }
}

void VnV_CJson_getBoolean(c_json json, bool *result, int *err) {

    bool yes;
    VnV_CJson_is_boolean(json,&yes,err);
    if (*err == CJSON_OK) {
        if (yes) {
            *result = VnV_CJson_castJson(json)->get<bool>();
        } else {
            *err = CJSON_NOT_A_BOOL;
        }
    }
}

void VnV_CJson_printJson(c_json json, int *err) {
    if (VnV_CJson_check(json,err)) {
        std::cout << VnV_CJson_castJson(json)->dump();
    }
}

void _VnV_registerOptions(const char* name, const char* s, options_callback_ptr v) {
   json k = json::parse(s);
   VnV::OptionsParserStore::instance().add(name,k, v);
}


void VnV::RegisterOptions(std::string packageName, std::string schema, options_cpp_callback_ptr callback) {
   json j = json::parse(schema);
   VnV::RegisterOptions_Json(packageName, j, callback);
}

void VnV::RegisterOptions_Json(std::string name, json &schema, options_cpp_callback_ptr callback) {
   VnV::OptionsParserStore::instance().add(name, schema, callback);
}

json* VnV::asJson(c_json json) {
    return VnV_CJson_castJson(json);
}
