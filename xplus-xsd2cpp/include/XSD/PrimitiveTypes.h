
 //
 //  This file was automatically generated using XmlPlus xsd2cpp tool.
 //  Please do not edit.
 //

#ifndef __XSD_PRIMITIVETYPES_H__ 
#define __XSD_PRIMITIVETYPES_H__

extern "C" {
#include <math.h>
#include <assert.h>
}
#include <string>
#include <list>

#include "XPlus/Types.h"
#include "XPlus/StringUtils.h"
#include "XSD/Enums.h"
#include "XSD/UrTypes.h"
#include "XSD/SimpleTypeListTmpl.h"
      
    
#include "XSD/xsdUtils.h"
#include "DOM/DOMCommonInc.h"

using namespace std;
using namespace XPlus;
using namespace DOM;
using namespace XMLSchema;


namespace XMLSchema {
    

namespace Types 
{
    

  /// class for simpleType with restriction on base
  class bt_string : public XMLSchema::Types::anySimpleType
  {
  public:
    /// constructor  
    bt_string(AnyTypeCreateArgs args)
    
        : anySimpleType(args, PD_STRING)
        
    {
    
      _whiteSpaceCFacet.stringValue("preserve");
      
      this->allowedCFacets( CF_NONE  | CF_LENGTH | CF_MINLENGTH | CF_MAXLENGTH | CF_PATTERN | CF_ENUMERATION | CF_WHITESPACE );
    
      this->appliedCFacets( appliedCFacets() | CF_WHITESPACE );
    }
    
    inline DOM::DOMString value() {
      return _implValue;
    }
    inline void value(DOM::DOMString val) {
      
      string strVal = toString<DOM::DOMString>(val);
        
      anySimpleType::stringValue(strVal);
      //_implValue = val;
    }
      
    virtual inline DOMString sampleValue() {
      return anySimpleType::generateSample(Sampler::stringSamples);
    }
    
  protected:
    
    inline virtual void setTypedValue() {
      
      _implValue = fromString<DOM::DOMString>(_value);
        
    }

    DOM::DOMString _implValue;
      
  };
  

  /// class for simpleType with restriction on base
  class bt_boolean : public XMLSchema::Types::anySimpleType
  {
  public:
    /// constructor  
    bt_boolean(AnyTypeCreateArgs args)
    
        : anySimpleType(args, PD_BOOLEAN)
        
    {
    
      _whiteSpaceCFacet.stringValue("collapse");
      _whiteSpaceCFacet.fixed(true);
      this->allowedCFacets( CF_NONE  | CF_PATTERN | CF_WHITESPACE );
    
      this->appliedCFacets( appliedCFacets() | CF_WHITESPACE );
    }
    
    inline bool value() {
      return _implValue;
    }
    inline void value(bool val) {
      
      string strVal = toString<bool>(val);
        
      anySimpleType::stringValue(strVal);
      //_implValue = val;
    }
      
    virtual inline DOMString sampleValue() {
      return anySimpleType::generateSample(Sampler::booleanSamples);
    }
    
  protected:
    
    inline virtual void setTypedValue() {
      
      _implValue = fromString<bool>(_value);
        
    }

    bool _implValue;
      
  };
  

  /// class for simpleType with restriction on base
  class bt_float : public XMLSchema::Types::anySimpleType
  {
  public:
    /// constructor  
    bt_float(AnyTypeCreateArgs args)
    
        : anySimpleType(args, PD_FLOAT)
        
    {
    
      _whiteSpaceCFacet.stringValue("collapse");
      _whiteSpaceCFacet.fixed(true);
      this->allowedCFacets( CF_NONE  | CF_PATTERN | CF_ENUMERATION | CF_WHITESPACE | CF_MAXINCLUSIVE | CF_MAXEXCLUSIVE | CF_MININCLUSIVE | CF_MINEXCLUSIVE );
    
      this->appliedCFacets( appliedCFacets() | CF_WHITESPACE );
    }
    
    inline float value() {
      return _implValue;
    }
    inline void value(float val) {
      
      string strVal = toString<float>(val);
        
      anySimpleType::stringValue(strVal);
      //_implValue = val;
    }
      
    virtual inline DOMString sampleValue() {
      return anySimpleType::generateSample(Sampler::floatSamples);
    }
    
  protected:
    
    virtual void applyMaxInclusiveCFacet() 
    {
      try {
      
        if(_implValue > _maxInclusiveCFacetDouble.value()) 
        
        {
          throwFacetViolation(CF_MAXINCLUSIVE);
        }
      }
      catch(DateTimeException& ex) {
        throwFacetViolation(CF_MAXINCLUSIVE, ex.rawMsg());
      }
      catch(XPlus::Exception& ex) {
        throwFacetViolation(CF_MAXINCLUSIVE);
      }
    }
      
    virtual void applyMaxExclusiveCFacet() 
    {
      try {
      
        if(_implValue >= _maxExclusiveCFacetDouble.value()) 
        
        {
          throwFacetViolation(CF_MAXEXCLUSIVE);
        }
      }
      catch(DateTimeException& ex) {
        throwFacetViolation(CF_MAXINCLUSIVE, ex.rawMsg());
      }
      catch(XPlus::Exception& ex) {
        throwFacetViolation(CF_MAXEXCLUSIVE);
      }
    }
      
    virtual void applyMinInclusiveCFacet() 
    {
      try {
      
        if(_implValue < _minInclusiveCFacetDouble.value()) 
        
        {
          throwFacetViolation(CF_MININCLUSIVE);
        }
      }
      catch(DateTimeException& ex) {
        throwFacetViolation(CF_MAXINCLUSIVE, ex.rawMsg());
      }
      catch(XPlus::Exception& ex) {
        throwFacetViolation(CF_MININCLUSIVE);
      }
    }
      
    virtual void applyMinExclusiveCFacet() 
    {
      try {
      
        if(_implValue <= _minExclusiveCFacetDouble.value()) 
        
        {
          throwFacetViolation(CF_MINEXCLUSIVE);
        }
      }
      catch(DateTimeException& ex) {
        throwFacetViolation(CF_MAXINCLUSIVE, ex.rawMsg());
      }
      catch(XPlus::Exception& ex) {
        throwFacetViolation(CF_MINEXCLUSIVE);
      }
    }
      
    inline virtual void setTypedValue() {
      
      _implValue = fromString<float>(_value);
        
    }

    float _implValue;
      
  };
  

  /// class for simpleType with restriction on base
  class bt_double : public XMLSchema::Types::anySimpleType
  {
  public:
    /// constructor  
    bt_double(AnyTypeCreateArgs args)
    
        : anySimpleType(args, PD_DOUBLE)
        
    {
    
      _whiteSpaceCFacet.stringValue("collapse");
      _whiteSpaceCFacet.fixed(true);
      this->allowedCFacets( CF_NONE  | CF_PATTERN | CF_ENUMERATION | CF_WHITESPACE | CF_MAXINCLUSIVE | CF_MAXEXCLUSIVE | CF_MININCLUSIVE | CF_MINEXCLUSIVE );
    
      this->appliedCFacets( appliedCFacets() | CF_WHITESPACE );
    }
    
    inline double value() {
      return _implValue;
    }
    inline void value(double val) {
      
      string strVal = toString<double>(val);
        
      anySimpleType::stringValue(strVal);
      //_implValue = val;
    }
      
    virtual inline DOMString sampleValue() {
      return anySimpleType::generateSample(Sampler::doubleSamples);
    }
    
  protected:
    
    virtual void applyMaxInclusiveCFacet() 
    {
      try {
      
        if(_implValue > _maxInclusiveCFacetDouble.value()) 
        
        {
          throwFacetViolation(CF_MAXINCLUSIVE);
        }
      }
      catch(DateTimeException& ex) {
        throwFacetViolation(CF_MAXINCLUSIVE, ex.rawMsg());
      }
      catch(XPlus::Exception& ex) {
        throwFacetViolation(CF_MAXINCLUSIVE);
      }
    }
      
    virtual void applyMaxExclusiveCFacet() 
    {
      try {
      
        if(_implValue >= _maxExclusiveCFacetDouble.value()) 
        
        {
          throwFacetViolation(CF_MAXEXCLUSIVE);
        }
      }
      catch(DateTimeException& ex) {
        throwFacetViolation(CF_MAXINCLUSIVE, ex.rawMsg());
      }
      catch(XPlus::Exception& ex) {
        throwFacetViolation(CF_MAXEXCLUSIVE);
      }
    }
      
    virtual void applyMinInclusiveCFacet() 
    {
      try {
      
        if(_implValue < _minInclusiveCFacetDouble.value()) 
        
        {
          throwFacetViolation(CF_MININCLUSIVE);
        }
      }
      catch(DateTimeException& ex) {
        throwFacetViolation(CF_MAXINCLUSIVE, ex.rawMsg());
      }
      catch(XPlus::Exception& ex) {
        throwFacetViolation(CF_MININCLUSIVE);
      }
    }
      
    virtual void applyMinExclusiveCFacet() 
    {
      try {
      
        if(_implValue <= _minExclusiveCFacetDouble.value()) 
        
        {
          throwFacetViolation(CF_MINEXCLUSIVE);
        }
      }
      catch(DateTimeException& ex) {
        throwFacetViolation(CF_MAXINCLUSIVE, ex.rawMsg());
      }
      catch(XPlus::Exception& ex) {
        throwFacetViolation(CF_MINEXCLUSIVE);
      }
    }
      
