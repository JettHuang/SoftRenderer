// \brief
//		renderer
//


/*
** The following is a discussion of the math used to do edge clipping against
** a clipping plane.
**
**     P1 is an end point of the edge
**     P2 is the other end point of the edge
**
**     Q = (1 - t) *P1 + t*P2
**     That is, Q lies somewhere on the line formed by P1 and P2.
**
**     0 <= t <= 1
**     This constrains Q to lie between P1 and P2.
**
**     C is the plane equation for the clipping plane
**
**     D1 = P1 dot C
**     D1 is the distance between P1 and C.  If P1 lies on the plane
**     then D1 will be zero.  The sign of D1 will determine which side
**     of the plane that P1 is on, with negative being outside.
**
**     D2 = P2 dot C
**     D2 is the distance between P2 and C.  If P2 lies on the plane
**     then D2 will be zero.  The sign of D2 will determine which side
**     of the plane that P2 is on, with negative being outside.
**
** Because we are trying to find the intersection of the P1 P2 line
** segment with the clipping plane we require that:
**
**     Q dot C = 0
**
** Therefore
**
**     ((1-t)*P1 + t*P2) dot C = 0
**
**     (t*P2 + P1 - t*P1) dot C = 0
**
**     t*P2 dot C + P1 dot C - t*P1 dot C = 0
**
** Substituting D1 and D2 in
**
**     t*D2 + D1 - t*D1 = 0
**
** Solving for t
**
**     t = -D1 / (D2 - D1)
**
**     t = D1 / (D1 - D2)
*/

/* the view volume plane (NDC space):

	1. < 1,  0,  0, 1>    left
	2. <-1,  0,  0, 1>    right
	3. < 0,  0,  1, 1>    front
	4. < 0,  0, -1, 1>    back
	5. < 0, -1,  0, 1>    top
	6. < 0,  1,  0, 1>    bottom
*/

#include <algorithm>
#include "SR_Renderer.h"
#include "SR_SSE.h"


static const glm::vec4 kClipPlanes[] =
{
	glm::vec4(1, 0, 0, 1),
	glm::vec4(-1, 0, 0, 1),
	glm::vec4(0, 0, 1, 1),
	glm::vec4(0, 0, -1, 1),
	glm::vec4(0, -1, 0, 1),
	glm::vec4(0, 1, 0, 1)
};

inline void InterpolateVertex_Linear(const FSRVertexShaderOutput& P1, const FSRVertexShaderOutput& P2, float t, FSRVertexShaderOutput& OutVert)
{
	assert(P1._attributes._count == P2._attributes._count);

	OutVert._vertex = glm::mix(P1._vertex, P2._vertex, t);
	for (uint32_t i = 0; i < P1._attributes._count; i++)
	{
		OutVert._attributes._members[i] = glm::mix(P1._attributes._members[i], P2._attributes._members[i], t);
	}
	OutVert._attributes._count = P1._attributes._count;
}

static uint32_t ClipAgainstNearPlane(const FSRVertexShaderOutput InVerts[3], FSRVertexShaderOutput OutVerts[4])
{
	const FSRVertexShaderOutput* P1 = &InVerts[2];
	float D1 = P1->_vertex.z + P1->_vertex.w;

	uint32_t newVerts = 0;
	float t = 0.f;
	for (int i = 0; i < 3; ++i)
	{
		const FSRVertexShaderOutput* P2 = &InVerts[i];
		float D2 = P2->_vertex.z + P2->_vertex.w;

		if (D2 >= 0.f) // P2 is at the front of plane
		{
			if (D2 == 0.f || D1 >= 0.f) {
				OutVerts[newVerts]._vertex = P2->_vertex;
				OutVerts[newVerts]._attributes = P2->_attributes;
				newVerts++;
			}
			else // P1 is at the back of plane
			{
				t = D1 / (D1 - D2);
				InterpolateVertex_Linear(*P1, *P2, t, OutVerts[newVerts]);
				newVerts++;
				OutVerts[newVerts]._vertex = P2->_vertex;
				OutVerts[newVerts]._attributes = P2->_attributes;
				newVerts++;
			}
		}
		else // P2 is at the back of plane
		{
			if (D1 > 0.f) {
				t = D1 / (D1 - D2);
				InterpolateVertex_Linear(*P1, *P2, t, OutVerts[newVerts]);
				newVerts++;
			}
			// otherwise discards the P1, P2
		}

		P1 = P2;
		D1 = D2;
	} // end for i

	return newVerts;
}

static inline float VDOTP(const glm::vec4 &V, const glm::vec4& P)
{
	return (P.x * V.x + P.y * V.y + P.z * V.z + P.w * V.w);
}

static uint32_t ClipAgainstPlane(const FSRVertexShaderOutput InVerts[], const uint32_t InVertsCnt, const glm::vec4 &InPlane,
	FSRVertexShaderOutput OutVerts[])
{
	assert(InVertsCnt >= 2);
	const FSRVertexShaderOutput* P1 = &InVerts[InVertsCnt - 1];
	float D1 = VDOTP(P1->_vertex, InPlane);

	uint32_t newVerts = 0;
	float t = 0.f;
	for (uint32_t i = 0; i < InVertsCnt; ++i)
	{
		const FSRVertexShaderOutput* P2 = &InVerts[i];
		float D2 = VDOTP(P2->_vertex, InPlane);

		if (D2 >= 0.f) // P2 is at the front of plane
		{
			if (D2 == 0.f || D1 >= 0.f) {
				assert(newVerts < MAX_CLIP_VTXCOUNT);
				OutVerts[newVerts]._vertex = P2->_vertex;
				OutVerts[newVerts]._attributes = P2->_attributes;
				newVerts++;
			}
			else // P1 is at the back of plane
			{
				t = D1 / (D1 - D2);
				assert(newVerts < MAX_CLIP_VTXCOUNT);
				InterpolateVertex_Linear(*P1, *P2, t, OutVerts[newVerts]);
				newVerts++;
				assert(newVerts < MAX_CLIP_VTXCOUNT);
				OutVerts[newVerts]._vertex = P2->_vertex;
				OutVerts[newVerts]._attributes = P2->_attributes;
				newVerts++;
			}
		}
		else // P2 is at the back of plane
		{
			if (D1 > 0.f) {
				t = D1 / (D1 - D2);
				assert(newVerts < MAX_CLIP_VTXCOUNT);
				InterpolateVertex_Linear(*P1, *P2, t, OutVerts[newVerts]);
				newVerts++;
			}
			// otherwise discards the P1, P2
		}

		P1 = P2;
		D1 = D2;
	} // end for i

	return newVerts;
}


