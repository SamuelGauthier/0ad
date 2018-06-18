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
#include "renderer/VertexBuffer.h"
#include "renderer/OceanWater.h"
#include "renderer/ProjectionSystem.h"

class CGridProjector : public CProjectionSystem
{
public:
	using WaterProperties = CFFTWaterModel::SFFTWaterProperties;

    /** @brief Creates a grid projector */
	CGridProjector();

    /** @brief Destructor */
	~CGridProjector();

	/**
	 *  @brief Render the projected grid
	 *  @param shader the shader program
	 */
	void Render(CShaderProgramPtr& shader) override;

	/**
	 *	@brief Intializes the projected grid.
	 */
	void Initialize() override;

    /**
     * @brief Sets the refelction camera to the one given in arguments.
     * * @param reflectionCamera the reflection camera
     */
    void SetReflectionCamera(CCamera reflectionCamera) { m_reflectionCam = reflectionCamera; }

    /**
     * @brief Sets the refraction camera to the one given in arguments.
     * @param refractionCamera the refraction camera
     */
    void SetRefractionCamera(CCamera refractionCamera) { m_refractionCam = refractionCamera; }

    /**
     * @brief Sets the camera for the entire scene capture
     * @param entireSceneCamera the entire scene camera
     */
    void SetEntireSceneCamera(CCamera entireSceneCamera) { m_entireSceneCam = entireSceneCamera; }
    
    /**
     * @brief Get the height of the water
     * @return returns the height of the water
     */
    float GetWaterHeight() { return m_water.GetWaterHeight(); }

    /**
     * @brief Get the width of the reflection texture
     * @return returns the width of the reflection texture
     */
    int GetReflectionTexWidth() { return m_reflectionTexSizeW; }

    /**
     * @brief Get the height of the reflection texture
     * @return returns the height of the reflection texture
     */
    int GetReflectionTexHeight() { return m_reflectionTexSizeH; }

    /**
     * @brief Get the width of the refraction texture
     * @return returns the width of the refraction texture
     */
	int GetRefractionTexWidth() { return m_reflectionTexSizeW; }

    /**
     * @brief Get the height of the refraction texture
     * @return returns the height of the refraction texture
     */
	int GetRefractionTexHeight() { return m_reflectionTexSizeH; }

    /**
     * @brief 
     * @return 
     */
	GLuint GetReflectionFBOID() { return m_reflectionFBOID; }

    /**
     * @brief 
     * @return 
     */
	GLuint GetRefractionFBOID() { return m_refractionFBOID; }

    /**
     * @brief 
     * @return 
     */
	GLuint GetEntireSceneFBOID() { return m_entireSceneFBOID; }

private:

    /**
     * @brief Generates a grid of vertices in screen space
     */
	void GenerateVertices();

    /**
     * @brief Generates the indices corresponding to the grid
     */
	void GenerateIndices();

    /**
     * @brief Updates the transformation matrices used by the grid projector.
     */
	void UpdateMatrices();

    /**
     * @brief Computes the intersection between the camera frustum and the min
     * max water planes. Adds the intersection to the \p span_buffer
     * @param cam_frustrum the camera frustum
     * @param span_buffer buffer where the intersections are added
     * @param maxWater max water plane
     * @param minWater min water plane
     * @param start start index
     * @param end end index
     */
	void ComputeIntersection(std::vector<CVector4D>& cam_frustrum, std::vector<CVector4D>& span_buffer, CPlane& maxWater, CPlane& minWater, int start, int end);

    /**
     * @brief Creates all the textures need by the grid projector
     */
    void CreateTextures();

    /**
     * @brief Updates the reflection far plane needed for the shader
     */
    void UpdateReflectionFarPlane();

    /**
     * @brief Updates the refraction far plane needed for the shader
     */
    void UpdateRefractionFarPlane();

    /**
     * @brief Updates the refraction far plane needed for the shader
     */
	void UpdateFarPlane();

    /**
     * @brief Creates a texture holding the height of the entire terrain.
     */
	void CreateTerrainTexture();


	float m_time;
	uint m_resolutionX;
	uint m_resolutionY;
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
	
	std::vector<GLuint> m_vectorDisplacementFieldsID;
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