    inline virtual void setTypedValue() {
      
      _implValue = fromString<double>(_value);
        
    }

    double _implValue;
      
  };
  

  /// class for simpleType with restriction on base
  class bt_decimal : public XMLSchema::Types::anySimpleType
  {
  public:
    /// constructor  
    bt_decimal(AnyTypeCreateArgs args)
    
        : anySimpleType(args, PD_DECIMAL)
        
    {
    
      _whiteSpaceCFacet.stringValue("collapse");
      _whiteSpaceCFacet.fixed(true);
      this->allowedCFacets( CF_NONE  | CF_TOTALDIGITS | CF_FRACTIONDIGITS | CF_PATTERN | CF_WHITESPACE | CF_ENUMERATION | CF_MAXINCLUSIVE | CF_MAXEXCLUSIVE | CF_MININCLUSIVE | CF_MINEXCLUSIVE );
    
      this->appliedCFacets( appliedCFacets() | CF_WHITESPACE );
    }
    
    inline double value() {
      return _implValue;
    }
    inline void value(double val) {
      
      string strVal = toString<double>(val);
        
      anySimpleType::stringValue(strVal);
      //_implValue = val;
    }
      
    virtual inline DOMString sampleValue() {
      return anySimpleType::generateSample(Sampler::decimalSamples);
    }
    
  protected:
    
    virtual void applyMaxInclusiveCFacet() 
    {
      try {
      
        if(_implValue > _maxInclusiveCFacetDouble.value()) 
        
        {
          throwFacetViolation(CF_MAXINCLUSIVE);
        }
      }
      catch(DateTimeException& ex) {
        throwFacetViolation(CF_MAXINCLUSIVE, ex.rawMsg());
      }
      catch(XPlus::Exception& ex) {
        throwFacetViolation(CF_MAXINCLUSIVE);
      }
    }
      
    virtual void applyMaxExclusiveCFacet() 
    {
      try {
      
        if(_implValue >= _maxExclusiveCFacetDouble.value()) 
        
        {
          throwFacetViolation(CF_MAXEXCLUSIVE);
        }
      }
      catch(DateTimeException& ex) {
        throwFacetViolation(CF_MAXINCLUSIVE, ex.rawMsg());
      }
      catch(XPlus::Exception& ex) {
        throwFacetViolation(CF_MAXEXCLUSIVE);
      }
    }
      
    virtual void applyMinInclusiveCFacet() 
    {
      try {
      
        if(_implValue < _minInclusiveCFacetDouble.value()) 
        
        {
          throwFacetViolation(CF_MININCLUSIVE);
        }
      }
      catch(DateTimeException& ex) {
        throwFacetViolation(CF_MAXINCLUSIVE, ex.rawMsg());
      }
      catch(XPlus::Exception& ex) {
        throwFacetViolation(CF_MININCLUSIVE);
      }
    }
      
    virtual void applyMinExclusiveCFacet() 
    {
      try {
      
        if(_implValue <= _minExclusiveCFacetDouble.value()) 
        
        {
          throwFacetViolation(CF_MINEXCLUSIVE);
        }
      }
      catch(DateTimeException& ex) {
        throwFacetViolation(CF_MAXINCLUSIVE, ex.rawMsg());
      }
      catch(XPlus::Exception& ex) {
        throwFacetViolation(CF_MINEXCLUSIVE);
      }
    }
      
    inline virtual void setTypedValue() {
      
      _implValue = fromString<double>(_value);
        
    }

    double _implValue;
      
  };
  

  /// class for simpleType with restriction on base
  class bt_duration : public XMLSchema::Types::anySimpleType
  {
  public:
    /// constructor  
    bt_duration(AnyTypeCreateArgs args)
    
        : anySimpleType(args, PD_DURATION)
        
    {
    
      _whiteSpaceCFacet.stringValue("collapse");
      _whiteSpaceCFacet.fixed(true);
      this->allowedCFacets( CF_NONE  | CF_PATTERN | CF_ENUMERATION | CF_WHITESPACE | CF_MAXINCLUSIVE | CF_MAXEXCLUSIVE | CF_MININCLUSIVE | CF_MINEXCLUSIVE );
    
      this->appliedCFacets( appliedCFacets() | CF_WHITESPACE );
    }
    
    inline XPlus::Duration value() {
      return _implValue;
    }
    inline void value(XPlus::Duration val) {
      
      string strVal = typeToString(val);
        
      anySimpleType::stringValue(strVal);
      //_implValue = val;
    }
      
    virtual inline DOMString sampleValue() {
      return anySimpleType::generateSample(Sampler::durationSamples);
    }
    
    static DOM::DOMString typeToString(XPlus::Duration val) {
      return DateTimeUtils::formatXsdDuration(val); 
    }

    static XPlus::Duration stringToType(DOM::DOMString strVal) {
       return DateTimeUtils::parseXsdDuration(strVal);
    }
        
  protected:
    
    virtual void applyMaxInclusiveCFacet() 
    {
      try {
      Duration& valueAsBound = dynamic_cast<Duration&>(_implValue);
        if(valueAsBound > _maxInclusiveCFacetDuration.value()) 
        
        {
          throwFacetViolation(CF_MAXINCLUSIVE);
        }
      }
      catch(DateTimeException& ex) {
        throwFacetViolation(CF_MAXINCLUSIVE, ex.rawMsg());
      }
      catch(XPlus::Exception& ex) {
        throwFacetViolation(CF_MAXINCLUSIVE);
      }
    }
      
    virtual void applyMaxExclusiveCFacet() 
    {
      try {
      Duration& valueAsBound = dynamic_cast<Duration&>(_implValue);
        if(valueAsBound >= _maxExclusiveCFacetDuration.value()) 
        
        {
          throwFacetViolation(CF_MAXEXCLUSIVE);
        }
      }
      catch(DateTimeException& ex) {
        throwFacetViolation(CF_MAXINCLUSIVE, ex.rawMsg());
      }
      catch(XPlus::Exception& ex) {
        throwFacetViolation(CF_MAXEXCLUSIVE);
      }
    }
      
    virtual void applyMinInclusiveCFacet() 
    {
      try {
      Duration& valueAsBound = dynamic_cast<Duration&>(_implValue);
        if(valueAsBound < _minInclusiveCFacetDuration.value()) 
        
        {
          throwFacetViolation(CF_MININCLUSIVE);
        }
      }
      catch(DateTimeException& ex) {
        throwFacetViolation(CF_MAXINCLUSIVE, ex.rawMsg());
      }
      catch(XPlus::Exception& ex) {
        throwFacetViolation(CF_MININCLUSIVE);
      }
    }
      
    virtual void applyMinExclusiveCFacet() 
    {
      try {
      Duration& valueAsBound = dynamic_cast<Duration&>(_implValue);
        if(valueAsBound <= _minExclusiveCFacetDuration.value()) 
        
        {
          throwFacetViolation(CF_MINEXCLUSIVE);
        }
      }
      catch(DateTimeException& ex) {
        throwFacetViolation(CF_MAXINCLUSIVE, ex.rawMsg());
      }
      catch(XPlus::Exception& ex) {
        throwFacetViolation(CF_MINEXCLUSIVE);
      }
    }
      
    inline virtual void setTypedValue() {
      
      _implValue = stringToType(_value);
        
    }

    XPlus::Duration _implValue;
      
  };
  

  /// class for simpleType with restriction on base
  class bt_dateTime : public XMLSchema::Types::anySimpleType
  {
  public:
    /// constructor  
    bt_dateTime(AnyTypeCreateArgs args)
    
        : anySimpleType(args, PD_DATETIME)
        
    {
    

      vector<DOMString> values;
    
      values.push_back("\\-?\\d{4,}\\-(0[1-9]|10|11|12)\\-((0[1-9])|([1-2][0-9])|(3[0-1]))T(([0-1][0-9])|(2[0-4])):[0-5][0-9]:[0-5][0-9](\\.\\d+)?(Z|([+\\-]\\d\\d:\\d\\d))?");
    
      _patternCFacet.value(values);
      
      _whiteSpaceCFacet.stringValue("collapse");
      _whiteSpaceCFacet.fixed(true);
      this->allowedCFacets( CF_NONE  | CF_PATTERN | CF_ENUMERATION | CF_WHITESPACE | CF_MAXINCLUSIVE | CF_MAXEXCLUSIVE | CF_MININCLUSIVE | CF_MINEXCLUSIVE );
    
      this->appliedCFacets( appliedCFacets() | CF_PATTERN| CF_WHITESPACE );
    }
    
    inline XPlus::DateTime value() {
      return _implValue;
    }
    inline void value(XPlus::DateTime val) {
      
      string strVal = typeToString(val);
        
      anySimpleType::stringValue(strVal);
      //_implValue = val;
    }
      
    virtual inline DOMString sampleValue() {
      return anySimpleType::generateSample(Sampler::dateTimeSamples);
    }
    
    static DOM::DOMString typeToString(XPlus::DateTime val) {
      return DateTimeUtils::formatISO8601DateTime(val); 
    }

    static XPlus::DateTime stringToType(DOM::DOMString strVal) {
       return DateTimeUtils::parseISO8601DateTime(strVal);
    }
        