// https://www.scratchapixel.com/lessons/3d-basic-rendering/rasterization-practical-implementation/rasterization-stage
// https://www.scratchapixel.com/lessons/3d-basic-rendering/rasterization-practical-implementation/visibility-problem-depth-buffer-depth-interpolation
// https://www.scratchapixel.com/lessons/3d-basic-rendering/rasterization-practical-implementation/perspective-correct-interpolation-vertex-attributes
struct FSR_RasterizedVert
{
	glm::vec3	_ndc_pos;
	glm::vec3	_screen_pos;
	float		_inv_w;
};

static bool intersect(const FSR_Rectangle& InA, const FSR_Rectangle& InB, FSR_Rectangle& Out)
{
	float minx = std::max(InA._minx, InB._minx);
	float miny = std::max(InA._miny, InB._miny);
	float maxx = std::min(InA._maxx, InB._maxx);
	float maxy = std::min(InA._maxy, InB._maxy);

	if (minx >= maxx || miny >= maxy) {
		return false;
	}

	Out._minx = minx;
	Out._miny = miny;
	Out._maxx = maxx;
	Out._maxy = maxy;
	return true;
}

// edge function
// NOTE: the following is increment of edge function
// 1. A, B, P
// EdgeFunction = ((P.x - A.x) * (B.y - A.y) - (P.y - A.y) * (B.x - A.x))
// 2. A, B, P1(P.x+1, P.y)
// EdgeFunctionPx = ((P.x + 1.0 - A.x) * (B.y - A.y) - (P.y - A.y) * (B.x - A.x))
//                = ((P.x - A.x) * (B.y - A.y) - (P.y - A.y) * (B.x - A.x)) + (B.y - A.y)
//				  = EdgeFunction + (B.y - A.y)
// 3. A, B, P1(P.x, P.y+1)
// EdgeFunctionPy = ((P.x - A.x) * (B.y - A.y) - (P.y + 1.0 - A.y) * (B.x - A.x))
//                = ((P.x - A.x) * (B.y - A.y) - (P.y - A.y) * (B.x - A.x)) + (B.y - A.y)
//				  = EdgeFunction - (B.x - A.x)
//
inline float EdgeFunction(const glm::vec3& A, const glm::vec3& B, const glm::vec3& P)
{
	return ((P.x - A.x) * (B.y - A.y) - (P.y - A.y) * (B.x - A.x));
}

inline FSR_Rectangle BoundingboxOfTriangle(const glm::vec3& V0, const glm::vec3& V1, const glm::vec3& V2)
{
	float minx = V0.x, miny = V0.y, maxx = V0.x, maxy = V0.y;

	if (V1.x < minx) {
		minx = V1.x; 
	} else if (V1.x > maxx) { 
		maxx = V1.x; 
	}
	if (V1.y < miny) { 
		miny = V1.y; 
	} else if (V1.y > maxy) {
		maxy = V1.y; 
	}

	if (V2.x < minx) { 
		minx = V2.x; 
	} else if (V2.x > maxx) { 
		maxx = V2.x; 
	}
	if (V2.y < miny) { 
		miny = V2.y; 
	} else if (V2.y > maxy) { 
		maxy = V2.y; 
	}
	
	return FSR_Rectangle(minx, miny, maxx, maxy);
}


inline void DivideVertexAttributesByW(const FSRVertexAttributes& VInput, float InOneOverW, FSRVertexAttributes& PInput)
{
#if 0
	for (uint32_t k = 0; k < VInput._count; ++k)
	{
		PInput._members[k] = VInput._members[k] * InOneOverW;
	}
	PInput._count = VInput._count;
#else
	VectorRegister regOneOverW = VectorSetFloat1(InOneOverW);
	for (uint32_t k = 0; k < VInput._count; ++k)
	{
		VectorRegister Value = VectorLoad(&(VInput._members[k].x));
		VectorRegister Result = VectorMultiply(Value, regOneOverW);
		VectorStore(Result, &(PInput._members[k].x));
	}
	PInput._count = VInput._count;
#endif
}

inline void InterpolateVertexAttributes(const FSRVertexAttributes& V0, float w0,
	const FSRVertexAttributes& V1, float w1,
	const FSRVertexAttributes& V2, float w2,
	float W,
	FSRVertexAttributes& Output)
{
#if 0 // before optimize
	for (uint32_t k = 0; k < V0._count; ++k)
	{
		Output._members[k] = (V0._members[k] * w0 +
			V1._members[k] * w1 +
			V2._members[k] * w2) * W;
	}
#else

	VectorRegister RW012 = VectorSetFloat3(w0, w1, w2);
	VectorRegister RW = VectorSetFloat1(W);
	VectorRegister RWW = VectorMultiply(RW012, RW);

	for (uint32_t k = 0; k < V0._count; ++k)
	{
		const glm::vec4& a = V0._members[k];
		const glm::vec4& b = V1._members[k];
		const glm::vec4& c = V2._members[k];
		glm::vec4& o = Output._members[k];

		/*
			o.x = (a.x * w0 + b.x * w1 + c.x * w2) * W;
			o.y = (a.y * w0 + b.y * w1 + c.y * w2) * W;
			o.z = (a.z * w0 + b.z * w1 + c.z * w2) * W;
			o.w = (a.w * w0 + b.w * w1 + c.w * w2) * W;
		*/
		VectorRegister RX = VectorSetFloat3(a.x, b.x, c.x);
		VectorRegister RY = VectorSetFloat3(a.y, b.y, c.y);
		VectorRegister RZ = VectorSetFloat3(a.z, b.z, c.z);
		VectorRegister RW = VectorSetFloat3(a.w, b.w, c.w);

		VectorRegister RXdot = VectorDot3(RX, RWW);
		VectorRegister RYdot = VectorDot3(RY, RWW);
		VectorRegister RZdot = VectorDot3(RZ, RWW);
		VectorRegister RWdot = VectorDot3(RW, RWW);

		o.x = VectorGetComponent(RXdot, 0);
		o.y = VectorGetComponent(RYdot, 0);
		o.z = VectorGetComponent(RZdot, 0);
		o.w = VectorGetComponent(RWdot, 0);
	}
#endif
}



