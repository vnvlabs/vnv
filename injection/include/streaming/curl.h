#ifndef CURL_WRAPPER_VNV_H
#define CURL_WRAPPER_VNV_H

#if WITH_LIBCURL

#include "json-schema.hpp"
#include <curl/curl.h>
using nlohmann::json;

namespace VnV {
namespace Curl {



class CurlWrapper {
  std::string rr;
  CURL *curl = NULL;
  struct curl_slist* headers = NULL;

  static size_t writefunc(void* ptr, size_t size, size_t nmemb, std::string* s) {
    s->append(static_cast<char*>(ptr), size * nmemb);
    return size * nmemb;
  }   

  CurlWrapper() {
      curl_global_init(CURL_GLOBAL_ALL);
      curl = curl_easy_init();
      
      if (!curl) {
        throw VnV::VnVExceptionBase("Could not set up curl");
      }

      headers = curl_slist_append(headers, "charset: utf-8");
      curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
      curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcrp/0.1");
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWrapper::writefunc);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, &rr);
      curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  }
  


  ~CurlWrapper() {
      curl_slist_free_all(headers);
      curl_easy_cleanup(curl);
      curl_global_cleanup();
  }


public:
   static CurlWrapper& instance() {
       static CurlWrapper wrapper;
       return wrapper;
   }

  std::string u,p;
  void addBasic(const std::string &user, const std::string &password) {
     u = user;
     p = password;
     curl_easy_setopt(curl, CURLOPT_HTTPAUTH, (long)CURLAUTH_BASIC);
     curl_easy_setopt(curl, CURLOPT_USERNAME, u.c_str());
     curl_easy_setopt(curl, CURLOPT_PASSWORD, p.c_str());
  }

  std::string url_;
  void setUrl(const std::string &url) {
      url_ = url;
      curl_easy_setopt(curl, CURLOPT_URL, url_.c_str());
  }  

  void clearData() {
      rr.clear();
  }

  std::string getData() {
      return rr;
  }

  std::string pdata;
  void setPostFields(const std::string &postData) {
     pdata = postData;
     curl_easy_setopt(curl, CURLOPT_POSTFIELDS, pdata.c_str());
  }

  void setPostFields(std::map<std::string,std::string> &m) {
      std::ostringstream oss;
      
      int i=0;
      for (auto it : m) {
          oss << ((++i==1)?"":"&") << it.first << it.second;
      }
      setPostFields(oss.str());
  }

  long getCode() {
      long http_code;
      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
      return http_code;
  }

  void send() {
     curl_easy_perform(curl);
  }

};



}
}
#endif
#endif
