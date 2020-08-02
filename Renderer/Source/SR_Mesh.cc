// \brief
//		mesh implementation
//

#include <map>
#include <string>

#include "SR_Mesh.h"
#include "tiny_obj_loader.h"


void FSR_Mesh::Purge()
{
	_VertexBuffer.clear();
	_IndexBuffer.clear();
	_Materials.clear();
	_SubMeshes.clear();
}

bool FSR_Mesh::LoadFromObjFile(const char* fileName, const char* mtlBaseDir)
{
	// clear current data
	Purge();

	// load obj file
	tinyobj::attrib_t attribs;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::map<std::string, std::shared_ptr<FSR_Texture2D>> textures;
	std::string err = "";

	bool ret = tinyobj::LoadObj(&attribs, &shapes, &materials, nullptr, &err, fileName, mtlBaseDir, true /*triangulate*/, true /*default_vcols_fallback*/);
	if (ret)
	{
		// Process materials to load images
		{
			for (unsigned i = 0; i < materials.size(); i++)
			{
				const tinyobj::material_t& m = materials[i];

				std::string diffuseTexName = m.diffuse_texname;
				assert(!diffuseTexName.empty() && "Mesh missing texture!");

				if (textures.find(diffuseTexName) == textures.end())
				{
					std::shared_ptr<FSR_Texture2D> diffuseTex = FSR_Buffer2D_Helper::LoadImageFile((mtlBaseDir + diffuseTexName).c_str());
					assert(diffuseTex != nullptr && "Failed to load image!");

					textures[diffuseTexName] = diffuseTex;
				}

				std::shared_ptr<FSR_Material> NewMaterial = std::make_shared<FSR_Material>();
				NewMaterial->_diffuse_tex = textures[diffuseTexName];

				_Materials.push_back(NewMaterial);
			} // end for i
		}

		// Process vertices
		{
			// POD of indices of vertex data provided by tinyobjloader, used to map unique vertex data to indexed primitive
			struct IndexedPrimitive
			{
				uint32_t PosIdx;
				uint32_t NormalIdx;
				uint32_t UVIdx;

				bool operator<(const IndexedPrimitive& other) const
				{
					return memcmp(this, &other, sizeof(IndexedPrimitive)) > 0;
				}
			};

			std::map<IndexedPrimitive, uint32_t> indexedPrims;
			for (size_t s = 0; s < shapes.size(); s++)
			{
				const tinyobj::shape_t& shape = shapes[s];

				uint32_t meshIdxBase = _IndexBuffer.size();
				for (size_t i = 0; i < shape.mesh.indices.size(); i++)
				{
					auto index = shape.mesh.indices[i];

					// Fetch indices to construct an IndexedPrimitive to first look up existing unique vertices
					int vtxIdx = index.vertex_index;
					assert(vtxIdx != -1);

					bool hasNormals = index.normal_index != -1;
					bool hasUV = index.texcoord_index != -1;

					int normalIdx = index.normal_index;
					int uvIdx = index.texcoord_index;

					IndexedPrimitive prim;
					prim.PosIdx = vtxIdx;
					prim.NormalIdx = hasNormals ? normalIdx : SR_INVALID_INDEX;
					prim.UVIdx = hasUV ? uvIdx : SR_INVALID_INDEX;

					auto res = indexedPrims.find(prim);
					if (res != indexedPrims.end())
					{
						// Vertex is already defined in terms of POS/NORMAL/UV indices, just append index data to index buffer
						_IndexBuffer.push_back(res->second);
					}
					else
					{
						// New unique vertex found, get vertex data and append it to vertex buffer and update indexed primitives
						auto newIdx = _VertexBuffer.size();
						indexedPrims[prim] = newIdx;
						_IndexBuffer.push_back(newIdx);

						auto vx = attribs.vertices[3 * index.vertex_index];
						auto vy = attribs.vertices[3 * index.vertex_index + 1];
						auto vz = attribs.vertices[3 * index.vertex_index + 2];

						glm::vec3 pos(vx, vy, vz);

						glm::vec3 normal(0.f);
						if (hasNormals)
						{
							auto nx = attribs.normals[3 * index.normal_index];
							auto ny = attribs.normals[3 * index.normal_index + 1];
							auto nz = attribs.normals[3 * index.normal_index + 2];

							normal.x = nx;
							normal.y = ny;
							normal.z = nz;
						}

						glm::vec3 uv(0.f, 0.f, 1.f);
						if (hasUV)
						{
							auto ux = attribs.texcoords[2 * index.texcoord_index];
							auto uy = 1.f - attribs.texcoords[2 * index.texcoord_index + 1];

							uv.s = glm::abs(ux);
							uv.t = glm::abs(uy);
						}

						FSRVertex uniqueVertex;
						uniqueVertex._vertex = pos;
						uniqueVertex._attributes._members[SRMESH_NORMAL_ATTRIB] = normal;
						uniqueVertex._attributes._members[SRMESH_UV_ATTRIB] = uv;
						uniqueVertex._attributes._count = 2;

						_VertexBuffer.push_back(uniqueVertex);
					}
				} // end for mesh

				// Push new mesh to be rendered in the scene 
				FSR_SubMesh mesh;
				mesh._IndexOffset = meshIdxBase;
				mesh._IndexCount = shape.mesh.indices.size();

				assert((shape.mesh.material_ids[0] != -1) && "Mesh missing a material!");
				mesh._MaterialIndex = shape.mesh.material_ids[0]; // No per-face material but fixed one

				_SubMeshes.push_back(mesh);
			} // end for s

			// sort by material
			std::sort(_SubMeshes.begin(), _SubMeshes.end(), [](const FSR_SubMesh& lhs, const FSR_SubMesh& rhs) -> bool { return lhs._MaterialIndex < rhs._MaterialIndex; });
		}
	}
	else
	{
		printf("ERROR: %s\n", err.c_str());
		assert(false && "Failed to load .OBJ file, check file paths!");
		return false;
	}

	return true;
}