//////////////////////////////////////////////////////////////
/// Tiled Rendering
///
struct FTiledRenderingContext
{
	const FSR_Context* _SRCtx;
	FSR_Context::FPointersShadow _Pointers;
	FSRPixelShaderContext _psCtx;

	/* triangle info */
	float _kOneOverE012;
	FSR_RasterizedVert _SV0, _SV1, _SV2;
	FSRVertexAttributes _VA0, _VA1, _VA2;

	/* tile info */
	int32_t _X0, _Y0, _X1, _Y1;
};

static void RasterizeTriangleNormal_Tile(const FTiledRenderingContext &InCtx)
{
	const float kOneOverE012 = InCtx._kOneOverE012;
	const FSR_RasterizedVert& SV0 = InCtx._SV0;
	const FSR_RasterizedVert& SV1 = InCtx._SV1;
	const FSR_RasterizedVert& SV2 = InCtx._SV2;
	const FSRVertexAttributes& VA0 = InCtx._VA0;
	const FSRVertexAttributes& VA1 = InCtx._VA1;
	const FSRVertexAttributes& VA2 = InCtx._VA2;

	const int32_t X0 = InCtx._X0;
	const int32_t Y0 = InCtx._Y0;
	const int32_t X1 = InCtx._X1;
	const int32_t Y1 = InCtx._Y1;

	FSR_PixelShader* ps = InCtx._Pointers._ps;
	assert(ps);

	FSRPixelShaderInput PixelInput;
	FSRPixelShaderOutput PixelOutput;

	// set count only once
	PixelInput._attributes._count = VA0._count;
	PixelOutput._color_cnt = ps->OutputColorCount();
	assert(PixelOutput._color_cnt <= MAX_MRT_COUNT);

	// depth buffer & color buffer
	uint8_t* pDepthBufferRow;
	uint8_t* pColorBufferRows[MAX_MRT_COUNT];

	assert(InCtx._Pointers._rt_depth);

	const glm::vec3 P(X0 + 0.5f, Y0 + 0.5f, 0.f);
	const float PE12 = EdgeFunction(SV1._screen_pos, SV2._screen_pos, P);
	const float PE20 = EdgeFunction(SV2._screen_pos, SV0._screen_pos, P);
	const float PE01 = EdgeFunction(SV0._screen_pos, SV1._screen_pos, P);
	const glm::vec3 edge12 = SV2._screen_pos - SV1._screen_pos;
	const glm::vec3 edge20 = SV0._screen_pos - SV2._screen_pos;
	const glm::vec3 edge01 = SV1._screen_pos - SV0._screen_pos;

	VectorRegister RegEY = MakeVectorRegister(PE12, PE20, PE01, 1.f);
	VectorRegister RegVX = MakeVectorRegister(edge12.x, edge20.x, edge01.x, 0.f);
	VectorRegister RegVY = MakeVectorRegister(edge12.y, edge20.y, edge01.y, 0.f);
	for (int32_t cy = Y0; cy < Y1; ++cy, RegEY = VectorSubtract(RegEY, RegVX))
	{
		pDepthBufferRow = InCtx._Pointers._rt_depth->GetRowData(cy);
		for (uint32_t k = 0; k < PixelOutput._color_cnt; ++k)
		{
			assert(InCtx._Pointers._rt_colors[k]);
			pColorBufferRows[k] = InCtx._Pointers._rt_colors[k]->GetRowData(cy);;
		}

		VectorRegister RegEX = RegEY;
		for (int32_t cx = X0; cx < X1; ++cx, RegEX = VectorAdd(RegEX, RegVY))
		{
			float E12 = VectorGetComponent(RegEX, 0);
			float E20 = VectorGetComponent(RegEX, 1);
			float E01 = VectorGetComponent(RegEX, 2);

			if (E12 < 0.f)
			{
				if (edge12.y <= 0.f)
				{
					break; // break out of for cx
				}
				// outside of the triangle
				continue;
			}
			if (E20 < 0.f)
			{
				if (edge20.y <= 0.f)
				{
					break; // break out of for cx
				}
				// outside of the triangle
				continue;
			}
			if (E01 < 0.f)
			{
				if (edge01.y <= 0.f)
				{
					break; // break out of for cx
				}
				// outside of the triangle
				continue;
			}

			// top-left rule:
			// the pixel or point is considered to overlap a triangle if it is either inside the triangle or 
			// lies on either a triangle top edge or any edge that is considered to be a left edge.
			if (E12 == 0.f) {
				if (!((edge12.y == 0.f && edge12.x > 0.f) || (edge12.y > 0.f))) {
					continue;
				}
			}
			if (E20 == 0.f) {
				if (!((edge20.y == 0.f && edge20.x > 0.f) || (edge20.y > 0.f))) {
					continue;
				}
			}
			if (E01 == 0.f) {
				if (!((edge01.y == 0.f && edge01.x > 0.f) || (edge01.y > 0.f))) {
					continue;
				}
			}

			// perspective correct interpolate
			const float w0 = E12 * kOneOverE012;
			const float w1 = E20 * kOneOverE012;
			const float w2 = (1.f - w0 - w1); // fixed for (w0 + w1 + w2) != 1.0

			const float depth = w0 * SV0._screen_pos.z + w1 * SV1._screen_pos.z + w2 * SV2._screen_pos.z;
			const float W = 1.f / (w0 * SV0._inv_w + w1 * SV1._inv_w + w2 * SV2._inv_w);

			bool bPassDepth = false;
			{
				float PrevDepth;
				InCtx._Pointers._rt_depth->Read(pDepthBufferRow, cx, PrevDepth);
				if (depth <= PrevDepth)
				{
					InCtx._Pointers._rt_depth->Write(pDepthBufferRow, cx, depth);
					bPassDepth = true;
				}
			}

			if (!bPassDepth)
			{
				continue;
			}

			// attributes
			InterpolateVertexAttributes(VA0, w0, VA1, w1, VA2, w2, W, PixelInput._attributes);

			ps->Process(InCtx._psCtx, PixelInput, PixelOutput);

			// output and merge color
			for (uint32_t k = 0; k < PixelOutput._color_cnt; ++k)
			{
				const glm::vec4& color = PixelOutput._colors[k];
				FSR_Texture2D* rt = InCtx._Pointers._rt_colors[k];
				rt->Write(pColorBufferRows[k], cx, &color.r);
			} // end for k

		} //end cx
	} // end cy
}

