#pragma once

/*
 * fk-astro — FKAstroInference process definition
 */

#include <pcl/ProcessImplementation.h>
#include "FKAstroInterface.h"
#include <pcl/MetaProcess.h>
#include <pcl/MetaParameter.h>
#include <pcl/String.h>


class FKAstroInterface;
extern FKAstroInterface* TheFKAstroInterface;
class FKAstroInferenceProcess;
extern FKAstroInferenceProcess* TheFKAstroInferenceProcess;

// forward declarations
class FKModelPathParameter;
class FKSharpenStarsParameter;
class FKAdjustStarHalosParameter;
class FKSharpenNonstellarParameter;
class FKTileOverlapParameter;
class FKDeviceParameter;
class FKVerboseParameter;

extern FKModelPathParameter*    TheFKModelPathParameter;
extern FKSharpenStarsParameter* TheFKSharpenStarsParameter;
extern FKAdjustStarHalosParameter* TheFKAdjustStarHalosParameter;
extern FKSharpenNonstellarParameter* TheFKSharpenNonstellarParameter;
class FKAutoPSFParameter;
class FKPSFDiameterParameter;
extern FKAutoPSFParameter* TheFKAutoPSFParameter;
extern FKPSFDiameterParameter* TheFKPSFDiameterParameter;
class FKCorrectOnlyParameter;
extern FKCorrectOnlyParameter* TheFKCorrectOnlyParameter;
class FKNormSchemeParameter;
class FKNormWpParameter;
class FKNormMtParameter;
class FKNormAtParameter;
class FKNormTbParameter;
class FKReflectPadParameter;
extern FKNormSchemeParameter* TheFKNormSchemeParameter;
extern FKNormWpParameter* TheFKNormWpParameter;
extern FKNormMtParameter* TheFKNormMtParameter;
extern FKNormAtParameter* TheFKNormAtParameter;
extern FKNormTbParameter* TheFKNormTbParameter;
extern FKReflectPadParameter* TheFKReflectPadParameter;
extern FKTileOverlapParameter*  TheFKTileOverlapParameter;
extern FKDeviceParameter*       TheFKDeviceParameter;
extern FKVerboseParameter*      TheFKVerboseParameter;

// ---------------------------------------------------------------------------
// Process parameter metaclasses
// ---------------------------------------------------------------------------

class FKModelPathParameter : public pcl::MetaString
{
public:
   FKModelPathParameter( pcl::MetaProcess* P ) : pcl::MetaString( P )
   {
      TheFKModelPathParameter = this;
   }
   pcl::IsoString Id() const override
   {
      return "modelPath";
   }
};





class FKSharpenStarsParameter : public pcl::MetaFloat
{
public:
   FKSharpenStarsParameter( pcl::MetaProcess* P ) : pcl::MetaFloat( P )
   {
      TheFKSharpenStarsParameter = this;
   }
   pcl::IsoString Id() const override
   {
      return "sharpenStars";
   }
};

class FKAdjustStarHalosParameter : public pcl::MetaFloat
{
public:
   FKAdjustStarHalosParameter( pcl::MetaProcess* P ) : pcl::MetaFloat( P )
   {
      TheFKAdjustStarHalosParameter = this;
   }
   pcl::IsoString Id() const override
   {
      return "adjustStarHalos";
   }
};

class FKSharpenNonstellarParameter : public pcl::MetaFloat
{
public:
   FKSharpenNonstellarParameter( pcl::MetaProcess* P ) : pcl::MetaFloat( P )
   {
      TheFKSharpenNonstellarParameter = this;
   }
   pcl::IsoString Id() const override
   {
      return "sharpenNonstellar";
   }
};

class FKAutoPSFParameter : public pcl::MetaBoolean
{
public:
   FKAutoPSFParameter( pcl::MetaProcess* P ) : pcl::MetaBoolean( P )
   {
      TheFKAutoPSFParameter = this;
   }
   pcl::IsoString Id() const override
   {
      return "autoPSF";
   }
};

class FKPSFDiameterParameter : public pcl::MetaFloat
{
public:
   FKPSFDiameterParameter( pcl::MetaProcess* P ) : pcl::MetaFloat( P )
   {
      TheFKPSFDiameterParameter = this;
   }
   pcl::IsoString Id() const override
   {
      return "psfDiameter";
   }
};

class FKTileOverlapParameter : public pcl::MetaFloat
{
public:
   FKTileOverlapParameter( pcl::MetaProcess* P ) : pcl::MetaFloat( P )
   {
      TheFKTileOverlapParameter = this;
   }
   pcl::IsoString Id() const override
   {
      return "tileOverlap";
   }
};

class FKNormSchemeParameter : public pcl::MetaEnumeration
{
public:
   FKNormSchemeParameter( pcl::MetaProcess* P ) : pcl::MetaEnumeration( P )
   {
      TheFKNormSchemeParameter = this;
   }
   pcl::IsoString Id() const override
   {
      return "normScheme";
   }
   pcl::size_type NumberOfElements() const override
   {
      return 4;
   }
   pcl::IsoString ElementId( pcl::size_type idx ) const override
   {
      switch ( idx )
      {
      case 0:  return "None";
      case 1:  return "Arcsinh";
      case 2:  return "Mtf";
      default: return "MtfArcsinh";
      }
   }
   int ElementValue( pcl::size_type idx ) const override
   {
      return int( idx );
   }
};