  protected:
    
    virtual void applyMaxInclusiveCFacet() 
    {
      try {
      DateTime& valueAsBound = dynamic_cast<DateTime&>(_implValue);
        if(valueAsBound > _maxInclusiveCFacetDateTime.value()) 
        
        {
          throwFacetViolation(CF_MAXINCLUSIVE);
        }
      }
      catch(DateTimeException& ex) {
        throwFacetViolation(CF_MAXINCLUSIVE, ex.rawMsg());
      }
      catch(XPlus::Exception& ex) {
        throwFacetViolation(CF_MAXINCLUSIVE);
      }
    }
      
    virtual void applyMaxExclusiveCFacet() 
    {
      try {
      DateTime& valueAsBound = dynamic_cast<DateTime&>(_implValue);
        if(valueAsBound >= _maxExclusiveCFacetDateTime.value()) 
        
        {
          throwFacetViolation(CF_MAXEXCLUSIVE);
        }
      }
      catch(DateTimeException& ex) {
        throwFacetViolation(CF_MAXINCLUSIVE, ex.rawMsg());
      }
      catch(XPlus::Exception& ex) {
        throwFacetViolation(CF_MAXEXCLUSIVE);
      }
    }
      
    virtual void applyMinInclusiveCFacet() 
    {
      try {
      DateTime& valueAsBound = dynamic_cast<DateTime&>(_implValue);
        if(valueAsBound < _minInclusiveCFacetDateTime.value()) 
        
        {
          throwFacetViolation(CF_MININCLUSIVE);
        }
      }
      catch(DateTimeException& ex) {
        throwFacetViolation(CF_MAXINCLUSIVE, ex.rawMsg());
      }
      catch(XPlus::Exception& ex) {
        throwFacetViolation(CF_MININCLUSIVE);
      }
    }
      
    virtual void applyMinExclusiveCFacet() 
    {
      try {
      DateTime& valueAsBound = dynamic_cast<DateTime&>(_implValue);
        if(valueAsBound <= _minExclusiveCFacetDateTime.value()) 
        
        {
          throwFacetViolation(CF_MINEXCLUSIVE);
        }
      }
      catch(DateTimeException& ex) {
        throwFacetViolation(CF_MAXINCLUSIVE, ex.rawMsg());
      }
      catch(XPlus::Exception& ex) {
        throwFacetViolation(CF_MINEXCLUSIVE);
      }
    }
      
    inline virtual void setTypedValue() {
      
      _implValue = stringToType(_value);
        
    }

    XPlus::DateTime _implValue;
      
  };
  

  /// class for simpleType with restriction on base
  class bt_time : public XMLSchema::Types::anySimpleType
  {
  public:
    /// constructor  
    bt_time(AnyTypeCreateArgs args)
    
        : anySimpleType(args, PD_TIME)
        
    {
    
      _whiteSpaceCFacet.stringValue("collapse");
      _whiteSpaceCFacet.fixed(true);
      this->allowedCFacets( CF_NONE  | CF_PATTERN | CF_ENUMERATION | CF_WHITESPACE | CF_MAXINCLUSIVE | CF_MAXEXCLUSIVE | CF_MININCLUSIVE | CF_MINEXCLUSIVE );
    
      this->appliedCFacets( appliedCFacets() | CF_WHITESPACE );
    }
    
    inline XPlus::Time value() {
      return _implValue;
    }
    inline void value(XPlus::Time val) {
      
      string strVal = typeToString(val);
        
      anySimpleType::stringValue(strVal);
      //_implValue = val;
    }
      
    virtual inline DOMString sampleValue() {
      return anySimpleType::generateSample(Sampler::timeSamples);
    }
    
    static DOM::DOMString typeToString(XPlus::Time val) {
      return DateTimeUtils::formatXsdTime(val); 
    }

    static XPlus::Time stringToType(DOM::DOMString strVal) {
       return DateTimeUtils::parseXsdTime(strVal);
    }
        
  protected:
    
    virtual void applyMaxInclusiveCFacet() 
    {
      try {
      DateTime& valueAsBound = dynamic_cast<DateTime&>(_implValue);
        if(valueAsBound > _maxInclusiveCFacetDateTime.value()) 
        
        {
          throwFacetViolation(CF_MAXINCLUSIVE);
        }
      }
      catch(DateTimeException& ex) {
        throwFacetViolation(CF_MAXINCLUSIVE, ex.rawMsg());
      }
      catch(XPlus::Exception& ex) {
        throwFacetViolation(CF_MAXINCLUSIVE);
      }
    }
      
    virtual void applyMaxExclusiveCFacet() 
    {
      try {
      DateTime& valueAsBound = dynamic_cast<DateTime&>(_implValue);
        if(valueAsBound >= _maxExclusiveCFacetDateTime.value()) 
        
        {
          throwFacetViolation(CF_MAXEXCLUSIVE);
        }
      }
      catch(DateTimeException& ex) {
        throwFacetViolation(CF_MAXINCLUSIVE, ex.rawMsg());
      }
      catch(XPlus::Exception& ex) {
        throwFacetViolation(CF_MAXEXCLUSIVE);
      }
    }
      
    virtual void applyMinInclusiveCFacet() 
    {
      try {
      DateTime& valueAsBound = dynamic_cast<DateTime&>(_implValue);
        if(valueAsBound < _minInclusiveCFacetDateTime.value()) 
        
        {
          throwFacetViolation(CF_MININCLUSIVE);
        }
      }
      catch(DateTimeException& ex) {
        throwFacetViolation(CF_MAXINCLUSIVE, ex.rawMsg());
      }
      catch(XPlus::Exception& ex) {
        throwFacetViolation(CF_MININCLUSIVE);
      }
    }
      
    virtual void applyMinExclusiveCFacet() 
    {
      try {
      DateTime& valueAsBound = dynamic_cast<DateTime&>(_implValue);
        if(valueAsBound <= _minExclusiveCFacetDateTime.value()) 
        
        {
          throwFacetViolation(CF_MINEXCLUSIVE);
        }
      }
      catch(DateTimeException& ex) {
        throwFacetViolation(CF_MAXINCLUSIVE, ex.rawMsg());
      }
      catch(XPlus::Exception& ex) {
        throwFacetViolation(CF_MINEXCLUSIVE);
      }
    }
      
    inline virtual void setTypedValue() {
      
      _implValue = stringToType(_value);
        
    }

    XPlus::Time _implValue;
      
  };
  

  /// class for simpleType with restriction on base
  class bt_date : public XMLSchema::Types::anySimpleType
  {
  public:
    /// constructor  
    bt_date(AnyTypeCreateArgs args)
    
        : anySimpleType(args, PD_DATE)
        
    {
    
      _whiteSpaceCFacet.stringValue("collapse");
      _whiteSpaceCFacet.fixed(true);
      this->allowedCFacets( CF_NONE  | CF_PATTERN | CF_ENUMERATION | CF_WHITESPACE | CF_MAXINCLUSIVE | CF_MAXEXCLUSIVE | CF_MININCLUSIVE | CF_MINEXCLUSIVE );
    
      this->appliedCFacets( appliedCFacets() | CF_WHITESPACE );
    }
    
    inline XPlus::Date value() {
      return _implValue;
    }
    inline void value(XPlus::Date val) {
      
      string strVal = typeToString(val);
        
      anySimpleType::stringValue(strVal);
      //_implValue = val;
    }
      
    virtual inline DOMString sampleValue() {
      return anySimpleType::generateSample(Sampler::dateSamples);
    }
    
    static DOM::DOMString typeToString(XPlus::Date val) {
      return DateTimeUtils::formatXsdDate(val); 
    }

    static XPlus::Date stringToType(DOM::DOMString strVal) {
       return DateTimeUtils::parseXsdDate(strVal);
    }
        
  protected:
    
    virtual void applyMaxInclusiveCFacet() 
    {
      try {
      DateTime& valueAsBound = dynamic_cast<DateTime&>(_implValue);
        if(valueAsBound > _maxInclusiveCFacetDateTime.value()) 
        
        {
          throwFacetViolation(CF_MAXINCLUSIVE);
        }
      }
      catch(DateTimeException& ex) {
        throwFacetViolation(CF_MAXINCLUSIVE, ex.rawMsg());
      }
      catch(XPlus::Exception& ex) {
        throwFacetViolation(CF_MAXINCLUSIVE);
      }
    }
      
    virtual void applyMaxExclusiveCFacet() 
    {
      try {
      DateTime& valueAsBound = dynamic_cast<DateTime&>(_implValue);
        if(valueAsBound >= _maxExclusiveCFacetDateTime.value()) 
        
        {
          throwFacetViolation(CF_MAXEXCLUSIVE);
        }
      }
      catch(DateTimeException& ex) {
        throwFacetViolation(CF_MAXINCLUSIVE, ex.rawMsg());
      }
      catch(XPlus::Exception& ex) {
        throwFacetViolation(CF_MAXEXCLUSIVE);
      }
    }
      