static void MultiThreadsProcessTile(const FTiledRenderingContext& InCtx, void (*handler)(const FTiledRenderingContext& InCtx));
static void RasterizeTriangleNormal(const FSR_Context& InContext, const FSRVertexShaderOutput& A, const FSRVertexShaderOutput& B, const FSRVertexShaderOutput& C)
{
	const FSRVertexShaderOutput* ABC[3] = { &A, &B, &C };
	FSR_RasterizedVert screen[3];

	// perspective divide
	for (int32_t k=0; k<3; ++k)
	{
		const glm::vec4& vertex = ABC[k]->_vertex;
		FSR_RasterizedVert& srv = screen[k];

		float inv_w = 1.f / vertex.w;
		srv._inv_w = inv_w;
#if 0
		srv._ndc_pos.x = vertex.x * inv_w;
		srv._ndc_pos.y = vertex.y * inv_w;
		srv._ndc_pos.z = vertex.z * inv_w;
#else
		VectorRegister R0 = VectorLoadFloat3_W0(&vertex.x);
		VectorRegister R1 = VectorSetFloat1(inv_w);
		VectorRegister R2 = VectorMultiply(R0, R1);
		VectorStoreFloat3(R2, &(srv._ndc_pos.x));
#endif
		srv._screen_pos = InContext.NDCToScreenPostion(srv._ndc_pos);
	}

	uint32_t iv0 = 0, iv1 = 1, iv2 = 2;
	float E012 = EdgeFunction(screen[0]._screen_pos, screen[1]._screen_pos, screen[2]._screen_pos);
	if (E012 > -1.f && E012 < 1.f)
	{
		return; // DISCARD!
	}

	bool bClockwise = (E012 >= 0.f);
	bool bClipped = bClockwise ^ (InContext._front_face == EFrontFace::FACE_CW);
	if (bClipped)
	{
		return;  // DISCARD!
	}

	if (!bClockwise) // exchange v to clockwise
	{
		iv1 = 2; iv2 = 1;
		E012 = -E012;
	}

	FSR_Rectangle bbox;
	const FSR_Rectangle bbox_triangle = BoundingboxOfTriangle(screen[0]._screen_pos, screen[1]._screen_pos, screen[2]._screen_pos);
	if (!intersect(bbox_triangle, InContext.ViewportRectangle(), bbox))
	{
		return; // DISCARD!
	}

	FTiledRenderingContext TileCtx;

	TileCtx._SRCtx = &InContext;
	TileCtx._Pointers = InContext._pointers_shadow;
	TileCtx._psCtx._mvps = InContext._mvps;
	TileCtx._psCtx._material = InContext._pointers_shadow._material;

	const float kOneOverE012 = 1.f / E012;
	const FSR_RasterizedVert& SV0 = screen[iv0];
	const FSR_RasterizedVert& SV1 = screen[iv1];
	const FSR_RasterizedVert& SV2 = screen[iv2];

	TileCtx._kOneOverE012 = kOneOverE012;
	TileCtx._SV0 = SV0;
	TileCtx._SV1 = SV1;
	TileCtx._SV2 = SV2;

	DivideVertexAttributesByW(ABC[iv0]->_attributes, SV0._inv_w, TileCtx._VA0);
	DivideVertexAttributesByW(ABC[iv1]->_attributes, SV1._inv_w, TileCtx._VA1);
	DivideVertexAttributesByW(ABC[iv2]->_attributes, SV2._inv_w, TileCtx._VA2);

	const int32_t X0 = static_cast<int32_t>(floor(bbox._minx));
	const int32_t Y0 = static_cast<int32_t>(floor(bbox._miny));
	const int32_t X1 = static_cast<int32_t>(ceilf(bbox._maxx));
	const int32_t Y1 = static_cast<int32_t>(ceilf(bbox._maxy));

	TileCtx._X0 = X0;
	TileCtx._Y0 = Y0;
	TileCtx._X1 = X1;
	TileCtx._Y1 = Y1;

	if (InContext._bEnableMultiThreads) {
		MultiThreadsProcessTile(TileCtx, &RasterizeTriangleNormal_Tile);
	}
	else {
		RasterizeTriangleNormal_Tile(TileCtx);
	}
}

