// \brief
//	math SSE, reference from UE4
//

#pragma once

#include <cstdint>
// We require SSE2
#include <emmintrin.h>


/**
 *	float4 vector register type, where the first float (X) is stored in the lowest 32 bits, and so on.
 */
typedef __m128	VectorRegister;
typedef __m128i VectorRegisterInt;
typedef __m128d VectorRegisterDouble;

// for an __m128, we need a single set of braces (for clang)
#define DECLARE_VECTOR_REGISTER(X, Y, Z, W) { X, Y, Z, W }

/**
 * @param A0	Selects which element (0-3) from 'A' into 1st slot in the result
 * @param A1	Selects which element (0-3) from 'A' into 2nd slot in the result
 * @param B2	Selects which element (0-3) from 'B' into 3rd slot in the result
 * @param B3	Selects which element (0-3) from 'B' into 4th slot in the result
 */
#define SHUFFLEMASK(A0,A1,B2,B3) ( (A0) | ((A1)<<2) | ((B2)<<4) | ((B3)<<6) )


class VectorRegsiterConstants
{
public:
	static const VectorRegister FloatOne;
	static const VectorRegister FloatZero;
	static const VectorRegister FloatNonFractional;
	static const VectorRegister SignMask;
	static const VectorRegister FloatMinusOne;
	static const VectorRegister FloatOneOver255;
	static const VectorRegister Float255;

	static const VectorRegisterInt IntAllMask;
	static const VectorRegisterInt IntOne;
	static const VectorRegisterInt IntZero;
	static const VectorRegisterInt IntMinusOne;
};

/**
 * Returns a bitwise equivalent vector based on 4 DWORDs.
 * @return		Bitwise equivalent vector with 4 floats
 */
inline VectorRegister MakeVectorRegister(uint32_t X, uint32_t Y, uint32_t Z, uint32_t W)
{
	union { VectorRegister v; VectorRegisterInt i; } Tmp;
	Tmp.i = _mm_setr_epi32(X, Y, Z, W);
	return Tmp.v;
}

/**
 * Returns a vector based on 4 FLOATs.
 * @return		Vector of the 4 FLOATs
 */
inline VectorRegister MakeVectorRegister(float X, float Y, float Z, float W)
{
	return _mm_setr_ps(X, Y, Z, W);
}

/**
* Returns a vector based on 4 int32_t.
* @return		Vector of the 4 int32_t
*/
inline VectorRegisterInt MakeVectorRegisterInt(int32_t X, int32_t Y, int32_t Z, int32_t W)
{
	return _mm_setr_epi32(X, Y, Z, W);
}

/**
 * Returns a vector with all zeros.
 * @return		VectorRegister(0.0f, 0.0f, 0.0f, 0.0f)
 */
inline VectorRegister VectorZero(void)
{
	return _mm_setzero_ps();
}

/**
 * Loads 4 FLOATs from unaligned memory.
 *
 * @param Ptr	Unaligned memory pointer to the 4 FLOATs
 * @return		VectorRegister(Ptr[0], Ptr[1], Ptr[2], Ptr[3])
 */
inline VectorRegister VectorLoad(const void* Ptr)
{
	return _mm_loadu_ps((float*)(Ptr));
}


/**
 * Returns an component from a vector.
 *
 * @param Vec				Vector register
 * @param ComponentIndex	Which component to get, X=0, Y=1, Z=2, W=3
 * @return					The component as a float
 */
inline float VectorGetComponent(VectorRegister Vec, uint32_t ComponentIndex)
{
	return (((float*)&(Vec))[ComponentIndex]);
}


/**
 * Loads 3 FLOATs from unaligned memory and leaves W undefined.
 *
 * @param Ptr	Unaligned memory pointer to the 3 FLOATs
 * @return		VectorRegister(Ptr[0], Ptr[1], Ptr[2], undefined)
 */
#define VectorLoadFloat3( Ptr )			MakeVectorRegister( ((const float*)(Ptr))[0], ((const float*)(Ptr))[1], ((const float*)(Ptr))[2], 0.0f )

 /**
  * Loads 3 FLOATs from unaligned memory and sets W=0.
  *
  * @param Ptr	Unaligned memory pointer to the 3 FLOATs
  * @return		VectorRegister(Ptr[0], Ptr[1], Ptr[2], 0.0f)
  */
#define VectorLoadFloat3_W0( Ptr )		MakeVectorRegister( ((const float*)(Ptr))[0], ((const float*)(Ptr))[1], ((const float*)(Ptr))[2], 0.0f )

  /**
   * Loads 3 FLOATs from unaligned memory and sets W=1.
   *
   * @param Ptr	Unaligned memory pointer to the 3 FLOATs
   * @return		VectorRegister(Ptr[0], Ptr[1], Ptr[2], 1.0f)
   */