    virtual void applyMinInclusiveCFacet() 
    {
      try {
      DateTime& valueAsBound = dynamic_cast<DateTime&>(_implValue);
        if(valueAsBound < _minInclusiveCFacetDateTime.value()) 
        
        {
          throwFacetViolation(CF_MININCLUSIVE);
        }
      }
      catch(DateTimeException& ex) {
        throwFacetViolation(CF_MAXINCLUSIVE, ex.rawMsg());
      }
      catch(XPlus::Exception& ex) {
        throwFacetViolation(CF_MININCLUSIVE);
      }
    }
      
    virtual void applyMinExclusiveCFacet() 
    {
      try {
      DateTime& valueAsBound = dynamic_cast<DateTime&>(_implValue);
        if(valueAsBound <= _minExclusiveCFacetDateTime.value()) 
        
        {
          throwFacetViolation(CF_MINEXCLUSIVE);
        }
      }
      catch(DateTimeException& ex) {
        throwFacetViolation(CF_MAXINCLUSIVE, ex.rawMsg());
      }
      catch(XPlus::Exception& ex) {
        throwFacetViolation(CF_MINEXCLUSIVE);
      }
    }
      
    inline virtual void setTypedValue() {
      
      _implValue = stringToType(_value);
        
    }

    XPlus::Date _implValue;
      
  };
  

  /// class for simpleType with restriction on base
  class bt_gYearMonth : public XMLSchema::Types::anySimpleType
  {
  public:
    /// constructor  
    bt_gYearMonth(AnyTypeCreateArgs args)
    
        : anySimpleType(args, PD_GYEARMONTH)
        
    {
    
      _whiteSpaceCFacet.stringValue("collapse");
      _whiteSpaceCFacet.fixed(true);
      this->allowedCFacets( CF_NONE  | CF_PATTERN | CF_ENUMERATION | CF_WHITESPACE | CF_MAXINCLUSIVE | CF_MAXEXCLUSIVE | CF_MININCLUSIVE | CF_MINEXCLUSIVE );
    
      this->appliedCFacets( appliedCFacets() | CF_WHITESPACE );
    }
    
    inline XPlus::YearMonth value() {
      return _implValue;
    }
    inline void value(XPlus::YearMonth val) {
      
      string strVal = typeToString(val);
        
      anySimpleType::stringValue(strVal);
      //_implValue = val;
    }
      
    virtual inline DOMString sampleValue() {
      return anySimpleType::generateSample(Sampler::gYearMonthSamples);
    }
    
    static DOM::DOMString typeToString(XPlus::YearMonth val) {
      return DateTimeUtils::formatXsdYearMonth(val); 
    }

    static XPlus::YearMonth stringToType(DOM::DOMString strVal) {
       return DateTimeUtils::parseXsdYearMonth(strVal);
    }
        
  protected:
    
    virtual void applyMaxInclusiveCFacet() 
    {
      try {
      DateTime& valueAsBound = dynamic_cast<DateTime&>(_implValue);
        if(valueAsBound > _maxInclusiveCFacetDateTime.value()) 
        
        {
          throwFacetViolation(CF_MAXINCLUSIVE);
        }
      }
      catch(DateTimeException& ex) {
        throwFacetViolation(CF_MAXINCLUSIVE, ex.rawMsg());
      }
      catch(XPlus::Exception& ex) {
        throwFacetViolation(CF_MAXINCLUSIVE);
      }
    }
      
    virtual void applyMaxExclusiveCFacet() 
    {
      try {
      DateTime& valueAsBound = dynamic_cast<DateTime&>(_implValue);
        if(valueAsBound >= _maxExclusiveCFacetDateTime.value()) 
        
        {
          throwFacetViolation(CF_MAXEXCLUSIVE);
        }
      }
      catch(DateTimeException& ex) {
        throwFacetViolation(CF_MAXINCLUSIVE, ex.rawMsg());
      }
      catch(XPlus::Exception& ex) {
        throwFacetViolation(CF_MAXEXCLUSIVE);
      }
    }
      
    virtual void applyMinInclusiveCFacet() 
    {
      try {
      DateTime& valueAsBound = dynamic_cast<DateTime&>(_implValue);
        if(valueAsBound < _minInclusiveCFacetDateTime.value()) 
        
        {
          throwFacetViolation(CF_MININCLUSIVE);
        }
      }
      catch(DateTimeException& ex) {
        throwFacetViolation(CF_MAXINCLUSIVE, ex.rawMsg());
      }
      catch(XPlus::Exception& ex) {
        throwFacetViolation(CF_MININCLUSIVE);
      }
    }
      
    virtual void applyMinExclusiveCFacet() 
    {
      try {
      DateTime& valueAsBound = dynamic_cast<DateTime&>(_implValue);
        if(valueAsBound <= _minExclusiveCFacetDateTime.value()) 
        
        {
          throwFacetViolation(CF_MINEXCLUSIVE);
        }
      }
      catch(DateTimeException& ex) {
        throwFacetViolation(CF_MAXINCLUSIVE, ex.rawMsg());
      }
      catch(XPlus::Exception& ex) {
        throwFacetViolation(CF_MINEXCLUSIVE);
      }
    }
      
    inline virtual void setTypedValue() {
      
      _implValue = stringToType(_value);
        
    }

    XPlus::YearMonth _implValue;
      
  };
  

  /// class for simpleType with restriction on base
  class bt_gYear : public XMLSchema::Types::anySimpleType
  {
  public:
    /// constructor  
    bt_gYear(AnyTypeCreateArgs args)
    
        : anySimpleType(args, PD_GYEAR)
        
    {
    
      _whiteSpaceCFacet.stringValue("collapse");
      _whiteSpaceCFacet.fixed(true);
      this->allowedCFacets( CF_NONE  | CF_PATTERN | CF_ENUMERATION | CF_WHITESPACE | CF_MAXINCLUSIVE | CF_MAXEXCLUSIVE | CF_MININCLUSIVE | CF_MINEXCLUSIVE );
    
      this->appliedCFacets( appliedCFacets() | CF_WHITESPACE );
    }
    
    inline XPlus::Year value() {
      return _implValue;
    }
    inline void value(XPlus::Year val) {
      
      string strVal = typeToString(val);
        
      anySimpleType::stringValue(strVal);
      //_implValue = val;
    }
      
    virtual inline DOMString sampleValue() {
      return anySimpleType::generateSample(Sampler::gYearSamples);
    }
    
    static DOM::DOMString typeToString(XPlus::Year val) {
      int year = val.year();
      return XPlus::toString<int>(year); 
    }

    static XPlus::Year stringToType(DOM::DOMString strVal) {
       return Year(XPlus::fromString<int>(strVal));
    }
        
  protected:
    
    virtual void applyMaxInclusiveCFacet() 
    {
      try {
      DateTime& valueAsBound = dynamic_cast<DateTime&>(_implValue);
        if(valueAsBound > _maxInclusiveCFacetDateTime.value()) 
        
        {
          throwFacetViolation(CF_MAXINCLUSIVE);
        }
      }
      catch(DateTimeException& ex) {
        throwFacetViolation(CF_MAXINCLUSIVE, ex.rawMsg());
      }
      catch(XPlus::Exception& ex) {
        throwFacetViolation(CF_MAXINCLUSIVE);
      }
    }
      
    virtual void applyMaxExclusiveCFacet() 
    {
      try {
      DateTime& valueAsBound = dynamic_cast<DateTime&>(_implValue);
        if(valueAsBound >= _maxExclusiveCFacetDateTime.value()) 
        
        {
          throwFacetViolation(CF_MAXEXCLUSIVE);
        }
      }
      catch(DateTimeException& ex) {
        throwFacetViolation(CF_MAXINCLUSIVE, ex.rawMsg());
      }
      catch(XPlus::Exception& ex) {
        throwFacetViolation(CF_MAXEXCLUSIVE);
      }
    }
      
    virtual void applyMinInclusiveCFacet() 
    {
      try {
      DateTime& valueAsBound = dynamic_cast<DateTime&>(_implValue);
        if(valueAsBound < _minInclusiveCFacetDateTime.value()) 
        
        {
          throwFacetViolation(CF_MININCLUSIVE);
        }
      }
      catch(DateTimeException& ex) {
        throwFacetViolation(CF_MAXINCLUSIVE, ex.rawMsg());
      }
      catch(XPlus::Exception& ex) {
        throwFacetViolation(CF_MININCLUSIVE);
      }
    }
      
    virtual void applyMinExclusiveCFacet() 
    {
      try {
      DateTime& valueAsBound = dynamic_cast<DateTime&>(_implValue);
        if(valueAsBound <= _minExclusiveCFacetDateTime.value()) 
        
        {
          throwFacetViolation(CF_MINEXCLUSIVE);
        }
      }
      catch(DateTimeException& ex) {
        throwFacetViolation(CF_MAXINCLUSIVE, ex.rawMsg());
      }
      catch(XPlus::Exception& ex) {
        throwFacetViolation(CF_MINEXCLUSIVE);
      }
    }
      
    inline virtual void setTypedValue() {
      
      _implValue = stringToType(_value);
        
    }

    XPlus::Year _implValue;
      
  };
  

  /// class for simpleType with restriction on base
  class bt_gMonthDay : public XMLSchema::Types::anySimpleType
  {
  public:
    /// constructor  
    bt_gMonthDay(AnyTypeCreateArgs args)
    
        : anySimpleType(args, PD_GMONTHDAY)
        