static void RasterizeTriangleMSAA4_Tile(const FTiledRenderingContext& InCtx)
{
	static const float samples_pattern[4][2] = { { 0.25f, 0.25f }, { 0.75f, 0.25f }, { 0.75f, 0.75f }, { 0.25f, 0.75f } };

	const float kOneOverE012 = InCtx._kOneOverE012;
	const FSR_RasterizedVert& SV0 = InCtx._SV0;
	const FSR_RasterizedVert& SV1 = InCtx._SV1;
	const FSR_RasterizedVert& SV2 = InCtx._SV2;
	const FSRVertexAttributes& VA0 = InCtx._VA0;
	const FSRVertexAttributes& VA1 = InCtx._VA1;
	const FSRVertexAttributes& VA2 = InCtx._VA2;

	const int32_t X0 = InCtx._X0;
	const int32_t Y0 = InCtx._Y0;
	const int32_t X1 = InCtx._X1;
	const int32_t Y1 = InCtx._Y1;

	FSR_PixelShader* ps = InCtx._Pointers._ps;
	assert(ps);

	glm::vec3 P(0);
	FSRPixelShaderInput PixelInput;
	FSRPixelShaderOutput PixelOutput;

	// set count only once
	PixelInput._attributes._count = VA0._count;
	PixelOutput._color_cnt = ps->OutputColorCount();

	for (int32_t cy = Y0; cy < Y1; ++cy)
	{
		for (int32_t cx = X0; cx < X1; ++cx)
		{
			int32_t bitMask = 0;
			for (int32_t sampleIndex = 0; sampleIndex < 4; ++sampleIndex)
			{

				P.x = cx + samples_pattern[sampleIndex][0];
				P.y = cy + samples_pattern[sampleIndex][1];

				float E12 = EdgeFunction(SV1._screen_pos, SV2._screen_pos, P);
				float E20 = EdgeFunction(SV2._screen_pos, SV0._screen_pos, P);
				float E01 = EdgeFunction(SV0._screen_pos, SV1._screen_pos, P);
				if (E12 < 0.f || E20 < 0.f || E01 < 0.f)
				{
					// outside of the triangle
					continue;
				}

				glm::vec3 edge12 = SV2._screen_pos - SV1._screen_pos;
				glm::vec3 edge20 = SV0._screen_pos - SV2._screen_pos;
				glm::vec3 edge01 = SV1._screen_pos - SV0._screen_pos;

				// top-left rule:
				// the pixel or point is considered to overlap a triangle if it is either inside the triangle or 
				// lies on either a triangle top edge or any edge that is considered to be a left edge.
				if (E12 == 0.f) {
					if (!((edge12.y == 0.f && edge12.x > 0.f) || (edge12.y > 0.f))) {
						continue;
					}
				}
				if (E20 == 0.f) {
					if (!((edge20.y == 0.f && edge20.x > 0.f) || (edge20.y > 0.f))) {
						continue;
					}
				}
				if (E01 == 0.f) {
					if (!((edge01.y == 0.f && edge01.x > 0.f) || (edge01.y > 0.f))) {
						continue;
					}
				}

				// perspective correct interpolate
				const float w0 = E12 * kOneOverE012;
				const float w1 = E20 * kOneOverE012;
				const float w2 = (1.f - w0 - w1); // fixed for (w0 + w1 + w2) != 1.0

				const float depth = w0 * SV0._screen_pos.z + w1 * SV1._screen_pos.z + w2 * SV2._screen_pos.z;

				bool bPassDepth = InCtx._SRCtx->DepthTestAndOverrideMSAA(cx, cy, depth, sampleIndex);

				if (!bPassDepth)
				{
					continue;
				}

				bitMask |= (0x01 << sampleIndex);
			} // end for sampleIndex

			if (!bitMask)
			{
				continue;
			}

			// calculate center of pixel
			{
				P.x = cx + 0.5f;
				P.y = cy + 0.5f;

				float E12 = EdgeFunction(SV1._screen_pos, SV2._screen_pos, P);
				float E20 = EdgeFunction(SV2._screen_pos, SV0._screen_pos, P);
				float E01 = EdgeFunction(SV0._screen_pos, SV1._screen_pos, P);
				// perspective correct interpolate
				const float w0 = E12 * kOneOverE012;
				const float w1 = E20 * kOneOverE012;
				const float w2 = (1.f - w0 - w1); // fixed for (w0 + w1 + w2) != 1.0
				const float W = 1.f / (w0 * SV0._inv_w + w1 * SV1._inv_w + w2 * SV2._inv_w);

				// attributes
				InterpolateVertexAttributes(VA0, w0, VA1, w1, VA2, w2, W, PixelInput._attributes);
			}

			ps->Process(InCtx._psCtx, PixelInput, PixelOutput);

			InCtx._SRCtx->OutputAndMergeColorMSAA(cx, cy, PixelOutput, bitMask);

		} //end cx
	} // end cy
}