#define VectorLoadFloat3_W1( Ptr )		MakeVectorRegister( ((const float*)(Ptr))[0], ((const float*)(Ptr))[1], ((const float*)(Ptr))[2], 1.0f )

   /**
	* Loads 4 FLOATs from aligned memory.
	*
	* @param Ptr	Aligned memory pointer to the 4 FLOATs
	* @return		VectorRegister(Ptr[0], Ptr[1], Ptr[2], Ptr[3])
	*/
#define VectorLoadAligned( Ptr )		_mm_load_ps( (const float*)(Ptr) )

	/**
	 * Loads 1 float from unaligned memory and replicates it to all 4 elements.
	 *
	 * @param Ptr	Unaligned memory pointer to the float
	 * @return		VectorRegister(Ptr[0], Ptr[0], Ptr[0], Ptr[0])
	 */
#define VectorLoadFloat1( Ptr )			_mm_load1_ps( (const float*)(Ptr) )

	 /**
	  * Loads 2 floats from unaligned memory into X and Y and duplicates them in Z and W.
	  *
	  * @param Ptr	Unaligned memory pointer to the floats
	  * @return		VectorRegister(Ptr[0], Ptr[1], Ptr[0], Ptr[1])
	  */
#define VectorLoadFloat2( Ptr )			_mm_castpd_ps(_mm_load1_pd((const double*)(Ptr)))

	  /**
	   * Creates a vector out of three FLOATs and leaves W undefined.
	   *
	   * @param X		1st float component
	   * @param Y		2nd float component
	   * @param Z		3rd float component
	   * @return		VectorRegister(X, Y, Z, undefined)
	   */
#define VectorSetFloat3( X, Y, Z )		MakeVectorRegister( X, Y, Z, 0.0f )

	   /**
		* Propagates passed in float to all registers
		*
	   * @param F		Float to set
			   * @return		VectorRegister(F,F,F,F)
			   */
#define VectorSetFloat1( F )	_mm_set1_ps( F )

			   /**
				* Creates a vector out of four FLOATs.
				*
				* @param X		1st float component
				* @param Y		2nd float component
				* @param Z		3rd float component
				* @param W		4th float component
				* @return		VectorRegister(X, Y, Z, W)
				*/
inline VectorRegister VectorSet(float X, float Y, float Z, float W)
{
	return MakeVectorRegister(X, Y, Z, W);
}

/**
 * Stores a vector to aligned memory.
 *
 * @param Vec	Vector to store
 * @param Ptr	Aligned memory pointer
 */
#define VectorStoreAligned( Vec, Ptr )	_mm_store_ps( (float*)(Ptr), Vec )


 /**
  * Performs non-temporal store of a vector to aligned memory without polluting the caches
  *
  * @param Vec	Vector to store
  * @param Ptr	Aligned memory pointer
  */
#define VectorStoreAlignedStreamed( Vec, Ptr )	_mm_stream_ps( (float*)(Ptr), Vec )

  /**
   * Stores a vector to memory (aligned or unaligned).
   *
   * @param Vec	Vector to store
   * @param Ptr	Memory pointer
   */
inline void VectorStore(const VectorRegister& Vec, void* Ptr)
{
	_mm_storeu_ps((float*)(Ptr), Vec);
}

/**
 * Stores the XYZ components of a vector to unaligned memory.
 *
 * @param Vec	Vector to store XYZ
 * @param Ptr	Unaligned memory pointer
 */
inline void VectorStoreFloat3(const VectorRegister& Vec, void* Ptr)
{
	union { VectorRegister v; float f[4]; } Tmp;
	Tmp.v = Vec;
	float* FloatPtr = (float*)(Ptr);
	FloatPtr[0] = Tmp.f[0];
	FloatPtr[1] = Tmp.f[1];
	FloatPtr[2] = Tmp.f[2];
}

/**
 * Replicates one element into all four elements and returns the new vector.
 *
 * @param Vec			Source vector
 * @param ElementIndex	Index (0-3) of the element to replicate
 * @return				VectorRegister( Vec[ElementIndex], Vec[ElementIndex], Vec[ElementIndex], Vec[ElementIndex] )
 */