    {
    
      _whiteSpaceCFacet.stringValue("collapse");
      _whiteSpaceCFacet.fixed(true);
      this->allowedCFacets( CF_NONE  | CF_PATTERN | CF_ENUMERATION | CF_WHITESPACE | CF_MAXINCLUSIVE | CF_MAXEXCLUSIVE | CF_MININCLUSIVE | CF_MINEXCLUSIVE );
    
      this->appliedCFacets( appliedCFacets() | CF_WHITESPACE );
    }
    
    inline XPlus::MonthDay value() {
      return _implValue;
    }
    inline void value(XPlus::MonthDay val) {
      
      string strVal = typeToString(val);
        
      anySimpleType::stringValue(strVal);
      //_implValue = val;
    }
      
    virtual inline DOMString sampleValue() {
      return anySimpleType::generateSample(Sampler::gMonthDaySamples);
    }
    
    static DOM::DOMString typeToString(XPlus::MonthDay val) {
      return DateTimeUtils::formatXsdMonthDay(val); 
    }

    static XPlus::MonthDay stringToType(DOM::DOMString strVal) {
       return DateTimeUtils::parseXsdMonthDay(strVal);
    }
        
  protected:
    
    virtual void applyMaxInclusiveCFacet() 
    {
      try {
      DateTime& valueAsBound = dynamic_cast<DateTime&>(_implValue);
        if(valueAsBound > _maxInclusiveCFacetDateTime.value()) 
        
        {
          throwFacetViolation(CF_MAXINCLUSIVE);
        }
      }
      catch(DateTimeException& ex) {
        throwFacetViolation(CF_MAXINCLUSIVE, ex.rawMsg());
      }
      catch(XPlus::Exception& ex) {
        throwFacetViolation(CF_MAXINCLUSIVE);
      }
    }
      
    virtual void applyMaxExclusiveCFacet() 
    {
      try {
      DateTime& valueAsBound = dynamic_cast<DateTime&>(_implValue);
        if(valueAsBound >= _maxExclusiveCFacetDateTime.value()) 
        
        {
          throwFacetViolation(CF_MAXEXCLUSIVE);
        }
      }
      catch(DateTimeException& ex) {
        throwFacetViolation(CF_MAXINCLUSIVE, ex.rawMsg());
      }
      catch(XPlus::Exception& ex) {
        throwFacetViolation(CF_MAXEXCLUSIVE);
      }
    }
      
    virtual void applyMinInclusiveCFacet() 
    {
      try {
      DateTime& valueAsBound = dynamic_cast<DateTime&>(_implValue);
        if(valueAsBound < _minInclusiveCFacetDateTime.value()) 
        
        {
          throwFacetViolation(CF_MININCLUSIVE);
        }
      }
      catch(DateTimeException& ex) {
        throwFacetViolation(CF_MAXINCLUSIVE, ex.rawMsg());
      }
      catch(XPlus::Exception& ex) {
        throwFacetViolation(CF_MININCLUSIVE);
      }
    }
      
    virtual void applyMinExclusiveCFacet() 
    {
      try {
      DateTime& valueAsBound = dynamic_cast<DateTime&>(_implValue);
        if(valueAsBound <= _minExclusiveCFacetDateTime.value()) 
        
        {
          throwFacetViolation(CF_MINEXCLUSIVE);
        }
      }
      catch(DateTimeException& ex) {
        throwFacetViolation(CF_MAXINCLUSIVE, ex.rawMsg());
      }
      catch(XPlus::Exception& ex) {
        throwFacetViolation(CF_MINEXCLUSIVE);
      }
    }
      
    inline virtual void setTypedValue() {
      
      _implValue = stringToType(_value);
        
    }

    XPlus::MonthDay _implValue;
      
  };
  

  /// class for simpleType with restriction on base
  class bt_gDay : public XMLSchema::Types::anySimpleType
  {
  public:
    /// constructor  
    bt_gDay(AnyTypeCreateArgs args)
    
        : anySimpleType(args, PD_GDAY)
        
    {
    
      _whiteSpaceCFacet.stringValue("collapse");
      _whiteSpaceCFacet.fixed(true);
      this->allowedCFacets( CF_NONE  | CF_PATTERN | CF_ENUMERATION | CF_WHITESPACE | CF_MAXINCLUSIVE | CF_MAXEXCLUSIVE | CF_MININCLUSIVE | CF_MINEXCLUSIVE );
    
      this->appliedCFacets( appliedCFacets() | CF_WHITESPACE );
    }
    
    inline XPlus::Day value() {
      return _implValue;
    }
    inline void value(XPlus::Day val) {
      
      string strVal = typeToString(val);
        
      anySimpleType::stringValue(strVal);
      //_implValue = val;
    }
      
    virtual inline DOMString sampleValue() {
      return anySimpleType::generateSample(Sampler::gDaySamples);
    }
    
    static DOM::DOMString typeToString(XPlus::Day val) {
      return DateTimeUtils::formatXsdDay(val); 
    }

    static XPlus::Day stringToType(DOM::DOMString strVal) {
       return DateTimeUtils::parseXsdDay(strVal);
    }
        
  protected:
    
    virtual void applyMaxInclusiveCFacet() 
    {
      try {
      DateTime& valueAsBound = dynamic_cast<DateTime&>(_implValue);
        if(valueAsBound > _maxInclusiveCFacetDateTime.value()) 
        
        {
          throwFacetViolation(CF_MAXINCLUSIVE);
        }
      }
      catch(DateTimeException& ex) {
        throwFacetViolation(CF_MAXINCLUSIVE, ex.rawMsg());
      }
      catch(XPlus::Exception& ex) {
        throwFacetViolation(CF_MAXINCLUSIVE);
      }
    }
      
    virtual void applyMaxExclusiveCFacet() 
    {
      try {
      DateTime& valueAsBound = dynamic_cast<DateTime&>(_implValue);
        if(valueAsBound >= _maxExclusiveCFacetDateTime.value()) 
        
        {
          throwFacetViolation(CF_MAXEXCLUSIVE);
        }
      }
      catch(DateTimeException& ex) {
        throwFacetViolation(CF_MAXINCLUSIVE, ex.rawMsg());
      }
      catch(XPlus::Exception& ex) {
        throwFacetViolation(CF_MAXEXCLUSIVE);
      }
    }
      
    virtual void applyMinInclusiveCFacet() 
    {
      try {
      DateTime& valueAsBound = dynamic_cast<DateTime&>(_implValue);
        if(valueAsBound < _minInclusiveCFacetDateTime.value()) 
        
        {
          throwFacetViolation(CF_MININCLUSIVE);
        }
      }
      catch(DateTimeException& ex) {
        throwFacetViolation(CF_MAXINCLUSIVE, ex.rawMsg());
      }
      catch(XPlus::Exception& ex) {
        throwFacetViolation(CF_MININCLUSIVE);
      }
    }
      
    virtual void applyMinExclusiveCFacet() 
    {
      try {
      DateTime& valueAsBound = dynamic_cast<DateTime&>(_implValue);
        if(valueAsBound <= _minExclusiveCFacetDateTime.value()) 
        
        {
          throwFacetViolation(CF_MINEXCLUSIVE);
        }
      }
      catch(DateTimeException& ex) {
        throwFacetViolation(CF_MAXINCLUSIVE, ex.rawMsg());
      }
      catch(XPlus::Exception& ex) {
        throwFacetViolation(CF_MINEXCLUSIVE);
      }
    }
      
    inline virtual void setTypedValue() {
      
      _implValue = stringToType(_value);
        
    }

    XPlus::Day _implValue;
      
  };
  

  /// class for simpleType with restriction on base
  class bt_gMonth : public XMLSchema::Types::anySimpleType
  {
  public:
    /// constructor  
    bt_gMonth(AnyTypeCreateArgs args)
    
        : anySimpleType(args, PD_GMONTH)
        
    {
    
      _whiteSpaceCFacet.stringValue("collapse");
      _whiteSpaceCFacet.fixed(true);
      this->allowedCFacets( CF_NONE  | CF_PATTERN | CF_ENUMERATION | CF_WHITESPACE | CF_MAXINCLUSIVE | CF_MAXEXCLUSIVE | CF_MININCLUSIVE | CF_MINEXCLUSIVE );
    
      this->appliedCFacets( appliedCFacets() | CF_WHITESPACE );
    }
    
    inline XPlus::Month value() {
      return _implValue;
    }
    inline void value(XPlus::Month val) {
      
      string strVal = typeToString(val);
        
      anySimpleType::stringValue(strVal);
      //_implValue = val;
    }
      
    virtual inline DOMString sampleValue() {
      return anySimpleType::generateSample(Sampler::gMonthSamples);
    }
    
    static DOM::DOMString typeToString(XPlus::Month val) {
      return DateTimeUtils::formatXsdMonth(val); 
    }

    static XPlus::Month stringToType(DOM::DOMString strVal) {
       return DateTimeUtils::parseXsdMonth(strVal);
    }
        
  protected:
    
    virtual void applyMaxInclusiveCFacet() 
    {
      try {
      DateTime& valueAsBound = dynamic_cast<DateTime&>(_implValue);
        if(valueAsBound > _maxInclusiveCFacetDateTime.value()) 
        
        {
          throwFacetViolation(CF_MAXINCLUSIVE);
        }
      }
      catch(DateTimeException& ex) {
        throwFacetViolation(CF_MAXINCLUSIVE, ex.rawMsg());
      }
      catch(XPlus::Exception& ex) {
        throwFacetViolation(CF_MAXINCLUSIVE);
      }
    }
      