class FKNormWpParameter : public pcl::MetaFloat
{
public:
   FKNormWpParameter( pcl::MetaProcess* P ) : pcl::MetaFloat( P )
   {
      TheFKNormWpParameter = this;
   }
   pcl::IsoString Id() const override
   {
      return "normWp";
   }
};

class FKNormMtParameter : public pcl::MetaFloat
{
public:
   FKNormMtParameter( pcl::MetaProcess* P ) : pcl::MetaFloat( P )
   {
      TheFKNormMtParameter = this;
   }
   pcl::IsoString Id() const override
   {
      return "normMt";
   }
};

class FKNormAtParameter : public pcl::MetaFloat
{
public:
   FKNormAtParameter( pcl::MetaProcess* P ) : pcl::MetaFloat( P )
   {
      TheFKNormAtParameter = this;
   }
   pcl::IsoString Id() const override
   {
      return "normAt";
   }
};

class FKNormTbParameter : public pcl::MetaFloat
{
public:
   FKNormTbParameter( pcl::MetaProcess* P ) : pcl::MetaFloat( P )
   {
      TheFKNormTbParameter = this;
   }
   pcl::IsoString Id() const override
   {
      return "normTb";
   }
};

class FKReflectPadParameter : public pcl::MetaBoolean
{
public:
   FKReflectPadParameter( pcl::MetaProcess* P ) : pcl::MetaBoolean( P )
   {
      TheFKReflectPadParameter = this;
   }
   pcl::IsoString Id() const override
   {
      return "reflectPad";
   }
};

class FKCorrectOnlyParameter : public pcl::MetaBoolean
{
public:
   FKCorrectOnlyParameter( pcl::MetaProcess* P ) : pcl::MetaBoolean( P )
   {
      TheFKCorrectOnlyParameter = this;
   }
   pcl::IsoString Id() const override
   {
      return "correctOnly";
   }
};

class FKDeviceParameter : public pcl::MetaEnumeration
{
public:
   FKDeviceParameter( pcl::MetaProcess* P ) : pcl::MetaEnumeration( P )
   {
      TheFKDeviceParameter = this;
   }
   pcl::IsoString Id() const override
   {
      return "device";
   }
   pcl::size_type NumberOfElements() const override
   {
      return 3;
   }
   pcl::IsoString ElementId( pcl::size_type idx ) const override
   {
      switch ( idx )
      {
      case 0:  return "Auto";
      case 1:  return "GPU";
      default: return "CPU";
      }
   }
   int ElementValue( pcl::size_type idx ) const override
   {
      return int( idx );
   }
};

class FKVerboseParameter : public pcl::MetaBoolean
{
public:
   FKVerboseParameter( pcl::MetaProcess* P ) : pcl::MetaBoolean( P )
   {
      TheFKVerboseParameter = this;
   }
   pcl::IsoString Id() const override
   {
      return "verbose";
   }
};


// ---------------------------------------------------------------------------
// Process metaobject
// ---------------------------------------------------------------------------

class FKAstroInferenceProcess : public pcl::MetaProcess
{
public:

   FKAstroInferenceProcess();

   pcl::IsoString Id() const override;
   pcl::IsoString Aliases() const override;
   pcl::IsoString Categories() const override;
   pcl::IsoString IconImageSVG() const override;

   // View context only: never executable globally (no target image).
   bool CanProcessGlobal() const override
   {
      return false;
   }

   // Custom GUI so the process opens directly when activated.
   pcl::ProcessInterface* DefaultInterface() const override
   {
      return TheFKAstroInterface;
   }

   pcl::ProcessImplementation* Create() const override;
   pcl::ProcessImplementation* Clone( const pcl::ProcessImplementation& ) const override;
};

// ---------------------------------------------------------------------------
// Process instance
// ---------------------------------------------------------------------------

class FKAstroInferenceInstance : public pcl::ProcessImplementation
{
public:

   FKAstroInferenceInstance( const FKAstroInferenceProcess* );

   void Assign( const pcl::ProcessImplementation& ) override;
   bool Validate( pcl::String& info ) override;
   bool CanExecuteOn( const pcl::View& view, pcl::String& whyNot ) const override;
   bool ExecuteOn( pcl::View& view ) override;

   void* LockParameter( const pcl::MetaParameter* p, pcl::size_type tableRow ) override;

   // Process parameters
   pcl::String p_modelPath;
   double      p_sharpenStars      = 0.5;   // [0, 0.7]
   double      p_adjustStarHalos   = 0.0;   // [-0.5, 0.5]
   double      p_sharpenNonstellar = 0.5;   // [0, 1]
   bool        p_autoPSF      = true;
   double      p_psfDiameter  = 0.0;         // [0, 8], used when !p_autoPSF
   double      p_tileOverlap  = 0.2;         // fraction of tile size, [0, 0.5]
   bool        p_correctOnly  = true;
   pcl::pcl_enum p_normScheme = 0;    // 0=none 1=arcsinh 2=mtf 3=mtf+arcsinh
   double      p_normWp       = 0.01;
   double      p_normMt       = 0.25;
   double      p_normAt       = 0.50;
   double      p_normTb       = 0.00;
   bool        p_reflectPad   = true;
   pcl::pcl_enum p_device       = 0;  // 0=Auto 1=GPU 2=CPU
   bool        p_verbose      = true;
};