#define VectorReplicate( Vec, ElementIndex )	_mm_shuffle_ps( Vec, Vec, SHUFFLEMASK(ElementIndex,ElementIndex,ElementIndex,ElementIndex) )

 /**
  * Returns the absolute value (component-wise).
  *
  * @param Vec			Source vector
  * @return				VectorRegister( abs(Vec.x), abs(Vec.y), abs(Vec.z), abs(Vec.w) )
  */
#define VectorAbs( Vec )				_mm_and_ps(Vec, VectorRegsiterConstants::SignMask)

  /**
   * Returns the negated value (component-wise).
   *
   * @param Vec			Source vector
   * @return				VectorRegister( -Vec.x, -Vec.y, -Vec.z, -Vec.w )
   */
#define VectorNegate( Vec )				_mm_sub_ps(_mm_setzero_ps(),Vec)

   /**
	* Adds two vectors (component-wise) and returns the result.
	*
	* @param Vec1	1st vector
	* @param Vec2	2nd vector
	* @return		VectorRegister( Vec1.x+Vec2.x, Vec1.y+Vec2.y, Vec1.z+Vec2.z, Vec1.w+Vec2.w )
	*/

inline VectorRegister VectorAdd(const VectorRegister& Vec1, const VectorRegister& Vec2)
{
	return _mm_add_ps(Vec1, Vec2);
}

/**
 * Subtracts a vector from another (component-wise) and returns the result.
 *
 * @param Vec1	1st vector
 * @param Vec2	2nd vector
 * @return		VectorRegister( Vec1.x-Vec2.x, Vec1.y-Vec2.y, Vec1.z-Vec2.z, Vec1.w-Vec2.w )
 */
inline VectorRegister VectorSubtract(const VectorRegister& Vec1, const VectorRegister& Vec2)
{
	return _mm_sub_ps(Vec1, Vec2);
}

/**
 * Multiplies two vectors (component-wise) and returns the result.
 *
 * @param Vec1	1st vector
 * @param Vec2	2nd vector
 * @return		VectorRegister( Vec1.x*Vec2.x, Vec1.y*Vec2.y, Vec1.z*Vec2.z, Vec1.w*Vec2.w )
 */
inline VectorRegister VectorMultiply(const VectorRegister& Vec1, const VectorRegister& Vec2)
{
	return _mm_mul_ps(Vec1, Vec2);
}

/**
 * Multiplies two vectors (component-wise), adds in the third vector and returns the result.
 *
 * @param Vec1	1st vector
 * @param Vec2	2nd vector
 * @param Vec3	3rd vector
 * @return		VectorRegister( Vec1.x*Vec2.x + Vec3.x, Vec1.y*Vec2.y + Vec3.y, Vec1.z*Vec2.z + Vec3.z, Vec1.w*Vec2.w + Vec3.w )
 */
#define VectorMultiplyAdd( Vec1, Vec2, Vec3 )	_mm_add_ps( _mm_mul_ps(Vec1, Vec2), Vec3 )

 /**
  * Calculates the dot3 product of two vectors and returns a vector with the result in all 4 components.
  * Only really efficient on Xbox 360.
  *
  * @param Vec1	1st vector
  * @param Vec2	2nd vector
  * @return		d = dot3(Vec1.xyz, Vec2.xyz), VectorRegister( d, d, d, d )
  */
inline VectorRegister VectorDot3(const VectorRegister& Vec1, const VectorRegister& Vec2)
{
	VectorRegister Temp = VectorMultiply(Vec1, Vec2);
	return VectorAdd(VectorReplicate(Temp, 0), VectorAdd(VectorReplicate(Temp, 1), VectorReplicate(Temp, 2)));
}

/**
 * Calculates the dot4 product of two vectors and returns a vector with the result in all 4 components.
 * Only really efficient on Xbox 360.
 *
 * @param Vec1	1st vector
 * @param Vec2	2nd vector
 * @return		d = dot4(Vec1.xyzw, Vec2.xyzw), VectorRegister( d, d, d, d )
 */
inline VectorRegister VectorDot4(const VectorRegister& Vec1, const VectorRegister& Vec2)
{
	VectorRegister Temp1, Temp2;
	Temp1 = VectorMultiply(Vec1, Vec2);
	Temp2 = _mm_shuffle_ps(Temp1, Temp1, SHUFFLEMASK(2, 3, 0, 1));	// (Z,W,X,Y).
	Temp1 = VectorAdd(Temp1, Temp2);								// (X*X + Z*Z, Y*Y + W*W, Z*Z + X*X, W*W + Y*Y)
	Temp2 = _mm_shuffle_ps(Temp1, Temp1, SHUFFLEMASK(1, 2, 3, 0));	// Rotate left 4 bytes (Y,Z,W,X).
	return VectorAdd(Temp1, Temp2);								// (X*X + Z*Z + Y*Y + W*W, Y*Y + W*W + Z*Z + X*X, Z*Z + X*X + W*W + Y*Y, W*W + Y*Y + X*X + Z*Z)
}