    virtual void applyMaxExclusiveCFacet() 
    {
      try {
      DateTime& valueAsBound = dynamic_cast<DateTime&>(_implValue);
        if(valueAsBound >= _maxExclusiveCFacetDateTime.value()) 
        
        {
          throwFacetViolation(CF_MAXEXCLUSIVE);
        }
      }
      catch(DateTimeException& ex) {
        throwFacetViolation(CF_MAXINCLUSIVE, ex.rawMsg());
      }
      catch(XPlus::Exception& ex) {
        throwFacetViolation(CF_MAXEXCLUSIVE);
      }
    }
      
    virtual void applyMinInclusiveCFacet() 
    {
      try {
      DateTime& valueAsBound = dynamic_cast<DateTime&>(_implValue);
        if(valueAsBound < _minInclusiveCFacetDateTime.value()) 
        
        {
          throwFacetViolation(CF_MININCLUSIVE);
        }
      }
      catch(DateTimeException& ex) {
        throwFacetViolation(CF_MAXINCLUSIVE, ex.rawMsg());
      }
      catch(XPlus::Exception& ex) {
        throwFacetViolation(CF_MININCLUSIVE);
      }
    }
      
    virtual void applyMinExclusiveCFacet() 
    {
      try {
      DateTime& valueAsBound = dynamic_cast<DateTime&>(_implValue);
        if(valueAsBound <= _minExclusiveCFacetDateTime.value()) 
        
        {
          throwFacetViolation(CF_MINEXCLUSIVE);
        }
      }
      catch(DateTimeException& ex) {
        throwFacetViolation(CF_MAXINCLUSIVE, ex.rawMsg());
      }
      catch(XPlus::Exception& ex) {
        throwFacetViolation(CF_MINEXCLUSIVE);
      }
    }
      
    inline virtual void setTypedValue() {
      
      _implValue = stringToType(_value);
        
    }

    XPlus::Month _implValue;
      
  };
  

  /// class for simpleType with restriction on base
  class bt_hexBinary : public XMLSchema::Types::anySimpleType
  {
  public:
    /// constructor  
    bt_hexBinary(AnyTypeCreateArgs args)
    
        : anySimpleType(args, PD_HEXBINARY)
        
    {
    

      vector<DOMString> values;
    
      values.push_back("([0-9a-fA-F]{2})*");
    
      _patternCFacet.value(values);
      
      _whiteSpaceCFacet.stringValue("collapse");
      _whiteSpaceCFacet.fixed(true);
      this->allowedCFacets( CF_NONE  | CF_LENGTH | CF_MINLENGTH | CF_MAXLENGTH | CF_PATTERN | CF_ENUMERATION | CF_WHITESPACE );
    
      this->appliedCFacets( appliedCFacets() | CF_WHITESPACE| CF_PATTERN );
    }
    
    inline DOM::DOMString value() {
      return _implValue;
    }
    inline void value(DOM::DOMString val) {
      
      string strVal = toString<DOM::DOMString>(val);
        
      anySimpleType::stringValue(strVal);
      //_implValue = val;
    }
      
    virtual inline DOMString sampleValue() {
      return anySimpleType::generateSample(Sampler::hexBinarySamples);
    }
    
    virtual unsigned int lengthFacet() {
      return ceil(stringValue().length()/2);
    }
        
  protected:
    
    inline virtual void setTypedValue() {
      
      _implValue = fromString<DOM::DOMString>(_value);
        
    }

    DOM::DOMString _implValue;
      
  };
  

  /// class for simpleType with restriction on base
  class bt_base64Binary : public XMLSchema::Types::anySimpleType
  {
  public:
    /// constructor  
    bt_base64Binary(AnyTypeCreateArgs args)
    
        : anySimpleType(args, PD_BASE64BINARY)
        
    {
    

      vector<DOMString> values;
    
      values.push_back("((([A-Za-z0-9+/]\\s?){4})*(([A-Za-z0-9+/]\\s?){3}[A-Za-z0-9+/]|([A-Za-z0-9+/]\\s?){2}[AEIMQUYcgkosw048]\\s?=|[A-Za-z0-9+/]\\s?[AQgw]\\s?=\\s?=))?");
    
      _patternCFacet.value(values);
      
      _whiteSpaceCFacet.stringValue("collapse");
      _whiteSpaceCFacet.fixed(true);
      this->allowedCFacets( CF_NONE  | CF_LENGTH | CF_MINLENGTH | CF_MAXLENGTH | CF_PATTERN | CF_ENUMERATION | CF_WHITESPACE );
    
      this->appliedCFacets( appliedCFacets() | CF_WHITESPACE| CF_PATTERN );
    }
    
    inline DOM::DOMString value() {
      return _implValue;
    }
    inline void value(DOM::DOMString val) {
      
      string strVal = toString<DOM::DOMString>(val);
        
      anySimpleType::stringValue(strVal);
      //_implValue = val;
    }
      
    virtual inline DOMString sampleValue() {
      return anySimpleType::generateSample(Sampler::base64BinarySamples);
    }
    
    // pseudocode from w3c XSD1.1 spec    
    //    lex2   := killwhitespace(lexform)  
    //    lex3   := strip_equals(lex2)
    //    length := floor (length(lex3) * 3 / 4)  
    virtual unsigned int lengthFacet() 
    {
      DOMString lex = stringValue();
      lex.removeChars(UTF8FNS::isWhiteSpaceChar);
      
      // length of non-whitespace chars should be multiple of 4
      assert(lex.length()%4 == 0);
      lex.trimRight(UTF8FNS::isEqualChar);

      return floor(lex.length()*3/4);
    }
        
  protected:
    
    inline virtual void setTypedValue() {
      
      _implValue = fromString<DOM::DOMString>(_value);
        
    }

    DOM::DOMString _implValue;
      
  };
  

  /// class for simpleType with restriction on base
  class bt_anyURI : public XMLSchema::Types::anySimpleType
  {
  public:
    /// constructor  
    bt_anyURI(AnyTypeCreateArgs args)
    
        : anySimpleType(args, PD_ANYURI)
        
    {
    
      _whiteSpaceCFacet.stringValue("collapse");
      _whiteSpaceCFacet.fixed(true);
      this->allowedCFacets( CF_NONE  | CF_LENGTH | CF_MINLENGTH | CF_MAXLENGTH | CF_PATTERN | CF_ENUMERATION | CF_WHITESPACE );
    
      this->appliedCFacets( appliedCFacets() | CF_WHITESPACE );
    }
    
    inline DOM::DOMString value() {
      return _implValue;
    }
    inline void value(DOM::DOMString val) {
      
      string strVal = toString<DOM::DOMString>(val);
        
      anySimpleType::stringValue(strVal);
      //_implValue = val;
    }
      
    virtual inline DOMString sampleValue() {
      return anySimpleType::generateSample(Sampler::anyURISamples);
    }
    
  protected:
    
    inline virtual void setTypedValue() {
      
      _implValue = fromString<DOM::DOMString>(_value);
        
    }

    DOM::DOMString _implValue;
      
  };
  

  /// class for simpleType with restriction on base
  class bt_QName : public XMLSchema::Types::anySimpleType
  {
  public:
    /// constructor  
    bt_QName(AnyTypeCreateArgs args)
    
        : anySimpleType(args, PD_QNAME)
        
    {
    
      _whiteSpaceCFacet.stringValue("collapse");
      _whiteSpaceCFacet.fixed(true);
      this->allowedCFacets( CF_NONE  | CF_LENGTH | CF_MINLENGTH | CF_MAXLENGTH | CF_PATTERN | CF_ENUMERATION | CF_WHITESPACE );
    
      this->appliedCFacets( appliedCFacets() | CF_WHITESPACE );
    }
    
    inline DOM::DOMString value() {
      return _implValue;
    }
    inline void value(DOM::DOMString val) {
      
      string strVal = toString<DOM::DOMString>(val);
        
      anySimpleType::stringValue(strVal);
      //_implValue = val;
    }
      
    virtual inline DOMString sampleValue() {
      return anySimpleType::generateSample(Sampler::QNameSamples);
    }
    
  protected:
    
    inline virtual void setTypedValue() {
      
      _implValue = fromString<DOM::DOMString>(_value);
        
    }

    DOM::DOMString _implValue;
      
  };
  

  /// class for simpleType with restriction on base
  class bt_NOTATION : public XMLSchema::Types::anySimpleType
  {
  public:
    /// constructor  
    bt_NOTATION(AnyTypeCreateArgs args)
    
        : anySimpleType(args, PD_NOTATION)
        
    {
    
      _whiteSpaceCFacet.stringValue("collapse");
      _whiteSpaceCFacet.fixed(true);
      this->allowedCFacets( CF_NONE  | CF_LENGTH | CF_MINLENGTH | CF_MAXLENGTH | CF_PATTERN | CF_ENUMERATION | CF_WHITESPACE );
    
      this->appliedCFacets( appliedCFacets() | CF_WHITESPACE );
    }
    
    inline DOM::DOMString value() {
      return _implValue;
    }
    inline void value(DOM::DOMString val) {
      
      string strVal = toString<DOM::DOMString>(val);
        
      anySimpleType::stringValue(strVal);
      //_implValue = val;
    }
      
