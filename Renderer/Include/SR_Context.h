// \brief
//	renderer context.
//

#pragma once

#include <cstdint> // uint32_t
#include <cstdlib> // size_t
#include <cassert>
#include <memory>

#include <glm.hpp>
#include "SR_Common.h"
#include "SR_Buffer2D.h"
#include "SR_Shader.h"
#include "SR_Material.h"


// render context
class FSR_Context
{
public:
	FSR_Context();
	virtual ~FSR_Context();

	// set render target
	void SetRenderTarget(uint32_t w, uint32_t h, uint32_t nCount);
	// clear render target
	void ClearRenderTarget(const glm::vec4& InColor);
	// set cull face mode
	void SetCullFaceMode(EFrontFace InMode);
	
	// set viewport
	void SetViewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
	
	// set model-view matrix
	void SetModelViewMatrix(const glm::mat4x4& InModelView);
	// set projection matrix
	void SetProjectionMatrix(const glm::mat4x4& InProj);

	// set material
	void SetMaterial(const std::shared_ptr<FSR_Material>& InMaterial) { _material = InMaterial; }
	//set pipeline
	void SetShader(const std::shared_ptr<FSR_VertexShader>& InVs, const std::shared_ptr<FSR_PixelShader>& InPs) 
	{
		_vs = InVs;
		_ps = InPs;
	}

	// get frame-buffer
	std::shared_ptr<FSR_Buffer2D> GetDepthBuffer() const;
	std::shared_ptr<FSR_Buffer2D> GetColorBuffer(uint32_t InIndex) const;

	glm::vec3 NdcToScreenPostion(const glm::vec3& ndc) const;
	const FSR_Rectangle& ViewportRectangle() const { return _viewport_rect; }
	bool DepthTestAndOverride(uint32_t cx, uint32_t cy, float InDepth) const;
public:
	FSR_Rectangle	_viewport_rect;
	
	glm::mat4x4 _modelview;
	glm::mat4x4 _modelview_inv;
	glm::mat4x4 _projection;
	glm::mat4x4 _projection_inv;

	EFrontFace	_front_face;
	
	std::shared_ptr<FSR_DepthBuffer>	_rt_depth;
	std::shared_ptr<FSR_Texture2D>		_rt_colors[MAX_MRT_COUNT];

	std::shared_ptr<FSR_Material>		_material;
	std::shared_ptr<FSR_VertexShader>	_vs;
	std::shared_ptr<FSR_PixelShader>	_ps;
};
