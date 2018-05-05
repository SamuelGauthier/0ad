/* Copyright (C) 2018 Wildfire Games.
 *
 * 0 A.D. is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * 0 A.D. is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with 0 A.D.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef INCLUDED_GRIDPROJECTOR
#define INCLUDED_GRIDPROJECTOR

#include "graphics/Camera.h"
#include "graphics/ShaderProgramPtr.h"

#include "maths/Matrix3D.h"
#include "maths/Plane.h"

#include "ps/CLogger.h"

#include "renderer/FFTWaterModel.h"
//#include "renderer/VertexArray.h"
#include "renderer/VertexBuffer.h"
#include "renderer/OceanWater.h"
#include "renderer/ProjectionSystem.h"
//#include "renderer/Water.h"

class CGridProjector : public CProjectionSystem
{
public:
	using WaterProperties = CFFTWaterModel::SFFTWaterProperties;

	CGridProjector();
	~CGridProjector();

	void Render(CShaderProgramPtr& shader) override;

	void Initialize() override;

private:
	void GenerateVertices();
	void GenerateIndices();
	void UpdateMatrices();
	void ComputeIntersection(std::vector<CVector4D>& cam_frustrum, std::vector<CVector4D>& span_buffer, CPlane& maxWater, CPlane& minWater, int start, int end);
    void CreateTextures();


	float m_time;
	uint m_resolutionX;
	uint m_resolutionY;
	u32 m_totalResolution;
	COceanWater m_water;

	CCamera m_PCamera;
	CVertexBuffer::VBChunk* m_gridVBIndices;
	CVertexBuffer::VBChunk* m_gridVBVertices;

	CMatrix3D m_Mpiview;
	CMatrix3D m_Miperspective;
	CMatrix3D m_Mrange;
	CMatrix3D m_Mprojector;

	std::vector<GLuint> m_indices;
	std::vector<CVector4D> m_vertices;
	std::vector<CVector4D> m_verticesModel;
	
	std::vector<GLuint> m_heightMapsID;
	std::vector<GLuint> m_normalMapsID;
    GLuint m_variationMapID;
    GLuint m_flowMapID;

};

#endif // !INCLUDED_GRIDPROJECTOR