    virtual inline DOMString sampleValue() {
      return anySimpleType::generateSample(Sampler::NOTATIONSamples);
    }
    
  protected:
    
    inline virtual void setTypedValue() {
      
      _implValue = fromString<DOM::DOMString>(_value);
        
    }

    DOM::DOMString _implValue;
      
  };
  

  /// class for simpleType with restriction on base
  class bt_normalizedString : public XMLSchema::Types::bt_string
  {
  public:
    /// constructor  
    bt_normalizedString(AnyTypeCreateArgs args)
    
        : bt_string(args)
      
    {
    
      _builtinDerivedType = XMLSchema::BD_NORMALIZEDSTRING;  
    
      _whiteSpaceCFacet.stringValue("replace");
      
      this->appliedCFacets( appliedCFacets() | CF_WHITESPACE );
    }
    
    virtual inline DOMString sampleValue() {
      return anySimpleType::generateSample(Sampler::normalizedStringSamples);
    }
    
  protected:
    
  };
  

  /// class for simpleType with restriction on base
  class bt_token : public XMLSchema::Types::bt_normalizedString
  {
  public:
    /// constructor  
    bt_token(AnyTypeCreateArgs args)
    
        : bt_normalizedString(args)
      
    {
    
      _builtinDerivedType = XMLSchema::BD_TOKEN;  
    
      _whiteSpaceCFacet.stringValue("collapse");
      
      this->appliedCFacets( appliedCFacets() | CF_WHITESPACE );
    }
    
    virtual inline DOMString sampleValue() {
      return anySimpleType::generateSample(Sampler::tokenSamples);
    }
    
  protected:
    
  };
  

  /// class for simpleType with restriction on base
  class bt_language : public XMLSchema::Types::bt_token
  {
  public:
    /// constructor  
    bt_language(AnyTypeCreateArgs args)
    
        : bt_token(args)
      
    {
    
      _builtinDerivedType = XMLSchema::BD_LANGUAGE;  
    

      vector<DOMString> values;
    
      values.push_back("[a-zA-Z]{1,8}(-[a-zA-Z0-9]{1,8})*");
    
      _patternCFacet.value(values);
      
      this->appliedCFacets( appliedCFacets() | CF_PATTERN );
    }
    
    virtual inline DOMString sampleValue() {
      return anySimpleType::generateSample(Sampler::languageSamples);
    }
    
  protected:
    
  };
  

  /// class for simpleType with restriction on base
  class bt_Name : public XMLSchema::Types::bt_token
  {
  public:
    /// constructor  
    bt_Name(AnyTypeCreateArgs args)
    
        : bt_token(args)
      
    {
    
      _builtinDerivedType = XMLSchema::BD_NAME;  
    
      this->appliedCFacets( appliedCFacets()  );
    }
    
    virtual inline DOMString sampleValue() {
      return anySimpleType::generateSample(Sampler::NameSamples);
    }
    
  protected:
    
  };
  

  /// class for simpleType with restriction on base
  class bt_NCName : public XMLSchema::Types::bt_Name
  {
  public:
    /// constructor  
    bt_NCName(AnyTypeCreateArgs args)
    
        : bt_Name(args)
      
    {
    
      _builtinDerivedType = XMLSchema::BD_NCNAME;  
    

      vector<DOMString> values;
    
      values.push_back("([A-Z]|_|[a-z]|[\\xC0-\\xD6]|[\\xD8-\\xF6]|[\\xF8-\\xFF])(:|[A-Z]|_|[a-z]|[\\xC0-\\xD6]|[\\xD8-\\xF6]|[\\xF8-\\xFF]|\\-|\\.|[0-9]|\\xB7)*");
    
      _patternCFacet.value(values);
      
      this->appliedCFacets( appliedCFacets() | CF_PATTERN );
    }
    
    virtual inline DOMString sampleValue() {
      return anySimpleType::generateSample(Sampler::NCNameSamples);
    }
    
  protected:
    
  };
  

  /// class for simpleType with restriction on base
  class bt_IDREF : public XMLSchema::Types::bt_NCName
  {
  public:
    /// constructor  
    bt_IDREF(AnyTypeCreateArgs args)
    
        : bt_NCName(args)
      
    {
    
      _builtinDerivedType = XMLSchema::BD_IDREF;  
    
      this->appliedCFacets( appliedCFacets()  );
    }
    
    virtual inline DOMString sampleValue() {
      return anySimpleType::generateSample(Sampler::IDREFSamples);
    }
    
  protected:
    
  };

  typedef XMLSchema::Types::SimpleTypeListTmpl<XMLSchema::Types::bt_IDREF> IDREFS_restriction_simpleType;
    

  /// class for simpleType with restriction on simpleType
  class bt_IDREFS : public IDREFS_restriction_simpleType
  {
  public:
    /// constructor  
    bt_IDREFS(AnyTypeCreateArgs args):
      IDREFS_restriction_simpleType(args)
    {
      
      _builtinDerivedType = XMLSchema::BD_IDREFS;  
      
      _minLengthCFacet.stringValue("1");
      
      this->appliedCFacets( appliedCFacets() | CF_MINLENGTH );
    }
    
    virtual inline DOMString sampleValue() {
      return anySimpleType::generateSample(Sampler::IDREFSSamples);
    }
    
  };

  

  /// class for simpleType with restriction on base
  class bt_ENTITY : public XMLSchema::Types::bt_NCName
  {
  public:
    /// constructor  
    bt_ENTITY(AnyTypeCreateArgs args)
    
        : bt_NCName(args)
      
    {
    
      _builtinDerivedType = XMLSchema::BD_ENTITY;  
    
      this->appliedCFacets( appliedCFacets()  );
    }
    
    virtual inline DOMString sampleValue() {
      return anySimpleType::generateSample(Sampler::ENTITYSamples);
    }
    
  protected:
    
  };

  typedef XMLSchema::Types::SimpleTypeListTmpl<XMLSchema::Types::bt_ENTITY> ENTITIES_restriction_simpleType;
    

  /// class for simpleType with restriction on simpleType
  class bt_ENTITIES : public ENTITIES_restriction_simpleType
  {
  public:
    /// constructor  
    bt_ENTITIES(AnyTypeCreateArgs args):
      ENTITIES_restriction_simpleType(args)
    {
      
      _builtinDerivedType = XMLSchema::BD_ENTITIES;  
      
      _minLengthCFacet.stringValue("1");
      
      this->appliedCFacets( appliedCFacets() | CF_MINLENGTH );
    }
    
    virtual inline DOMString sampleValue() {
      return anySimpleType::generateSample(Sampler::ENTITIESSamples);
    }
    
  };

  

  /// class for simpleType with restriction on base
  class bt_NMTOKEN : public XMLSchema::Types::bt_token
  {
  public:
    /// constructor  
    bt_NMTOKEN(AnyTypeCreateArgs args)
    
        : bt_token(args)
      
    {
    
      _builtinDerivedType = XMLSchema::BD_NMTOKEN;  
    

      vector<DOMString> values;
    
      values.push_back("(:|[A-Z]|_|[a-z]|[\\xC0-\\xD6]|[\\xD8-\\xF6]|[\\xF8-\\xFF]|\\-|\\.|[0-9]|\\xB7)+");
    
      _patternCFacet.value(values);
      
      this->appliedCFacets( appliedCFacets() | CF_PATTERN );
    }
    
    virtual inline DOMString sampleValue() {
      return anySimpleType::generateSample(Sampler::NMTOKENSamples);
    }
    
  protected:
    
  };

  typedef XMLSchema::Types::SimpleTypeListTmpl<XMLSchema::Types::bt_NMTOKEN> NMTOKENS_restriction_simpleType;
    

  /// class for simpleType with restriction on simpleType
  class bt_NMTOKENS : public NMTOKENS_restriction_simpleType
  {
  public:
    /// constructor  
    bt_NMTOKENS(AnyTypeCreateArgs args):
      NMTOKENS_restriction_simpleType(args)
    {
      
      _builtinDerivedType = XMLSchema::BD_NMTOKENS;  
      
      _minLengthCFacet.stringValue("1");
      
      this->appliedCFacets( appliedCFacets() | CF_MINLENGTH );
    }
    
    virtual inline DOMString sampleValue() {
      return anySimpleType::generateSample(Sampler::NMTOKENSSamples);
    }
    
  };

  

  /// class for simpleType with restriction on base
  class bt_ID : public XMLSchema::Types::bt_NCName
  {
  public:
    /// constructor  
    bt_ID(AnyTypeCreateArgs args)
    
        : bt_NCName(args)
      
    {
    
      _builtinDerivedType = XMLSchema::BD_ID;  
    
      this->appliedCFacets( appliedCFacets()  );
    }
    
    virtual inline DOMString sampleValue() {
      return anySimpleType::generateSample(Sampler::IDSamples);
    }
    
  protected:
    
  };
  

  /// class for simpleType with restriction on base
  class bt_integer : public XMLSchema::Types::bt_decimal
  {
  public:
    /// constructor  
    bt_integer(AnyTypeCreateArgs args)
    
        : bt_decimal(args)
      
