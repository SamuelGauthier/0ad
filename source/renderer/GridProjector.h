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

	//void SetReflectionMatrix(CMatrix3D reflectionMatrix) { m_reflectionMatrix = reflectionMatrix; }
    //void SetReflectionTransMatrix(CMatrix3D reflectionTransMatrix)
    //void SetReflectionCamPos(CVector3D position) { m_reflectionCamPos = position; }
    //void SetReflectionLookAt(CVector3D lookAt) { m_reflectionLookAt = lookAt; }
    //void SetReflectionFarClip(CPlane farClippingPlane) { m_reflectionFarClip = farClippingPlane; }
    void SetReflectionCamera(CCamera reflectionCamera) { m_reflectionCam = reflectionCamera; }
    void SetRefractionCamera(CCamera refractionCamera) { m_refractionCam = refractionCamera; }
    void SetEntireSceneCamera(CCamera entireSceneCamera) { m_entireSceneCam = entireSceneCamera; }
    
    float GetMaxWaterHeight() { return m_water.GetMaxWaterHeight(); }
    float GetMinWaterHeight() { return m_water.GetMinWaterHeight(); }
    float GetWaterHeight() { return m_water.GetWaterHeight(); }

    int GetReflectionTexWidth() { return m_reflectionTexSizeW; }
    int GetReflectionTexHeight() { return m_reflectionTexSizeH; }
	int GetRefractionTexWidth() { return m_reflectionTexSizeW; }
	int GetRefractionTexHeight() { return m_reflectionTexSizeH; }

	GLuint GetReflectionFBOID() { return m_reflectionFBOID; }
    //CVector3D GetReflectionLookAt() { return m_reflectionLookAt; }
    CPlane GetReflectionFarClip() { return m_reflectionFarClip; }
	GLuint GetRefractionFBOID() { return m_refractionFBOID; }
	GLuint GetEntireSceneFBOID() { return m_entireSceneFBOID; }

private:
	void GenerateVertices();
	void GenerateIndices();
	void UpdateMatrices();
	void ComputeIntersection(std::vector<CVector4D>& cam_frustrum, std::vector<CVector4D>& span_buffer, CPlane& maxWater, CPlane& minWater, int start, int end);
    void CreateTextures();
    void UpdateReflectionFarPlane();
    void UpdateRefractionFarPlane();
	void UpdateFarPlane();
	void CreateTerrainTexture();


	float m_time;
	uint m_resolutionX;
	uint m_resolutionY;
	uint m_totalResolution;
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
	GLuint m_reflectionFBOID;
    GLuint m_reflectionDepthBufferID;
	GLuint m_reflectionID;
	GLuint m_refractionFBOID;
    GLuint m_refractionDepthBufferID;
	GLuint m_refractionID;
    GLuint m_entireSceneFBOID;
    GLuint m_entireSceneDepthBufferID;
    GLuint m_entireSceneID;
	GLuint m_terrainID;

	int m_reflectionTexSizeW;
    int m_reflectionTexSizeH;
	int m_terrainWorldSize;

	float m_maxTerrainElevation;
	float m_minTerrainElevation;

    CCamera m_reflectionCam;
    CPlane m_reflectionFarClip;
    CCamera m_refractionCam;
    CPlane m_refractionFarClip;
    CCamera m_entireSceneCam;
	CPlane m_farClip;
};

#endif // !INCLUDED_GRIDPROJECTOR