/**
 * Creates a four-part mask based on component-wise == compares of the input vectors
 *
 * @param Vec1	1st vector
 * @param Vec2	2nd vector
 * @return		VectorRegister( Vec1.x == Vec2.x ? 0xFFFFFFFF : 0, same for yzw )
 */
#define VectorCompareEQ( Vec1, Vec2 )			_mm_cmpeq_ps( Vec1, Vec2 )

 /**
  * Creates a four-part mask based on component-wise != compares of the input vectors
  *
  * @param Vec1	1st vector
  * @param Vec2	2nd vector
  * @return		VectorRegister( Vec1.x != Vec2.x ? 0xFFFFFFFF : 0, same for yzw )
  */
#define VectorCompareNE( Vec1, Vec2 )			_mm_cmpneq_ps( Vec1, Vec2 )

  /**
   * Creates a four-part mask based on component-wise > compares of the input vectors
   *
   * @param Vec1	1st vector
   * @param Vec2	2nd vector
   * @return		VectorRegister( Vec1.x > Vec2.x ? 0xFFFFFFFF : 0, same for yzw )
   */
#define VectorCompareGT( Vec1, Vec2 )			_mm_cmpgt_ps( Vec1, Vec2 )

   /**
	* Creates a four-part mask based on component-wise >= compares of the input vectors
	*
	* @param Vec1	1st vector
	* @param Vec2	2nd vector
	* @return		VectorRegister( Vec1.x >= Vec2.x ? 0xFFFFFFFF : 0, same for yzw )
	*/
#define VectorCompareGE( Vec1, Vec2 )			_mm_cmpge_ps( Vec1, Vec2 )

	/**
	* Creates a four-part mask based on component-wise < compares of the input vectors
	*
	* @param Vec1	1st vector
	* @param Vec2	2nd vector
	* @return		VectorRegister( Vec1.x < Vec2.x ? 0xFFFFFFFF : 0, same for yzw )
	*/
#define VectorCompareLT( Vec1, Vec2 )			_mm_cmplt_ps( Vec1, Vec2 )

	/**
	* Creates a four-part mask based on component-wise <= compares of the input vectors
	*
	* @param Vec1	1st vector
	* @param Vec2	2nd vector
	* @return		VectorRegister( Vec1.x <= Vec2.x ? 0xFFFFFFFF : 0, same for yzw )
	*/
#define VectorCompareLE( Vec1, Vec2 )			_mm_cmple_ps( Vec1, Vec2 )

	/**
	 * Does a bitwise vector selection based on a mask (e.g., created from VectorCompareXX)
	 *
	 * @param Mask  Mask (when 1: use the corresponding bit from Vec1 otherwise from Vec2)
	 * @param Vec1	1st vector
	 * @param Vec2	2nd vector
	 * @return		VectorRegister( for each bit i: Mask[i] ? Vec1[i] : Vec2[i] )
	 *
	 */

inline VectorRegister VectorSelect(const VectorRegister& Mask, const VectorRegister& Vec1, const VectorRegister& Vec2)
{
	return _mm_xor_ps(Vec2, _mm_and_ps(Mask, _mm_xor_ps(Vec1, Vec2)));
}

/**
 * Combines two vectors using bitwise OR (treating each vector as a 128 bit field)
 *
 * @param Vec1	1st vector
 * @param Vec2	2nd vector
 * @return		VectorRegister( for each bit i: Vec1[i] | Vec2[i] )
 */
#define VectorBitwiseOr(Vec1, Vec2)	_mm_or_ps(Vec1, Vec2)

 /**
  * Combines two vectors using bitwise AND (treating each vector as a 128 bit field)
  *
  * @param Vec1	1st vector
  * @param Vec2	2nd vector
  * @return		VectorRegister( for each bit i: Vec1[i] & Vec2[i] )
  */
#define VectorBitwiseAnd(Vec1, Vec2) _mm_and_ps(Vec1, Vec2)

  /**
   * Combines two vectors using bitwise XOR (treating each vector as a 128 bit field)
   *
   * @param Vec1	1st vector
   * @param Vec2	2nd vector
   * @return		VectorRegister( for each bit i: Vec1[i] ^ Vec2[i] )
   */