static void RasterizeTriangleMSAA4(const FSR_Context& InContext, const FSRVertexShaderOutput& A, const FSRVertexShaderOutput& B, const FSRVertexShaderOutput& C)
{
	assert(InContext._MSAASamplesNum == 4);

	const FSRVertexShaderOutput* ABC[3] = { &A, &B, &C };
	FSR_RasterizedVert screen[3];

	// perspective divide
	for (uint32_t i = 0; i < 3; ++i)
	{
		float inv_w = 1.f / ABC[i]->_vertex.w;
		screen[i]._inv_w = inv_w;
		screen[i]._ndc_pos.x = ABC[i]->_vertex.x * inv_w;
		screen[i]._ndc_pos.y = ABC[i]->_vertex.y * inv_w;
		screen[i]._ndc_pos.z = ABC[i]->_vertex.z * inv_w;
		screen[i]._screen_pos = InContext.NDCToScreenPostion(screen[i]._ndc_pos);
	}

	uint32_t iv0 = 0, iv1 = 1, iv2 = 2;
	float E012 = EdgeFunction(screen[0]._screen_pos, screen[1]._screen_pos, screen[2]._screen_pos);
	if (E012 > -1.f && E012 < 1.f)
	{
		return; // DISCARD!
	}

	bool bClockwise = (E012 >= 0.f);
	bool bClipped = bClockwise ^ (InContext._front_face == EFrontFace::FACE_CW);
	if (bClipped)
	{
		return;  // DISCARD!
	}

	if (!bClockwise) // exchange v to clockwise
	{
		iv1 = 2; iv2 = 1;
		E012 = -E012;
	}

	FSR_Rectangle bbox;
	const FSR_Rectangle bbox_triangle = BoundingboxOfTriangle(screen[0]._screen_pos, screen[1]._screen_pos, screen[2]._screen_pos);
	if (!intersect(bbox_triangle, InContext.ViewportRectangle(), bbox))
	{
		return; // DISCARD!
	}

	FTiledRenderingContext TileCtx;

	TileCtx._SRCtx = &InContext;
	TileCtx._Pointers = InContext._pointers_shadow;
	TileCtx._psCtx._mvps = InContext._mvps;
	TileCtx._psCtx._material = InContext._pointers_shadow._material;

	const float kOneOverE012 = 1.f / E012;
	const FSR_RasterizedVert& SV0 = screen[iv0];
	const FSR_RasterizedVert& SV1 = screen[iv1];
	const FSR_RasterizedVert& SV2 = screen[iv2];

	TileCtx._kOneOverE012 = kOneOverE012;
	TileCtx._SV0 = SV0;
	TileCtx._SV1 = SV1;
	TileCtx._SV2 = SV2;

	DivideVertexAttributesByW(ABC[iv0]->_attributes, SV0._inv_w, TileCtx._VA0);
	DivideVertexAttributesByW(ABC[iv1]->_attributes, SV1._inv_w, TileCtx._VA1);
	DivideVertexAttributesByW(ABC[iv2]->_attributes, SV2._inv_w, TileCtx._VA2);

	const int32_t X0 = static_cast<int32_t>(floor(bbox._minx));
	const int32_t Y0 = static_cast<int32_t>(floor(bbox._miny));
	const int32_t X1 = static_cast<int32_t>(ceilf(bbox._maxx));
	const int32_t Y1 = static_cast<int32_t>(ceilf(bbox._maxy));

	TileCtx._X0 = X0;
	TileCtx._Y0 = Y0;
	TileCtx._X1 = X1;
	TileCtx._Y1 = Y1;

	if (InContext._bEnableMultiThreads) {
		MultiThreadsProcessTile(TileCtx, &RasterizeTriangleMSAA4_Tile);
	}
	else {
		RasterizeTriangleMSAA4_Tile(TileCtx);
	}
}

static void RasterizeTriangle(const FSR_Context& InContext, const FSRVertexShaderOutput& A, const FSRVertexShaderOutput& B, const FSRVertexShaderOutput& C)
{
	if (InContext._bEnableMSAA)
	{
		RasterizeTriangleMSAA4(InContext, A, B, C);
	}
	else
	{
		RasterizeTriangleNormal(InContext, A, B, C);
	}
}
//////////////////////////////////////////////////////////////////////////

inline bool IsOnNegtiveSideOf_LeftPlane(const glm::vec4& V0, const glm::vec4& V1, const glm::vec4& V2)
{
	return ((V0.x + V0.w) < 0.f) && ((V1.x + V1.w) < 0.f) && ((V2.x + V2.w) < 0.f);
}

inline bool IsOnNegtiveSideOf_RightPlane(const glm::vec4& V0, const glm::vec4& V1, const glm::vec4& V2)
{
	return ((V0.w - V0.x) < 0.f) && ((V1.w - V1.x) < 0.f) && ((V2.w - V2.x) < 0.f);
}

inline bool IsOnNegtiveSideOf_FrontPlane(const glm::vec4& V0, const glm::vec4& V1, const glm::vec4& V2)
{
	return ((V0.z + V0.w) < 0.f) && ((V1.z + V1.w) < 0.f) && ((V2.z + V2.w) < 0.f);
}

inline bool IsOnNegtiveSideOf_BackPlane(const glm::vec4& V0, const glm::vec4& V1, const glm::vec4& V2)
{
	return ((V0.w - V0.z) < 0.f) && ((V1.w - V1.z) < 0.f) && ((V2.w - V2.z) < 0.f);
}

inline bool IsOnNegtiveSideOf_TopPlane(const glm::vec4& V0, const glm::vec4& V1, const glm::vec4& V2)
{
	return ((V0.w - V0.y) < 0.f) && ((V1.w - V1.y) < 0.f) && ((V2.w - V2.y) < 0.f);
}

inline bool IsOnNegtiveSideOf_BotPlane(const glm::vec4& V0, const glm::vec4& V1, const glm::vec4& V2)
{
	return ((V0.w + V0.y) < 0.f) && ((V1.w + V1.y) < 0.f) && ((V2.w + V2.y) < 0.f);
}