    {
    
      _builtinDerivedType = XMLSchema::BD_INTEGER;  
    

      vector<DOMString> values;
    
      values.push_back("[\\-+]?[0-9]+");
    
      _patternCFacet.value(values);
      
      _fractionDigitsCFacet.stringValue("0");
      _fractionDigitsCFacet.fixed(true);
      this->appliedCFacets( appliedCFacets() | CF_FRACTIONDIGITS| CF_PATTERN );
    }
    
    virtual inline DOMString sampleValue() {
      return anySimpleType::generateSample(Sampler::integerSamples);
    }
    
  protected:
    
  };
  

  /// class for simpleType with restriction on base
  class bt_nonPositiveInteger : public XMLSchema::Types::bt_integer
  {
  public:
    /// constructor  
    bt_nonPositiveInteger(AnyTypeCreateArgs args)
    
        : bt_integer(args)
      
    {
    
      _builtinDerivedType = XMLSchema::BD_NONPOSITIVEINTEGER;  
    
      maxInclusiveCFacet().stringValue("0");
          
      this->appliedCFacets( appliedCFacets() | CF_MAXINCLUSIVE );
    }
    
    virtual inline DOMString sampleValue() {
      return anySimpleType::generateSample(Sampler::nonPositiveIntegerSamples);
    }
    
  protected:
    
  };
  

  /// class for simpleType with restriction on base
  class bt_negativeInteger : public XMLSchema::Types::bt_nonPositiveInteger
  {
  public:
    /// constructor  
    bt_negativeInteger(AnyTypeCreateArgs args)
    
        : bt_nonPositiveInteger(args)
      
    {
    
      _builtinDerivedType = XMLSchema::BD_NEGATIVEINTEGER;  
    
      maxInclusiveCFacet().stringValue("-1");
          
      this->appliedCFacets( appliedCFacets() | CF_MAXINCLUSIVE );
    }
    
    virtual inline DOMString sampleValue() {
      return anySimpleType::generateSample(Sampler::negativeIntegerSamples);
    }
    
  protected:
    
  };
  

  /// class for simpleType with restriction on base
  class bt_long : public XMLSchema::Types::bt_integer
  {
  public:
    /// constructor  
    bt_long(AnyTypeCreateArgs args)
    
        : bt_integer(args)
      
    {
    
      _builtinDerivedType = XMLSchema::BD_LONG;  
    
      _minInclusiveCFacetDouble.value(XPlus::XSD_LONG_MININCL);
          
      _maxInclusiveCFacetDouble.value(XPlus::XSD_LONG_MAXINCL);
          
      this->appliedCFacets( appliedCFacets() | CF_MININCLUSIVE| CF_MAXINCLUSIVE );
    }
    
    virtual inline DOMString sampleValue() {
      return anySimpleType::generateSample(Sampler::longSamples);
    }
    
  protected:
    
  };
  

  /// class for simpleType with restriction on base
  class bt_int : public XMLSchema::Types::bt_long
  {
  public:
    /// constructor  
    bt_int(AnyTypeCreateArgs args)
    
        : bt_long(args)
      
    {
    
      _builtinDerivedType = XMLSchema::BD_INT;  
    
      _minInclusiveCFacetDouble.value(XPlus::XSD_INT_MININCL);
          
      _maxInclusiveCFacetDouble.value(XPlus::XSD_INT_MAXINCL);
          
      this->appliedCFacets( appliedCFacets() | CF_MININCLUSIVE| CF_MAXINCLUSIVE );
    }
    
    virtual inline DOMString sampleValue() {
      return anySimpleType::generateSample(Sampler::intSamples);
    }
    
  protected:
    
  };
  

  /// class for simpleType with restriction on base
  class bt_short : public XMLSchema::Types::bt_int
  {
  public:
    /// constructor  
    bt_short(AnyTypeCreateArgs args)
    
        : bt_int(args)
      
    {
    
      _builtinDerivedType = XMLSchema::BD_SHORT;  
    
      minInclusiveCFacet().stringValue("-32768");
          
      maxInclusiveCFacet().stringValue("32767");
          
      this->appliedCFacets( appliedCFacets() | CF_MININCLUSIVE| CF_MAXINCLUSIVE );
    }
    
    virtual inline DOMString sampleValue() {
      return anySimpleType::generateSample(Sampler::shortSamples);
    }
    
  protected:
    
  };
  

  /// class for simpleType with restriction on base
  class bt_byte : public XMLSchema::Types::bt_short
  {
  public:
    /// constructor  
    bt_byte(AnyTypeCreateArgs args)
    
        : bt_short(args)
      
    {
    
      _builtinDerivedType = XMLSchema::BD_BYTE;  
    
      minInclusiveCFacet().stringValue("-128");
          
      maxInclusiveCFacet().stringValue("127");
          
      this->appliedCFacets( appliedCFacets() | CF_MININCLUSIVE| CF_MAXINCLUSIVE );
    }
    
    virtual inline DOMString sampleValue() {
      return anySimpleType::generateSample(Sampler::byteSamples);
    }
    
  protected:
    
  };
  

  /// class for simpleType with restriction on base
  class bt_nonNegativeInteger : public XMLSchema::Types::bt_integer
  {
  public:
    /// constructor  
    bt_nonNegativeInteger(AnyTypeCreateArgs args)
    
        : bt_integer(args)
      
    {
    
      _builtinDerivedType = XMLSchema::BD_NONNEGATIVEINTEGER;  
    
      minInclusiveCFacet().stringValue("0");
          
      this->appliedCFacets( appliedCFacets() | CF_MININCLUSIVE );
    }
    
    virtual inline DOMString sampleValue() {
      return anySimpleType::generateSample(Sampler::nonNegativeIntegerSamples);
    }
    
  protected:
    
  };
  

  /// class for simpleType with restriction on base
  class bt_unsignedLong : public XMLSchema::Types::bt_nonNegativeInteger
  {
  public:
    /// constructor  
    bt_unsignedLong(AnyTypeCreateArgs args)
    
        : bt_nonNegativeInteger(args)
      
    {
    
      _builtinDerivedType = XMLSchema::BD_UNSIGNEDLONG;  
    
      _maxInclusiveCFacetDouble.value(XPlus::XSD_ULONG_MAXINCL);
          
      this->appliedCFacets( appliedCFacets() | CF_MAXINCLUSIVE );
    }
    
    virtual inline DOMString sampleValue() {
      return anySimpleType::generateSample(Sampler::unsignedLongSamples);
    }
    
  protected:
    
  };
  

  /// class for simpleType with restriction on base
  class bt_unsignedInt : public XMLSchema::Types::bt_unsignedLong
  {
  public:
    /// constructor  
    bt_unsignedInt(AnyTypeCreateArgs args)
    
        : bt_unsignedLong(args)
      
    {
    
      _builtinDerivedType = XMLSchema::BD_UNSIGNEDINT;  
    
      _maxInclusiveCFacetDouble.value(XPlus::XSD_UINT_MAXINCL);
          
      this->appliedCFacets( appliedCFacets() | CF_MAXINCLUSIVE );
    }
    
    virtual inline DOMString sampleValue() {
      return anySimpleType::generateSample(Sampler::unsignedIntSamples);
    }
    
  protected:
    
  };
  

  /// class for simpleType with restriction on base
  class bt_unsignedShort : public XMLSchema::Types::bt_unsignedInt
  {
  public:
    /// constructor  
    bt_unsignedShort(AnyTypeCreateArgs args)
    
        : bt_unsignedInt(args)
      
    {
    
      _builtinDerivedType = XMLSchema::BD_UNSIGNEDSHORT;  
    
      maxInclusiveCFacet().stringValue("65535");
          
      this->appliedCFacets( appliedCFacets() | CF_MAXINCLUSIVE );
    }
    
    virtual inline DOMString sampleValue() {
      return anySimpleType::generateSample(Sampler::unsignedShortSamples);
    }
    
  protected:
    
  };
  

  /// class for simpleType with restriction on base
  class bt_unsignedByte : public XMLSchema::Types::bt_unsignedShort
  {
  public:
    /// constructor  
    bt_unsignedByte(AnyTypeCreateArgs args)
    
        : bt_unsignedShort(args)
      
    {
    
      _builtinDerivedType = XMLSchema::BD_UNSIGNEDBYTE;  
    
      maxInclusiveCFacet().stringValue("255");
          
      this->appliedCFacets( appliedCFacets() | CF_MAXINCLUSIVE );
    }
    
    virtual inline DOMString sampleValue() {
      return anySimpleType::generateSample(Sampler::unsignedByteSamples);
    }
    
  protected:
    
  };
  

  /// class for simpleType with restriction on base
  class bt_positiveInteger : public XMLSchema::Types::bt_nonNegativeInteger
  {
  public:
    /// constructor  
    bt_positiveInteger(AnyTypeCreateArgs args)
    
        : bt_nonNegativeInteger(args)
      
    {
    
      _builtinDerivedType = XMLSchema::BD_POSITIVEINTEGER;  
    
      minInclusiveCFacet().stringValue("1");
          
      this->appliedCFacets( appliedCFacets() | CF_MININCLUSIVE );
    }
    
    virtual inline DOMString sampleValue() {
      return anySimpleType::generateSample(Sampler::positiveIntegerSamples);
    }
    
  protected:
    
  };
  
} // end namespace Types


} // end namespace XMLSchema
    

#endif
  