#define VectorBitwiseXor(Vec1, Vec2) _mm_xor_ps(Vec1, Vec2)


   /**
	* Calculates the cross product of two vectors (XYZ components). W is set to 0.
	*
	* @param Vec1	1st vector
	* @param Vec2	2nd vector
	* @return		cross(Vec1.xyz, Vec2.xyz). W is set to 0.
	*/
inline VectorRegister VectorCross(const VectorRegister& Vec1, const VectorRegister& Vec2)
{
	VectorRegister A_YZXW = _mm_shuffle_ps(Vec1, Vec1, SHUFFLEMASK(1, 2, 0, 3));
	VectorRegister B_ZXYW = _mm_shuffle_ps(Vec2, Vec2, SHUFFLEMASK(2, 0, 1, 3));
	VectorRegister A_ZXYW = _mm_shuffle_ps(Vec1, Vec1, SHUFFLEMASK(2, 0, 1, 3));
	VectorRegister B_YZXW = _mm_shuffle_ps(Vec2, Vec2, SHUFFLEMASK(1, 2, 0, 3));
	return VectorSubtract(VectorMultiply(A_YZXW, B_ZXYW), VectorMultiply(A_ZXYW, B_YZXW));
}

/**
* Returns an estimate of 1/sqrt(c) for each component of the vector
*
* @param Vector		Vector
* @return			VectorRegister(1/sqrt(t), 1/sqrt(t), 1/sqrt(t), 1/sqrt(t))
*/
#define VectorReciprocalSqrt(Vec)		_mm_rsqrt_ps( Vec )

/**
 * Computes an estimate of the reciprocal of a vector (component-wise) and returns the result.
 *
 * @param Vec	1st vector
 * @return		VectorRegister( (Estimate) 1.0f / Vec.x, (Estimate) 1.0f / Vec.y, (Estimate) 1.0f / Vec.z, (Estimate) 1.0f / Vec.w )
 */
#define VectorReciprocal(Vec)			_mm_rcp_ps(Vec)

 /**
  * Returns the minimum values of two vectors (component-wise).
  *
  * @param Vec1	1st vector
  * @param Vec2	2nd vector
  * @return		VectorRegister( min(Vec1.x,Vec2.x), min(Vec1.y,Vec2.y), min(Vec1.z,Vec2.z), min(Vec1.w,Vec2.w) )
  */
#define VectorMin( Vec1, Vec2 )			_mm_min_ps( Vec1, Vec2 )

  /**
   * Returns the maximum values of two vectors (component-wise).
   *
   * @param Vec1	1st vector
   * @param Vec2	2nd vector
   * @return		VectorRegister( max(Vec1.x,Vec2.x), max(Vec1.y,Vec2.y), max(Vec1.z,Vec2.z), max(Vec1.w,Vec2.w) )
   */
#define VectorMax( Vec1, Vec2 )			_mm_max_ps( Vec1, Vec2 )

   /**
	* Swizzles the 4 components of a vector and returns the result.
	*
	* @param Vec		Source vector
	* @param X			Index for which component to use for X (literal 0-3)
	* @param Y			Index for which component to use for Y (literal 0-3)
	* @param Z			Index for which component to use for Z (literal 0-3)
	* @param W			Index for which component to use for W (literal 0-3)
	* @return			The swizzled vector
	*/
#define VectorSwizzle( Vec, X, Y, Z, W )	_mm_shuffle_ps( Vec, Vec, SHUFFLEMASK(X,Y,Z,W) )

	/**
	 * Creates a vector through selecting two components from each vector via a shuffle mask.
	 *
	 * @param Vec1		Source vector1
	 * @param Vec2		Source vector2
	 * @param X			Index for which component of Vector1 to use for X (literal 0-3)
	 * @param Y			Index for which component of Vector1 to use for Y (literal 0-3)
	 * @param Z			Index for which component of Vector2 to use for Z (literal 0-3)
	 * @param W			Index for which component of Vector2 to use for W (literal 0-3)
	 * @return			The swizzled vector
	 */
#define VectorShuffle( Vec1, Vec2, X, Y, Z, W )	_mm_shuffle_ps( Vec1, Vec2, SHUFFLEMASK(X,Y,Z,W) )

	 /**
	  * Divides two vectors (component-wise) and returns the result.
	  *
	  * @param Vec1	1st vector
	  * @param Vec2	2nd vector
	  * @return		VectorRegister( Vec1.x/Vec2.x, Vec1.y/Vec2.y, Vec1.z/Vec2.z, Vec1.w/Vec2.w )
	  */
inline VectorRegister VectorDivide(const VectorRegister& Vec1, const VectorRegister& Vec2)
{
	return _mm_div_ps(Vec1, Vec2);
}