// draw a triangle
void FSR_Renderer::DrawTriangle(const FSR_Context& InContext, const FSRVertex& InA, const FSRVertex& InB, const FSRVertex& InC)
{
	FSR_Context& InCtx = const_cast<FSR_Context&>(InContext);

#if SR_ENABLE_PERFORMACE_STAT
	FPerformanceCounter	PerfCounter;
	FSR_Performance *Stats = InContext._pointers_shadow._stats;
	double elapse_microseconds = 0.0;
#endif

#if SR_ENABLE_PERFORMACE_STAT
	PerfCounter.StartPerf();
#endif

	FSR_VertexShader *vs = InContext._pointers_shadow._vs;
	assert(vs);

	vs->Process(InContext, InA, InCtx._clip_vtx_buffer0[0]);
	vs->Process(InContext, InB, InCtx._clip_vtx_buffer0[1]);
	vs->Process(InContext, InC, InCtx._clip_vtx_buffer0[2]);

#if SR_ENABLE_PERFORMACE_STAT
	elapse_microseconds = PerfCounter.EndPerf();

	Stats->_triangles_count++;
	Stats->_vertexes_count += 3;
	Stats->_vs_invoke_count += 3;
	Stats->_vs_total_microseconds += elapse_microseconds;
#endif

#if SR_ENABLE_PERFORMACE_STAT
	PerfCounter.StartPerf();
#endif
	// frustum culling
	bool bOutsideOfVolume = false;
	{
		const glm::vec4& homogeneous_v0 = InCtx._clip_vtx_buffer0[0]._vertex;
		const glm::vec4& homogeneous_v1 = InCtx._clip_vtx_buffer0[1]._vertex;
		const glm::vec4& homogeneous_v2 = InCtx._clip_vtx_buffer0[2]._vertex;

		bOutsideOfVolume =
			IsOnNegtiveSideOf_LeftPlane(homogeneous_v0, homogeneous_v1, homogeneous_v2) ||
			IsOnNegtiveSideOf_RightPlane(homogeneous_v0, homogeneous_v1, homogeneous_v2) ||
			IsOnNegtiveSideOf_FrontPlane(homogeneous_v0, homogeneous_v1, homogeneous_v2) ||
			IsOnNegtiveSideOf_BackPlane(homogeneous_v0, homogeneous_v1, homogeneous_v2) ||
			IsOnNegtiveSideOf_TopPlane(homogeneous_v0, homogeneous_v1, homogeneous_v2) ||
			IsOnNegtiveSideOf_BotPlane(homogeneous_v0, homogeneous_v1, homogeneous_v2);
	}

#if SR_ENABLE_PERFORMACE_STAT
	elapse_microseconds = PerfCounter.EndPerf();
	Stats->_check_inside_frustum_count++;
	Stats->_check_inside_frustum_microseconds += elapse_microseconds;
#endif

	if (bOutsideOfVolume)
	{
		return; // DISCARD!
	}

#if SR_ENABLE_PERFORMACE_STAT
	PerfCounter.StartPerf();
#endif

#if 0 // DEBUG
	InCtx._clip_vtx_buffer0[0]._attributes._members[0] = glm::vec4(1.0f, 0.f, 0.f, 1.f);
	InCtx._clip_vtx_buffer0[1]._attributes._members[0] = glm::vec4(0.0f, 1.f, 0.f, 1.f);
	InCtx._clip_vtx_buffer0[2]._attributes._members[0] = glm::vec4(0.0f, 0.f, 1.f, 1.f);
#endif

	uint32_t verts_cnt = 3;
	FSRVertexShaderOutput* vtx_buffer0 = InCtx._clip_vtx_buffer0;
	FSRVertexShaderOutput* vtx_buffer1 = InCtx._clip_vtx_buffer1;
	for (uint32_t i = 0; (i < sizeof(kClipPlanes) / sizeof(kClipPlanes[0])) && verts_cnt >= 3; ++i)
	{
		verts_cnt = ClipAgainstPlane(vtx_buffer0, verts_cnt, kClipPlanes[i], vtx_buffer1);
		FSRVertexShaderOutput* temp = vtx_buffer0;
		vtx_buffer0 = vtx_buffer1;
		vtx_buffer1 = temp;
	};
	if (verts_cnt < 3)
	{
		return; // DISCARD!
	}

	const FSRVertexShaderOutput* vtx_buffer = vtx_buffer0;

#if SR_ENABLE_PERFORMACE_STAT
	elapse_microseconds = PerfCounter.EndPerf();

	Stats->_clip_invoke_count++;
	Stats->_clip_total_microseconds += elapse_microseconds;
#endif

#if SR_ENABLE_PERFORMACE_STAT
	PerfCounter.StartPerf();
#endif

	uint32_t iv0 = 0, iv1 = 1, iv2 = 2;
	for (uint32_t i=2; i<verts_cnt; ++i)
	{
		iv2 = i;
		RasterizeTriangle(InContext, vtx_buffer[iv0], vtx_buffer[iv1], vtx_buffer[iv2]);
		iv1 = iv2;
	}

#if SR_ENABLE_PERFORMACE_STAT
	elapse_microseconds = PerfCounter.EndPerf();

	Stats->_raster_invoked_count += (verts_cnt == 3) ? 1 : ((verts_cnt == 4) ? 2 : 0);
	Stats->_raster_total_microseconds += elapse_microseconds;
#endif
}


// draw a mesh
void FSR_Renderer::DrawMesh(FSR_Context& InContext, const FSR_Mesh& InMesh)
{
	const std::vector<FSRVertex> &VertexBuffer = InMesh._VertexBuffer;
	const std::vector<uint32_t>& IndexBuffer = InMesh._IndexBuffer;;
	const std::vector<std::shared_ptr<FSR_Material>>& Materials = InMesh._Materials;

	for (uint32_t k=0; k<InMesh._SubMeshes.size(); ++k)
	{
		const FSR_Mesh::FSR_SubMesh& subMesh = InMesh._SubMeshes[k];

		if (subMesh._MaterialIndex != SR_INVALID_INDEX) {
			InContext.SetMaterial(Materials[subMesh._MaterialIndex]);
		}
		
		// draw triangles
		const uint32_t triangleCount = subMesh._IndexCount / 3;
		for (uint32_t idx = 0; idx < triangleCount; idx++)
		{
			const FSRVertex& V0 = VertexBuffer[IndexBuffer[subMesh._IndexOffset + (idx * 3)]];
			const FSRVertex& V1 = VertexBuffer[IndexBuffer[subMesh._IndexOffset + (idx * 3 + 1)]];
			const FSRVertex& V2 = VertexBuffer[IndexBuffer[subMesh._IndexOffset + (idx * 3 + 2)]];

			DrawTriangle(InContext, V0, V1, V2);
		}

	} // end for k
}


//////////////////////////////////////////////////////////////////////////
//  Multi-Thread Tiled-Rendering
//
//////////////////////////////////////////////////////////////////////////
#include <thread>
#include <mutex>
#include <condition_variable>


typedef void (*pfnTileHandler)(const FTiledRenderingContext& InCtx);

struct FTileCommand {
	bool _terminate;
	FTiledRenderingContext	_ctx;
	pfnTileHandler	_handler;
};

