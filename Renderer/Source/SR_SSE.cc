// \brief
//	SR_SSE.cc
//

#include "SR_SSE.h"


#define FLOAT_NON_FRACTIONAL (8388608.f) /* All single-precision floating point numbers greater than or equal to this have no fractional value. */
/** Bitmask to AND out the sign bit of each components in a vector */
#define SIGN_BIT ((1 << 31))

const VectorRegister VectorRegsiterConstants::FloatOne = MakeVectorRegister(1.0f, 1.0f, 1.0f, 1.0f);
const VectorRegister VectorRegsiterConstants::FloatZero = MakeVectorRegister(0.0f, 0.0f, 0.0f, 0.0f);
const VectorRegister VectorRegsiterConstants::FloatNonFractional = MakeVectorRegister(FLOAT_NON_FRACTIONAL, FLOAT_NON_FRACTIONAL, FLOAT_NON_FRACTIONAL, FLOAT_NON_FRACTIONAL);
const VectorRegister VectorRegsiterConstants::SignMask = MakeVectorRegister((uint32_t)(~SIGN_BIT), (uint32_t)(~SIGN_BIT), (uint32_t)(~SIGN_BIT), (uint32_t)(~SIGN_BIT));
const VectorRegister VectorRegsiterConstants::FloatMinusOne = MakeVectorRegister(-1.0f, -1.0f, -1.0f, -1.0f);
const VectorRegister VectorRegsiterConstants::FloatOneOver255 = MakeVectorRegister(1.f/255.f, 1.f / 255.f, 1.f / 255.f, 1.f / 255.f);
const VectorRegister VectorRegsiterConstants::Float255 = MakeVectorRegister(255.f, 255.f, 255.f, 255.f);

const VectorRegisterInt VectorRegsiterConstants::IntAllMask = MakeVectorRegisterInt(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);
const VectorRegisterInt VectorRegsiterConstants::IntOne = MakeVectorRegisterInt(1, 1, 1, 1);
const VectorRegisterInt VectorRegsiterConstants::IntZero = MakeVectorRegisterInt(0, 0, 0, 0);
const VectorRegisterInt VectorRegsiterConstants::IntMinusOne = MakeVectorRegisterInt(-1, -1, -1, -1);
	
#undef SIGN_BIT