/**
 * Loads 4 BYTEs from unaligned memory and converts them into 4 FLOATs.
 * IMPORTANT: You need to call VectorResetFloatRegisters() before using scalar FLOATs after you've used this intrinsic!
 *
 * @param Ptr			Unaligned memory pointer to the 4 BYTEs.
 * @return				VectorRegister( float(Ptr[0]), float(Ptr[1]), float(Ptr[2]), float(Ptr[3]) )
 */
 // Looks complex but is really quite straightforward:
 // Load as 32-bit value, unpack 4x unsigned bytes to 4x 16-bit ints, then unpack again into 4x 32-bit ints, then convert to 4x floats
#define VectorLoadByte4( Ptr )			_mm_cvtepi32_ps(_mm_unpacklo_epi16(_mm_unpacklo_epi8(_mm_cvtsi32_si128(*(int32_t*)Ptr), _mm_setzero_si128()), _mm_setzero_si128()))

/**
* Loads 4 signed BYTEs from unaligned memory and converts them into 4 FLOATs.
* IMPORTANT: You need to call VectorResetFloatRegisters() before using scalar FLOATs after you've used this intrinsic!
*
* @param Ptr			Unaligned memory pointer to the 4 BYTEs.
* @return				VectorRegister( float(Ptr[0]), float(Ptr[1]), float(Ptr[2]), float(Ptr[3]) )
*/
// Looks complex but is really quite straightforward:
// Load as 32-bit value, unpack 4x unsigned bytes to 4x 16-bit ints, then unpack again into 4x 32-bit ints, then convert to 4x floats
inline VectorRegister VectorLoadSignedByte4(const void* Ptr)
{
	auto Temp = _mm_unpacklo_epi16(_mm_unpacklo_epi8(_mm_cvtsi32_si128(*(int32_t*)Ptr), _mm_setzero_si128()), _mm_setzero_si128());
	auto Mask = _mm_cmpgt_epi32(Temp, _mm_set1_epi32(127));
	auto Comp = _mm_and_si128(Mask, _mm_set1_epi32(~127));
	return _mm_cvtepi32_ps(_mm_or_si128(Comp, Temp));
}

/**
 * Loads 4 BYTEs from unaligned memory and converts them into 4 FLOATs in reversed order.
 * IMPORTANT: You need to call VectorResetFloatRegisters() before using scalar FLOATs after you've used this intrinsic!
 *
 * @param Ptr			Unaligned memory pointer to the 4 BYTEs.
 * @return				VectorRegister( float(Ptr[3]), float(Ptr[2]), float(Ptr[1]), float(Ptr[0]) )
 */
inline VectorRegister VectorLoadByte4Reverse(void* Ptr)
{
	VectorRegister Temp = VectorLoadByte4(Ptr);
	return _mm_shuffle_ps(Temp, Temp, SHUFFLEMASK(3, 2, 1, 0));
}

/**
 * Converts the 4 FLOATs in the vector to 4 BYTEs, clamped to [0,255], and stores to unaligned memory.
 * IMPORTANT: You need to call VectorResetFloatRegisters() before using scalar FLOATs after you've used this intrinsic!
 *
 * @param Vec			Vector containing 4 FLOATs
 * @param Ptr			Unaligned memory pointer to store the 4 BYTEs.
 */
inline void VectorStoreByte4(const VectorRegister& Vec, void* Ptr)
{
	// Looks complex but is really quite straightforward:
	// Convert 4x floats to 4x 32-bit ints, then pack into 4x 16-bit ints, then into 4x 8-bit unsigned ints, then store as a 32-bit value
	*(int32_t*)Ptr = _mm_cvtsi128_si32(_mm_packus_epi16(_mm_packs_epi32(_mm_cvttps_epi32(Vec), _mm_setzero_si128()), _mm_setzero_si128()));
}

/**
* Converts the 4 FLOATs in the vector to 4 BYTEs, clamped to [-127,127], and stores to unaligned memory.
* IMPORTANT: You need to call VectorResetFloatRegisters() before using scalar FLOATs after you've used this intrinsic!
*
* @param Vec			Vector containing 4 FLOATs
* @param Ptr			Unaligned memory pointer to store the 4 BYTEs.
*/
inline void VectorStoreSignedByte4(const VectorRegister& Vec, void* Ptr)
{
	// Looks complex but is really quite straightforward:
	// Convert 4x floats to 4x 32-bit ints, then pack into 4x 16-bit ints, then into 4x 8-bit unsigned ints, then store as a 32-bit value
	*(int32_t*)Ptr = _mm_cvtsi128_si32(_mm_packs_epi16(_mm_packs_epi32(_mm_cvttps_epi32(Vec), _mm_setzero_si128()), _mm_setzero_si128()));
}