#define BUFFER_SIZE		32
class FRingBuffer
{
public:
	FRingBuffer() 
		: _head(0)
		, _tail(0)
	{}

	void Enqueue(const FTileCommand& InCmd);
	void Dequeue(FTileCommand &cmd);
	void WaitForEmpty();
protected:
	bool IsFull() { return ((_tail + 1) % BUFFER_SIZE) == _head; }
	bool IsEmpty() { return _head == _tail; }
protected:
	FTileCommand _cmdbuffer[BUFFER_SIZE];
	int _head, _tail;

	std::mutex	_mutex;
	std::condition_variable _cv;
};

void FRingBuffer::Enqueue(const FTileCommand& InCmd)
{
	std::unique_lock<std::mutex> lock(_mutex);

	while (IsFull()) {
		_cv.wait(lock);
	}

	_cmdbuffer[_tail] = InCmd;
	_tail = (_tail + 1) % BUFFER_SIZE;

	_cv.notify_all();
}

void FRingBuffer::Dequeue(FTileCommand& OutCmd)
{
	std::unique_lock<std::mutex> lock(_mutex);

	while (IsEmpty()) {
		_cv.wait(lock);
	}

	OutCmd = _cmdbuffer[_head];
	_head = (_head + 1) % BUFFER_SIZE;

	_cv.notify_all();
}

void FRingBuffer::WaitForEmpty()
{
	std::unique_lock<std::mutex> lock(_mutex);

	while (!IsEmpty()) {
		_cv.wait(lock);
	}
}


#define TILES_X		6
#define TILES_Y		6

class FTileRenderSystem
{
public:
	static FTileRenderSystem& sharedInstance();

	void Start();
	void Terminate();
	void FlushCommands();

public:
	FTileRenderSystem() {}

	FRingBuffer _cmdbuffers[TILES_Y][TILES_X];
	std::thread _threads[TILES_Y][TILES_X];
};

static void thread_callback(int tilex, int tiley)
{
	FTileRenderSystem& sharedSys = FTileRenderSystem::sharedInstance();
	FRingBuffer& buffer = sharedSys._cmdbuffers[tiley][tilex];
	FTileCommand cmd;

	while (1) {
		buffer.Dequeue(cmd);
		if (cmd._terminate)
		{
			break;
		}
		cmd._handler(cmd._ctx);
	}
}

FTileRenderSystem& FTileRenderSystem::sharedInstance()
{
	static FTileRenderSystem shared;

	return shared;
}

void FTileRenderSystem::Start()
{
	for (int y = 0; y<TILES_Y; y++)
	{
		for (int x=0; x<TILES_X; x++)
		{
			_threads[y][x] = std::thread(&thread_callback, x, y);
		}
	}
}

void FTileRenderSystem::Terminate()
{
	FTileCommand Cmd;

	Cmd._terminate = true;
	for (int y = 0; y < TILES_Y; y++)
	{
		for (int x = 0; x < TILES_X; x++)
		{
			_cmdbuffers[y][x].Enqueue(Cmd);
			_threads[y][x].join();
		}
	}
}

void FTileRenderSystem::FlushCommands()
{
	for (int y = 0; y < TILES_Y; y++)
	{
		for (int x = 0; x < TILES_X; x++)
		{
			_cmdbuffers[y][x].WaitForEmpty();
		}
	}
}

static void MultiThreadsProcessTile(const FTiledRenderingContext& InCtx, void (*handler)(const FTiledRenderingContext& InCtx))
{
	FTileRenderSystem& sharedSys = FTileRenderSystem::sharedInstance();

	const FSR_Rectangle	&vprect = InCtx._SRCtx->_viewport_rect;
	float X[TILES_X+1], Y[TILES_Y+1];

	float dx = (int32_t)((vprect._maxx - vprect._minx) / TILES_X);
	float dy = (int32_t)((vprect._maxy - vprect._miny) / TILES_Y);
	int32_t k, i, j;

	for (k=1, X[0] = vprect._minx; k<TILES_X; k++)
	{
		X[k] = X[k - 1] + dx;
	}
	X[k] = vprect._maxx;

	for (k=1, Y[0] = vprect._miny; k<TILES_Y; k++)
	{
		Y[k] = Y[k - 1] + dy;
	}
	Y[k] = vprect._maxy;


	FTileCommand cmd = { false, InCtx, handler };
	FSR_Rectangle rect0, rect1, rect2;
	rect0._minx = InCtx._X0;
	rect0._miny = InCtx._Y0;
	rect0._maxx = InCtx._X1;
	rect0._maxy = InCtx._Y1;

	for (i=0; i<TILES_Y; i++)
	{
		rect1._miny = Y[i];
		rect1._maxy = Y[i + 1];
		for (j=0; j<TILES_X; j++)
		{
			rect1._minx = X[j];
			rect1._maxx = X[j + 1];
			if (intersect(rect0, rect1, rect2)) {

				cmd._ctx._X0 = rect2._minx;
				cmd._ctx._Y0 = rect2._miny;
				cmd._ctx._X1 = rect2._maxx;
				cmd._ctx._Y1 = rect2._maxy;

				sharedSys._cmdbuffers[i][j].Enqueue(cmd);
			}
		} // end for j
	} // end for i
}

bool FSR_Renderer::EnableMultiThreads()
{
	FTileRenderSystem& shared = FTileRenderSystem::sharedInstance();

	shared.Start();
	return true;
}

void FSR_Renderer::Flush(FSR_Context& InContext)
{
	FTileRenderSystem& shared = FTileRenderSystem::sharedInstance();

	if (InContext._bEnableMultiThreads) 
	{
		shared.FlushCommands();
	}
}

void FSR_Renderer::TerminateMultiThreads(FSR_Context& InContext)
{
	FTileRenderSystem& shared = FTileRenderSystem::sharedInstance();

	if (InContext._bEnableMultiThreads)
	{
		shared.Terminate();
	}
}
