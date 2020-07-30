// \brief
//		mesh class
//

#pragma once

#include <cstdint> // uint32_t
#include <cstdlib> // size_t
#include <cassert>
#include <vector>

#include "SR_Common.h"
#include "SR_Material.h"


#define SRMESH_NORMAL_ATTRIB		0
#define SRMESH_UV_ATTRIB			1

// mesh
class FSR_Mesh
{
public:
	FSR_Mesh() {}
	virtual ~FSR_Mesh() {}

	bool LoadFromObjFile(const char* fileName, const char* mtlBaseDir);
	void Purge();

public:
	struct FSR_SubMesh
	{
		// offset into the global index buffer
		uint32_t	_IndexOffset = 0u;
		// How many indices this mesh contains. Number of triangles therefore equals (m_IdxCount / 3)
		uint32_t	_IndexCount = 0u;
		// index of material
		uint32_t	_MaterialIndex = SR_INVALID_INDEX;
	};

	std::vector<FSRVertex>	_VertexBuffer;
	std::vector<uint32_t>	_IndexBuffer;
	std::vector<std::shared_ptr<FSR_Material>>	_Materials;

	std::vector<FSR_SubMesh>	_SubMeshes;
};