/**
 * Returns non-zero if any element in Vec1 is greater than the corresponding element in Vec2, otherwise 0.
 *
 * @param Vec1			1st source vector
 * @param Vec2			2nd source vector
 * @return				Non-zero integer if (Vec1.x > Vec2.x) || (Vec1.y > Vec2.y) || (Vec1.z > Vec2.z) || (Vec1.w > Vec2.w)
 */
#define VectorAnyGreaterThan( Vec1, Vec2 )		_mm_movemask_ps( _mm_cmpgt_ps(Vec1, Vec2) )

 /**
  * Resets the floating point registers so that they can be used again.
  * Some intrinsics use these for MMX purposes (e.g. VectorLoadByte4 and VectorStoreByte4).
  */
  // This is no longer necessary now that we don't use MMX instructions
#define VectorResetFloatRegisters()

inline VectorRegister VectorTruncate(const VectorRegister& X)
{
	return _mm_cvtepi32_ps(_mm_cvttps_epi32(X));
}

inline VectorRegister VectorFractional(const VectorRegister& X)
{
	return VectorSubtract(X, VectorTruncate(X));
}

inline VectorRegister VectorCeil(const VectorRegister& X)
{
	VectorRegister Trunc = VectorTruncate(X);
	VectorRegister PosMask = VectorCompareGE(X, VectorRegsiterConstants::FloatZero);
	VectorRegister Add = VectorSelect(PosMask, VectorRegsiterConstants::FloatOne, (VectorRegsiterConstants::FloatZero));
	return VectorAdd(Trunc, Add);
}

inline VectorRegister VectorFloor(const VectorRegister& X)
{
	VectorRegister Trunc = VectorTruncate(X);
	VectorRegister PosMask = VectorCompareGE(X, (VectorRegsiterConstants::FloatZero));
	VectorRegister Sub = VectorSelect(PosMask, (VectorRegsiterConstants::FloatZero), (VectorRegsiterConstants::FloatOne));
	return VectorSubtract(Trunc, Sub);
}

inline VectorRegister VectorMod(const VectorRegister& X, const VectorRegister& Y)
{
	VectorRegister Div = VectorDivide(X, Y);
	// Floats where abs(f) >= 2^23 have no fractional portion, and larger values would overflow VectorTruncate.
	VectorRegister NoFractionMask = VectorCompareGE(VectorAbs(Div), VectorRegsiterConstants::FloatNonFractional);
	VectorRegister Temp = VectorSelect(NoFractionMask, Div, VectorTruncate(Div));
	VectorRegister Result = VectorSubtract(X, VectorMultiply(Y, Temp));
	// Clamp to [-AbsY, AbsY] because of possible failures for very large numbers (>1e10) due to precision loss.
	VectorRegister AbsY = VectorAbs(Y);
	return VectorMax(VectorNegate(AbsY), VectorMin(Result, AbsY));
}

inline VectorRegister VectorSign(const VectorRegister& X)
{
	VectorRegister Mask = VectorCompareGE(X, (VectorRegsiterConstants::FloatZero));
	return VectorSelect(Mask, (VectorRegsiterConstants::FloatOne), (VectorRegsiterConstants::FloatMinusOne));
}

inline VectorRegister VectorStep(const VectorRegister& X)
{
	VectorRegister Mask = VectorCompareGE(X, (VectorRegsiterConstants::FloatZero));
	return VectorSelect(Mask, (VectorRegsiterConstants::FloatOne), (VectorRegsiterConstants::FloatZero));
}


//////////////////////////////////////////////////////////////////////////
//Integer ops

//Bitwise
/** = a & b */
#define VectorIntAnd(A, B)		_mm_and_si128(A, B)
/** = a | b */
#define VectorIntOr(A, B)		_mm_or_si128(A, B)
/** = a ^ b */
#define VectorIntXor(A, B)		_mm_xor_si128(A, B)
/** = (~a) & b */
#define VectorIntAndNot(A, B)	_mm_andnot_si128(A, B)
/** = ~a */
#define VectorIntNot(A)	_mm_xor_si128(A, VectorRegsiterConstants::IntAllMask)

