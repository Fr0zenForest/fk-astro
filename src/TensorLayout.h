#pragma once
#include <cstddef>
#include <vector>

namespace fka
{
// MLMultiArray strides are in elements, not bytes. A rank-4 image has a
// singleton batch at index zero; rank-2 is a monochrome HxW array.
inline size_t ImageOffset( const std::vector<size_t>& strides, bool chw, int c, int y, int x )
{
   if ( strides.size() == 2 )
      return size_t( y ) * strides[0] + size_t( x ) * strides[1];
   const size_t i = strides.size() - 3;
   return chw ? size_t( c ) * strides[i] + size_t( y ) * strides[i+1] + size_t( x ) * strides[i+2]
              : size_t( y ) * strides[i] + size_t( x ) * strides[i+1] + size_t( c ) * strides[i+2];
}

inline size_t LinearOffset( size_t index, const std::vector<size_t>& shape, const std::vector<size_t>& strides )
{
   size_t result = 0;
   for ( size_t d = shape.size(); d-- > 0; )
   {
      result += ( index % shape[d] ) * strides[d];
      index /= shape[d];
   }
   return result;
}
} // namespace fka
