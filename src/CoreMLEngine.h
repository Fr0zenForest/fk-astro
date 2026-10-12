#pragma once

/*
 * fk-astro — CoreML inference engine for PixInsight
 *
 * Loads a user-supplied CoreML model (.mlpackage / .mlmodel), discovers its
 * first multi-array input/output, and runs tiled inference.
 *
 * Pure C++ interface; implementation in CoreMLEngine.mm (Objective-C++).
 */

#include <string>

namespace fka
{

struct TensorShape
{
   int c = 0;
   int h = 0;
   int w = 0;

   bool IsValid() const
   {
      return c > 0 && h > 0 && w > 0;
   }

   size_t Size() const
   {
      return size_t( c ) * size_t( h ) * size_t( w );
   }
};

enum ComputeDevice
{
   DeviceAuto = 0,   // ANE + GPU + CPU (CoreML decides)
   DeviceGPU  = 1,   // CPU + GPU (Metal)
   DeviceNE   = 2,   // CPU + ANE (internal fallback level)
   DeviceCPU  = 3    // CPU only
};

class CoreMLEngine
{
public:

   static CoreMLEngine& Get();

   /*
    * Load (or reuse a cached) model. The model is compiled with
    * [MLModel compileModelAtURL:error:] on first use.
    *
    * 'path'   — file system path to .mlpackage or .mlmodel
    * 'device' — one of the ComputeDevice values above
    *
    * Returns true on success; on failure returns false and LastError()
    * carries a diagnostic message.
    */
   bool Load( const std::string& path, int device );

   bool IsLoaded() const
   {
      return m_loaded;
   }

   /*
    * Shape of the model's first multi-array input (typically [C,H,W]).
    * Valid only after a successful Load().
    */
   TensorShape InputShape() const
   {
      return m_inputShape;
   }

   /*
    * Shape of the model's first multi-array output.
    */
   TensorShape OutputShape() const
   {
      return m_outputShape;
   }

   /*
    * True if the model declares a second multi-array input (besides the
    * image), e.g. a parameters tensor as used by two-input inference
    * models. Its element count is ParamsSize().
    */
   bool HasParamsInput() const
   {
      return m_paramsSize > 0;
   }

   size_t ParamsSize() const
   {
      return m_paramsSize;
   }

   /*
    * Run one inference.
    *
    * 'in'  — Input samples, layout CHW: in[c*h*w + y*w + x]
    *         (channel-major, row-major within a channel).
    * 'out' — Output samples, same CHW layout, must provide
    *         OutputShape().Size() floats.
    * 'params' — Optional values for the model's second multi-array input
    *         (parameters tensor), if it has one. Extra elements are
    *         zero-filled; ignored when the model has no params input.
    *
    * The model may declare its tensors as CHW or HWC; the engine normalizes
    * both to CHW transparently.
    *
    * Returns true on success.
    */
   bool Predict( const float* in, float* out, const float* params = nullptr, size_t paramCount = 0 );

   // Internal: called under lock from Predict(); may throw ObjC exceptions,
   // which Predict() converts into LastError() diagnostics.
   bool PredictLocked( const float* in, float* out, const float* params, size_t paramCount );

   // (Re)create the MLModel from the cached compiled URL with the given
   // compute units. Returns true on success.
   bool BuildModel( int device );

   const std::string& LastError() const
   {
      return m_lastError;
   }

   /*
    * The compute device actually in use after automatic fallback
    * (Auto -> GPU -> CPU) caused by prediction failures.
    */
   int EffectiveDevice() const
   {
      return m_effectiveDevice;
   }

   void Unload();

private:

   CoreMLEngine() = default;
   ~CoreMLEngine() = default;

   bool         m_loaded = false;
   std::string  m_loadedPath;
   int          m_device = DeviceAuto;
   TensorShape  m_inputShape;
   TensorShape  m_outputShape;
   std::string  m_lastError;

   void* m_model = nullptr;        // MLModel* (bridged)
   void* m_inputName = nullptr;    // NSString* (bridged)
   void* m_outputName = nullptr;   // NSString* (bridged)
   void* m_paramsName = nullptr;   // NSString* (bridged), second input if any
   size_t m_paramsSize = 0;        // element count of the params tensor
   int   m_paramsRank = 0;         // native rank (e.g. 2 for [1,5])
   int   m_paramsDims[4] = { 1, 1, 1, 1 };

   // compute-unit fallback state
   int         m_requestedDevice = 0;
   int         m_effectiveDevice = 0;
   void*       m_compiledURL = nullptr;   // NSURL* (bridged), cached
   std::string m_compiledPath;
   int   m_inRank = 0;             // native input rank (2, 3 or 4)
   int   m_inDims[4] = { 0, 0, 0, 0 };
   int   m_outRank = 0;
   int   m_outDims[4] = { 0, 0, 0, 0 };
   int   m_inDataType = 0;         // MLMultiArrayDataType of the image input
   int   m_outDataType = 0;        // MLMultiArrayDataType of the image output
   int   m_paramsDataType = 0;     // MLMultiArrayDataType of the params input
   bool  m_inputCHW = true;        // false => model wants/returns HWC
   bool  m_outputCHW = true;
};

} // namespace fka