//Comparison
#define VectorIntCompareEQ(A, B)	_mm_cmpeq_epi32(A,B)
#define VectorIntCompareNEQ(A, B)	VectorIntNot(_mm_cmpeq_epi32(A,B))
#define VectorIntCompareGT(A, B)	_mm_cmpgt_epi32(A,B)
#define VectorIntCompareLT(A, B)	_mm_cmplt_epi32(A,B)
#define VectorIntCompareGE(A, B)	VectorIntNot(VectorIntCompareLT(A,B))
#define VectorIntCompareLE(A, B)	VectorIntNot(VectorIntCompareGT(A,B))


inline VectorRegisterInt VectorIntSelect(const VectorRegisterInt& Mask, const VectorRegisterInt& Vec1, const VectorRegisterInt& Vec2)
{
	return _mm_xor_si128(Vec2, _mm_and_si128(Mask, _mm_xor_si128(Vec1, Vec2)));
}

//Arithmetic
#define VectorIntAdd(A, B)	_mm_add_epi32(A, B)
#define VectorIntSubtract(A, B)	_mm_sub_epi32(A, B)

inline VectorRegisterInt VectorIntMultiply(const VectorRegisterInt& A, const VectorRegisterInt& B)
{
	//SSE2 doesn't have a multiply op for 4 32bit ints. Ugh.
	__m128i Temp0 = _mm_mul_epu32(A, B);
	__m128i Temp1 = _mm_mul_epu32(_mm_srli_si128(A, 4), _mm_srli_si128(B, 4));
	return _mm_unpacklo_epi32(_mm_shuffle_epi32(Temp0, _MM_SHUFFLE(0, 0, 2, 0)), _mm_shuffle_epi32(Temp1, _MM_SHUFFLE(0, 0, 2, 0)));
}

#define VectorIntNegate(A) VectorIntSubtract( VectorRegsiterConstants::IntZero, A)

inline VectorRegisterInt VectorIntMin(const VectorRegisterInt& A, const VectorRegisterInt& B)
{
	VectorRegisterInt Mask = VectorIntCompareLT(A, B);
	return VectorIntSelect(Mask, A, B);
}

inline VectorRegisterInt VectorIntMax(const VectorRegisterInt& A, const VectorRegisterInt& B)
{
	VectorRegisterInt Mask = VectorIntCompareGT(A, B);
	return VectorIntSelect(Mask, A, B);
}

inline VectorRegisterInt VectorIntAbs(const VectorRegisterInt& A)
{
	VectorRegisterInt Mask = VectorIntCompareGE(A, VectorRegsiterConstants::IntZero);
	return VectorIntSelect(Mask, A, VectorIntNegate(A));
}

#define VectorIntSign(A) VectorIntSelect( VectorIntCompareGE(A, VectorRegsiterConstants::IntZero), VectorRegsiterConstants::IntOne, VectorRegsiterConstants::IntMinusOne )

#define VectorIntToFloat(A) _mm_cvtepi32_ps(A)
#define VectorFloatToInt(A) _mm_cvttps_epi32(A)

//Loads and stores

/**
* Stores a vector to memory (aligned or unaligned).
*
* @param Vec	Vector to store
* @param Ptr	Memory pointer
*/
#define VectorIntStore( Vec, Ptr )			_mm_storeu_si128( (VectorRegisterInt*)(Ptr), Vec )

/**
* Loads 4 int32s from unaligned memory.
*
* @param Ptr	Unaligned memory pointer to the 4 int32s
* @return		VectorRegisterInt(Ptr[0], Ptr[1], Ptr[2], Ptr[3])
*/
#define VectorIntLoad( Ptr )				_mm_loadu_si128( (VectorRegisterInt*)(Ptr) )

/**
* Stores a vector to memory (aligned).
*
* @param Vec	Vector to store
* @param Ptr	Aligned Memory pointer
*/
#define VectorIntStoreAligned( Vec, Ptr )			_mm_store_si128( (VectorRegisterInt*)(Ptr), Vec )

/**
* Loads 4 int32s from aligned memory.
*
* @param Ptr	Aligned memory pointer to the 4 int32s
* @return		VectorRegisterInt(Ptr[0], Ptr[1], Ptr[2], Ptr[3])
*/
#define VectorIntLoadAligned( Ptr )				_mm_load_si128( (VectorRegisterInt*)(Ptr) )

/**
* Loads 1 int32_t from unaligned memory into all components of a vector register.
*
* @param Ptr	Unaligned memory pointer to the 4 int32s
* @return		VectorRegisterInt(*Ptr, *Ptr, *Ptr, *Ptr)
*/
#define VectorIntLoad1( Ptr )	_mm_shuffle_epi32(_mm_loadu_si128((VectorRegisterInt*)(Ptr)),_MM_SHUFFLE(0,0,0,0))


