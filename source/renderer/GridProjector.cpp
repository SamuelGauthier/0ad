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

/*
 * Based on Claes Johanson's Master Thesis "Real-time water rendering
 * Introducing the projected grid concept", 2004 and on Yury Kryachko's SIGGRAPH
 * 2016 (http://wargaming.com/en/news/siggraph-2016/) and GDC18 talks
 * (https://www.gdcvault.com/play/1025404/-World-of-Warships-Technical)
 */

#include "precompiled.h"

#include "GridProjector.h"

#include "graphics/LightEnv.h"
#include "graphics/LOSTexture.h"
#include "graphics/Terrain.h"
#include "graphics/ShaderManager.h"
#include "graphics/ShaderProgram.h"

#include "lib/ogl.h"
#include "lib/timer.h"

#include "maths/MathUtil.h"
#include "maths/Vector3D.h"
#include "maths/Vector2D.h"
#include "maths/Vector4D.h"

#include "ps/Game.h"
#include "ps/World.h"

#include "renderer/Renderer.h"
#include "renderer/SkyManager.h"
#include "renderer/VertexBuffer.h"
#include "renderer/VertexBufferManager.h"

#define DEBUG_COMPUTE_INTERSECTION 0
#define DEBUG_UPDATE_MATRICES_CAMERA_INFOS 0
#define DEBUG_UPDATE_MATRICES_PROJECTOR_INFOS 0
#define DEBUG_UPDATE_MATRICES_PROJECTED_POINT 0
#define DEBUG_UPDATE_MATRICES_SPAN_BUFFER 0
#define DEBUG_UPDATE_MATRICES_FRUSTRUM_SPAN_BUFFER 0
#define DEBUG_UPDATE_MATRICES_MIN_MAX_INTER 0
#define DEBUG_UPDATE_MATRICES_PROJECTOR 0
#define DEBUG_UPDATE_MATRICES_FRUSTRUM_POINTS_INFO 0
#define DEBUG_UPDATE_MATRICES_INV_CAMERA_INFOS 0
#define DEBUG_UPDATE_MATRICES_WATER_PLANE_INFO 0
#define DEBUG_RENDER_WORLD_POS 0
#define DEBUG_COMPUTE_INTERSECTION_MINMAXINTER 0
#define DEBUG_LEGEND 0
#define DEBUG_UPDATE_POINTS 0
#define DEBUG_UPDATE_POINTS_INTERSECT_INFO 0


double TIME = 3.2;
//CGridProjector::WaterProperties coarseWaves = CGridProjector::WaterProperties(3e-7f, 43, CVector2D(1.0f, 5.0f), 1.0f, 0.1f, 2048, 4000, TIME);
//CGridProjector::WaterProperties mediumWaves = CGridProjector::WaterProperties(6e-7f, 20, CVector2D(4.0f, 1.5f), 2.0f, 0.1f, 2048, 5000, TIME);
//CGridProjector::WaterProperties detailedWaves = CGridProjector::WaterProperties(6e-7f, 20, CVector2D(1.0f, 1.5f), 1.0f, 0.1f, 2048, 8000, TIME);
CGridProjector::WaterProperties coarseWaves = CGridProjector::WaterProperties(3e-7f, 43, CVector2D(1.0f, 5.0f), 1.0f, 0.1f, 2048, 1000, TIME);
CGridProjector::WaterProperties mediumWaves = CGridProjector::WaterProperties(6e-7f, 50, CVector2D(4.0f, -1.5f), 5.0f, 0.1f, 2048, 2500, TIME);
CGridProjector::WaterProperties detailedWaves = CGridProjector::WaterProperties(6e-7f, 30, CVector2D(-1.0f, 1.5f), 1.0f, 0.1f, 2048, 800, TIME);
//std::vector<CGridProjector::WaterProperties> wps = { coarseWaves, mediumWaves, detailedWaves };
std::vector<CGridProjector::WaterProperties> wps = { coarseWaves};

CGridProjector::CGridProjector() : m_water(CFFTWaterModel(wps)), m_gridVBIndices(0), m_gridVBVertices(0)
{
	m_time = 0.0;

    m_resolutionX = 0;
    m_resolutionY = 0;
    m_totalResolution = 0;

	m_PCamera = CCamera();

	m_Mpiview = CMatrix3D();
	m_Miperspective = CMatrix3D();
	m_Mrange = CMatrix3D();
	m_Mprojector = CMatrix3D();
    
    m_normalMapsID = std::vector<GLuint>(3);
    m_heightMapsID = std::vector<GLuint>(3);
	m_reflectionFBOID = 0;
    m_reflectionDepthBufferID = 0;
	m_reflectionID = 0;
	m_refractionFBOID = 0;
	m_refractionDepthBufferID = 0;
	m_refractionID = 0;
	m_entireSceneFBOID = 0;
	m_entireSceneDepthBufferID = 0;
	m_entireSceneID = 0;

	m_reflectionTexSizeW = 0;
    m_reflectionTexSizeH = 0;
}

CGridProjector::~CGridProjector()
{
	if (m_gridVBIndices) g_VBMan.Release(m_gridVBIndices);
	if (m_gridVBVertices) g_VBMan.Release(m_gridVBVertices);
    
    glDeleteTextures(m_heightMapsID.size(), &m_heightMapsID[0]);
    glDeleteTextures(m_normalMapsID.size(), &m_normalMapsID[0]);
    glDeleteTextures(1, &m_variationMapID);
    glDeleteTextures(1, &m_flowMapID);
	glDeleteTextures(1, &m_reflectionID);
    glDeleteTextures(1, &m_reflectionDepthBufferID);
    glDeleteTextures(1, &m_refractionID);
    glDeleteTextures(1, &m_refractionDepthBufferID);
	glDeleteTextures(1, &m_entireSceneID);
	glDeleteTextures(1, &m_entireSceneDepthBufferID);

	pglDeleteFramebuffersEXT(1, &m_reflectionFBOID);
	pglDeleteFramebuffersEXT(1, &m_refractionFBOID);
	pglDeleteFramebuffersEXT(1, &m_entireSceneFBOID);
}

void CGridProjector::Initialize()
{
	if(m_gridVBIndices)
	{
		g_VBMan.Release(m_gridVBIndices);
		m_gridVBIndices = 0;
	}
	if(m_gridVBVertices)
	{
		g_VBMan.Release(m_gridVBVertices);
		m_gridVBVertices = 0;
	}

    // ATM only possible to have a grid of 65526 vertices
    // Limitation due to constraints 16 bit indices constraints
    // TODO: split the grid into the right amount of sub-grids
    m_resolutionX = 128;//g_Renderer.GetWidth()/4;
    m_resolutionY = 512;//g_Renderer.GetHeight()/2;
    m_totalResolution = m_resolutionX * m_resolutionY;
    
	GenerateVertices();
	
	m_gridVBVertices = g_VBMan.Allocate(sizeof(CVector4D), m_vertices.size(), GL_STATIC_DRAW, GL_ARRAY_BUFFER);

	if (m_gridVBVertices == NULL || m_gridVBVertices == 0)
		LOGERROR("No storage available for water vertices!");
	
	GenerateIndices();
	
	m_gridVBIndices = g_VBMan.Allocate(sizeof(GLuint), m_indices.size(), GL_STATIC_DRAW, GL_ELEMENT_ARRAY_BUFFER);
	
	if (m_gridVBIndices == NULL || m_gridVBIndices == 0)
		LOGERROR("No storage available for water indices!");
	
	m_gridVBVertices->m_Owner->UpdateChunkVertices(m_gridVBVertices, &m_vertices[0]);
	
	m_gridVBIndices->m_Owner->UpdateChunkVertices(m_gridVBIndices, &m_indices[0]);

    m_water.GenerateWaterWaves();
    m_water.GenerateVariationMap();
    CreateTextures();
}

void CGridProjector::GenerateVertices()
{
	PROFILE3_GPU("generate vertices");
	if (m_verticesModel.size() != 0)
	{
		m_vertices = m_verticesModel;
		return;
	}

	float xRatio = 2.0 / (m_resolutionX - 1); // 200 for world debug size
	float yRatio = 2.0 / (m_resolutionY - 1); // 200 for world debug size
	
	for (uint j = 0; j < m_resolutionY; j++)
		for (uint i = 0; i <  m_resolutionX; i++)
			 m_verticesModel.push_back(CVector4D(i * xRatio - 1.0, 1.0 - j * yRatio, 0.0, 1.0));

	m_vertices = m_verticesModel;
}

void CGridProjector::GenerateIndices()
{
	m_indices.clear();

	for (uint j = 0; j < (m_resolutionY - 1); j++)
	{
		for (uint i = 0; i < (m_resolutionX - 1); i++)
		{
			// *-*
			// |/
			// *
			m_indices.push_back(j * m_resolutionX + i);
			m_indices.push_back((j + 1) * m_resolutionX + i);
			m_indices.push_back(j * m_resolutionX + (i + 1));
			//   *
			//  /|
			// *-*
			m_indices.push_back(j * m_resolutionX + (i + 1));
			m_indices.push_back((j + 1) * m_resolutionX + i);
			m_indices.push_back((j + 1) * m_resolutionX + (i + 1));
		}

	}
}

void CGridProjector::UpdateMatrices()
{
	PROFILE3_GPU("update matrices");
	// TODO: Temporary here, dependecy to WaterManager should be cut by implementing a couple of CCmpOceanWater classes.
    //float waterHeight = g_Renderer.GetWaterManager()->m_WaterHeight;
	//m_water.SetWaterHeight(waterHeight);
    m_water.UpdateWaterHeight();
	
	CCamera g_RCamera = g_Renderer.GetViewCamera();
	CMatrix3D invV = g_RCamera.GetOrientation();
	CVector3D camPosition = invV.GetTranslation();// (invV._14, invV._24, invV._34);
	CVector3D camDirection = invV.GetIn();// (invV._13, invV._23, invV._33);
	CVector3D camUp = invV.GetUp();// (invV._12, invV._22, invV._32);

	CVector3D projPosition = camPosition;
	CVector3D projDirection = camDirection;
	CVector3D projUp = camUp;

	CVector3D intersection;

	bool gotWater = m_water.GetWaterBase().FindRayIntersection(camPosition, camDirection, &intersection);

	ssize_t mapSize = g_Game->GetWorld()->GetTerrain()->GetVerticesPerSide();
	if (gotWater)
	{
		intersection.X = clamp(intersection.X, 0.f, (float)((mapSize - 1)*TERRAIN_TILE_SIZE));
		intersection.Z = clamp(intersection.Z, 0.f, (float)((mapSize - 1)*TERRAIN_TILE_SIZE));
	}

	// Shift the projector upwards out of the displacable water volume
	if (camPosition.Y <= intersection.Y)
		projPosition.Y = intersection.Y + m_water.GetMaxWaterHeight();

	projDirection = intersection - projPosition;

#if DEBUG_UPDATE_MATRICES_PROJECTOR_INFOS
	LOGWARNING("Camera position: (%f, %f, %f)", camPosition.X, camPosition.Y, camPosition.Z);
	LOGWARNING("Projector position: (%f, %f, %f)", projPosition.X, projPosition.Y, projPosition.Z);
#endif

	m_PCamera.LookAlong(projPosition, projDirection, projUp);
	m_PCamera.SetProjection(g_RCamera.GetNearPlane(), g_RCamera.GetFarPlane(), g_RCamera.GetFOV());
	m_PCamera.UpdateFrustum();

	m_Mpiview = m_PCamera.GetOrientation();
	
	g_RCamera.GetProjection().GetInverse(m_Miperspective);

#if DEBUG_UPDATE_MATRICES_CAMERA_INFOS
	CMatrix3D gR_PV = g_RCamera.GetViewProjection();
	CMatrix3D gP_PV = m_PCamera.GetViewProjection();
	LOGWARNING("Near plane at: %f", g_RCamera.GetNearPlane());
	LOGWARNING("Far plane at: %f", g_RCamera.GetFarPlane());
	LOGWARNING("Projector Near plane at: %f", m_PCamera.GetNearPlane());
	LOGWARNING("Projector Far plane at: %f", m_PCamera.GetFarPlane());
	LOGWARNING("camera PV:");
	LOGWARNING("%f, %f, %f, %f", gR_PV._11, gR_PV._21, gR_PV._31, gR_PV._41);
	LOGWARNING("%f, %f, %f, %f", gR_PV._12, gR_PV._22, gR_PV._32, gR_PV._42);
	LOGWARNING("%f, %f, %f, %f", gR_PV._13, gR_PV._23, gR_PV._33, gR_PV._43);
	LOGWARNING("%f, %f, %f, %f", gR_PV._14, gR_PV._24, gR_PV._34, gR_PV._44);
	LOGWARNING("projector PV:");
	LOGWARNING("%f, %f, %f, %f", gP_PV._11, gP_PV._21, gP_PV._31, gP_PV._41);
	LOGWARNING("%f, %f, %f, %f", gP_PV._12, gP_PV._22, gP_PV._32, gP_PV._42);
	LOGWARNING("%f, %f, %f, %f", gP_PV._13, gP_PV._23, gP_PV._33, gP_PV._43);
	LOGWARNING("%f, %f, %f, %f", gP_PV._14, gP_PV._24, gP_PV._34, gP_PV._44);
#endif

	// Compute Mrange conversion matrix
	// Project frustrum corner points into world coordinates
	
	CPlane water = m_water.GetWaterBase();
	std::vector<CVector4D> span_buffer;
	std::vector<CVector4D> cam_frustrum;
	cam_frustrum.push_back(CVector4D(+1, +1, +1, 1));
	cam_frustrum.push_back(CVector4D(+1, +1, -1, 1));
	cam_frustrum.push_back(CVector4D(+1, -1, +1, 1));
	cam_frustrum.push_back(CVector4D(+1, -1, -1, 1));
	cam_frustrum.push_back(CVector4D(-1, +1, +1, 1));
	cam_frustrum.push_back(CVector4D(-1, +1, -1, 1));
	cam_frustrum.push_back(CVector4D(-1, -1, +1, 1));
	cam_frustrum.push_back(CVector4D(-1, -1, -1, 1));


	CMatrix3D invP;
	g_RCamera.GetProjection().GetInverse(invP);

	CMatrix3D invPV = invV * invP;
	//CMatrix3D invPV = m_Mpiview * m_Miperspective;
	
#if DEBUG_UPDATE_MATRICES_INV_CAMERA_INFOS
	CMatrix3D ginvPV;
	g_Renderer.GetViewCamera().GetViewProjection().GetInverse(ginvPV);
	
	LOGWARNING("computed inverse PV:");
	LOGWARNING("%f, %f, %f, %f", invPV._11, invPV._21, invPV._31, invPV._41);
	LOGWARNING("%f, %f, %f, %f", invPV._12, invPV._22, invPV._32, invPV._42);
	LOGWARNING("%f, %f, %f, %f", invPV._13, invPV._23, invPV._33, invPV._43);
	LOGWARNING("%f, %f, %f, %f", invPV._14, invPV._24, invPV._34, invPV._44);
	LOGWARNING("render inverse PV:");
	LOGWARNING("%f, %f, %f, %f", ginvPV._11, ginvPV._21, ginvPV._31, ginvPV._41);
	LOGWARNING("%f, %f, %f, %f", ginvPV._12, ginvPV._22, ginvPV._32, ginvPV._42);
	LOGWARNING("%f, %f, %f, %f", ginvPV._13, ginvPV._23, ginvPV._33, ginvPV._43);
	LOGWARNING("%f, %f, %f, %f", ginvPV._14, ginvPV._24, ginvPV._34, ginvPV._44);
#endif
	
	float maxWaterHeight = m_water.GetMaxWaterHeight();
	float minWaterHeight = m_water.GetMinWaterHeight();

	CVector4D result;
	for (size_t i = 0; i < cam_frustrum.size(); i++)
	{
		result = invPV.Transform(cam_frustrum[i]);
		cam_frustrum[i] = result * 1/result.W;
		
		float distance = water.DistanceToPlane(CVector3D(cam_frustrum[i].X, cam_frustrum[i].Y, cam_frustrum[i].Z));//W instead of Z???
		
		if(distance < maxWaterHeight && distance > minWaterHeight)
			span_buffer.push_back(cam_frustrum[i]);
	}
#if DEBUG_UPDATE_MATRICES_FRUSTRUM_SPAN_BUFFER
	LOGWARNING("span_buffer size %u", span_buffer.size());
#endif
	
#if DEBUG_UPDATE_MATRICES_FRUSTRUM_POINTS_INFO
	for (size_t i = 0; i < cam_frustrum.size(); i++)
	{
		LOGWARNING("[%u] %f, %f, %f, %f", i, cam_frustrum[i].X, cam_frustrum[i].Y, cam_frustrum[i].Z, cam_frustrum[i].W);
	}
#endif
	
	// Check the intersections of the camera frustrum planes and the max/min water planes
	CPlane maxWater;
	CPlane minWater;
	// d = -(ax0 +by0+cz0)
	maxWater.Set(water.m_Norm, CVector3D(0, -water.m_Dist + maxWaterHeight, 0));
	minWater.Set(water.m_Norm, CVector3D(0, -water.m_Dist + minWaterHeight, 0));

#if DEBUG_UPDATE_MATRICES_WATER_PLANE_INFO
	LOGWARNING("water n:(%f, %f, %f), d:%f", water.m_Norm.X, water.m_Norm.Y, water.m_Norm.Z, water.m_Dist);
	LOGWARNING("maxwater n:(%f, %f, %f), d:%f", maxWater.m_Norm.X, maxWater.m_Norm.Y, maxWater.m_Norm.Z, maxWater.m_Dist);
	LOGWARNING("minwater n:(%f, %f, %f), d:%f", minWater.m_Norm.X, minWater.m_Norm.Y, minWater.m_Norm.Z, minWater.m_Dist);
#endif
	
	ComputeIntersection(cam_frustrum, span_buffer, maxWater, minWater, 1, 0);
	ComputeIntersection(cam_frustrum, span_buffer, maxWater, minWater, 5, 4);
	ComputeIntersection(cam_frustrum, span_buffer, maxWater, minWater, 3, 2);
	ComputeIntersection(cam_frustrum, span_buffer, maxWater, minWater, 7, 6);
	ComputeIntersection(cam_frustrum, span_buffer, maxWater, minWater, 1, 5);
	ComputeIntersection(cam_frustrum, span_buffer, maxWater, minWater, 3, 7);
	ComputeIntersection(cam_frustrum, span_buffer, maxWater, minWater, 4, 0);
	ComputeIntersection(cam_frustrum, span_buffer, maxWater, minWater, 6, 2);
	ComputeIntersection(cam_frustrum, span_buffer, maxWater, minWater, 1, 3);
	ComputeIntersection(cam_frustrum, span_buffer, maxWater, minWater, 5, 7);
	ComputeIntersection(cam_frustrum, span_buffer, maxWater, minWater, 4, 6);
	ComputeIntersection(cam_frustrum, span_buffer, maxWater, minWater, 0, 2);

	if(span_buffer.size() == 0) return;
	
	// Project the intersections onto the base water plane
	CVector3D start;
	CVector3D direction = -water.m_Norm;
	CVector3D projection;
	CMatrix3D viewProjection = m_PCamera.GetViewProjection();
	CVector4D transformed;
	CVector2D current;
	float x_max = -FLT_MAX;
	float x_min = +FLT_MAX;
	float y_max = -FLT_MAX;
	float y_min = +FLT_MAX;
	
	for (size_t i = 0; i < span_buffer.size(); i++)
	{
		start = CVector3D(span_buffer[i].X, span_buffer[i].Y, span_buffer[i].Z);
		water.FindRayIntersection(start, direction, &projection);

#if DEBUG_UPDATE_MATRICES_PROJECTED_POINT
		LOGWARNING("[W] projected point on water: (%f, %f, %f)", projection.X, projection.Y, projection.Z);
#endif
		
		// Transform the points into projector space
		transformed = viewProjection.Transform(CVector4D(projection.X, projection.Y, projection.Z, 1));
		span_buffer[i] = transformed / transformed.W;
		current = CVector2D(span_buffer[i].X, span_buffer[i].Y);

#if DEBUG_UPDATE_MATRICES_SPAN_BUFFER
		LOGWARNING("span buffer: (%f, %f)", span_buffer[i].X, span_buffer[i].Y);
#endif

		if (current.X > x_max) x_max = current.X;
		if (current.X < x_min) x_min = current.X;
		if (current.Y > y_max) y_max = current.Y;
		if (current.Y < y_min) y_min = current.Y;
	}

#if DEBUG_UPDATE_MATRICES_MIN_MAX_INTER
	LOGWARNING("x_max: %f, x_min: %f, y_max: %f, y_min: %f", x_max, x_min, y_max, y_min);
	LOGWARNING("x_max-x_min: %f, y_max-y_min: %f", x_max-x_min, y_max-y_min);
#endif
	
	// Create Mrange matrix
	m_Mrange = CMatrix3D(x_max - x_min, 0,			   0, x_min,
						 0,				y_max - y_min, 0, y_min,
						 0,				0,			   1, 0,
						 0,				0,			   0, 1);
	//m_Mrange.SetIdentity();

	m_Mprojector = m_Mpiview * m_Miperspective * m_Mrange;
	
#if DEBUG_UPDATE_MATRICES_PROJECTOR
	LOGWARNING("m_Mprojector:");
	LOGWARNING("%f, %f, %f, %f", m_Mprojector._11, m_Mprojector._21, m_Mprojector._31, m_Mprojector._41);
	LOGWARNING("%f, %f, %f, %f", m_Mprojector._12, m_Mprojector._22, m_Mprojector._32, m_Mprojector._42);
	LOGWARNING("%f, %f, %f, %f", m_Mprojector._13, m_Mprojector._23, m_Mprojector._33, m_Mprojector._43);
	LOGWARNING("%f, %f, %f, %f", m_Mprojector._14, m_Mprojector._24, m_Mprojector._34, m_Mprojector._44);
#endif
}

void CGridProjector::ComputeIntersection(std::vector<CVector4D>& cam_frustrum, std::vector<CVector4D>& span_buffer, CPlane& maxWater, CPlane& minWater, int startIndice, int endIndice)
{
	CVector3D intMax;
	CVector3D intMin;
	CVector3D start = CVector3D(cam_frustrum[startIndice].X, cam_frustrum[startIndice].Y, cam_frustrum[startIndice].Z);
	CVector3D end = CVector3D(cam_frustrum[endIndice].X, cam_frustrum[endIndice].Y, cam_frustrum[endIndice].Z);
	bool maxIntersect = maxWater.FindLineSegIntersection(start, end, &intMax);
	bool minIntersect = minWater.FindLineSegIntersection(start, end, &intMin);
	if (maxIntersect)
	{
		span_buffer.push_back(CVector4D(intMax.X, intMax.Y, intMax.Z, 1));
#if DEBUG_COMPUTE_INTERSECTION_MINMAXINTER
		LOGWARNING("[W] %u-%u maxInter: (%f, %f, %f)", startIndice, endIndice, intMax.X, intMax.Y, intMax.Z);
#endif
	}
	if (minIntersect)
	{
		span_buffer.push_back(CVector4D(intMin.X, intMin.Y, intMin.Z, 1));
#if DEBUG_COMPUTE_INTERSECTION_MINMAXINTER
		LOGWARNING("[W] %u-%u minInter: (%f, %f, %f)", startIndice, endIndice, intMin.X, intMin.Y, intMin.Z);
#endif
	}
}

void CGridProjector::Render(CShaderProgramPtr& shader)
{
	if (g_Renderer.m_SkipSubmit)
		return;

#if DEBUG_LEGEND
	LOGWARNING("[S] Screen space [W] world space");
#endif

	m_time = (float)timer_Time();
	UpdateMatrices();
    UpdateReflectionFarPlane();
	UpdateRefractionFarPlane();

	u8* base = m_gridVBVertices->m_Owner->Bind();

	shader->VertexAttribPointer(str_vertexPosition, 4, GL_FLOAT, GL_FALSE, sizeof(CVector4D), base);

#if DEBUG_RENDER_WORLD_POS
	CVector3D worldPos = g_Renderer.GetViewCamera().GetWorldCoordinates(512, 384);
	LOGWARNING("[W] pos: (%f, %f, %f)", worldPos.X, worldPos.Y, worldPos.Z);
#endif

	shader->Uniform(str_MVP, g_Renderer.GetViewCamera().GetViewProjection());
	shader->Uniform(str_invMVP, g_Renderer.GetViewCamera().GetViewProjection().GetInverse());
    shader->Uniform(str_cameraPos, g_Renderer.GetViewCamera().GetOrientation().GetTranslation());
    shader->Uniform(str_invV, g_Renderer.GetViewCamera().GetOrientation());
	shader->Uniform(str_projector, m_Mprojector);
	shader->Uniform(str_waterNormal, m_water.GetWaterBase().m_Norm);
	shader->Uniform(str_waterD, m_water.GetWaterBase().m_Dist);
	shader->Uniform(str_time, m_time);
    
    const CLightEnv& lightEnv = g_Renderer.GetLightEnv();
    shader->Uniform(str_ambient, lightEnv.m_TerrainAmbientColor);
    shader->Uniform(str_sunDir, lightEnv.GetSunDir());
    shader->Uniform(str_sunColor, lightEnv.m_SunColor);
    
    shader->BindTexture(str_heightMap1, m_heightMapsID.at(0));
    shader->BindTexture(str_heightMap2, m_heightMapsID.at(0));
    shader->BindTexture(str_heightMap3, m_heightMapsID.at(0));

    shader->BindTexture(str_normalMap1, m_normalMapsID.at(0));
    shader->BindTexture(str_normalMap2, m_normalMapsID.at(0));
    shader->BindTexture(str_normalMap3, m_normalMapsID.at(0));
    
    shader->BindTexture(str_variationMap, m_variationMapID);

	shader->BindTexture(str_reflectionMap, m_reflectionID);
    //shader->BindTexture(str_reflectionMapDepth, m_reflectionDepthBufferID);
    shader->Uniform(str_reflectionMVP, m_reflectionCam.GetViewProjection());
    shader->Uniform(str_reflectionFarClipN, m_reflectionFarClip.m_Norm);
    shader->Uniform(str_reflectionFarClipD, m_reflectionFarClip.m_Dist);

	shader->BindTexture(str_refractionMap, m_refractionID);
    shader->BindTexture(str_refractionMapDepth, m_refractionDepthBufferID);
	shader->Uniform(str_refractionMVP, m_refractionCam.GetViewProjection());
    shader->Uniform(str_refractionFarClipN, m_refractionFarClip.m_Norm);
    shader->Uniform(str_refractionFarClipD, m_refractionFarClip.m_Dist);
    shader->BindTexture(str_entireSceneDepth, m_entireSceneDepthBufferID);
    
    shader->BindTexture(str_skyCube, g_Renderer.GetSkyManager()->GetSkyCube());

    shader->Uniform(str_screenWidth, g_Renderer.GetWidth());
    shader->Uniform(str_screenHeight, g_Renderer.GetHeight());
	shader->Uniform(str_nearPlane, g_Renderer.GetViewCamera().GetNearPlane());
	shader->Uniform(str_farPlane, g_Renderer.GetViewCamera().GetFarPlane());

	CLOSTexture& losTexture = g_Renderer.GetScene().GetLOSTexture();
	shader->BindTexture(str_losMap, losTexture.GetTextureSmooth());
	shader->Uniform(str_losMatrix, losTexture.GetTextureMatrix());

	shader->AssertPointersBound();
	
	u8* indexBase = m_gridVBIndices->m_Owner->Bind();
	glDrawElements(GL_TRIANGLES, (GLsizei) m_gridVBIndices->m_Count, GL_UNSIGNED_INT, indexBase);
	
	CVertexBuffer::Unbind();
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}


void CGridProjector::CreateTextures()
{
    std::vector<CGridProjector::WaterProperties> waterProperties = m_water.GetWaterModel().GetWaterProperties();
    
    m_heightMapsID = std::vector<GLuint>(waterProperties.size());
    m_normalMapsID = std::vector<GLuint>(waterProperties.size());
   
	// Create the mipmapped height textures
	// Note: the information when looking towards the horizon becomes wrong when using mipmaps
	// see http://www.gdcvault.com/play/1025404/-World-of-Warships-Technical
    std::vector<std::vector<u8>> heightMaps = m_water.GetHeightMaps();
    
    glGenTextures((GLsizei)m_heightMapsID.size(), &m_heightMapsID[0]);
    for (size_t i = 0; i < heightMaps.size(); i++)
    {
        g_Renderer.BindTexture(i, m_heightMapsID.at(i));
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, waterProperties.at(i).m_resolution, waterProperties.at(i).m_resolution, 0, GL_RGB, GL_UNSIGNED_BYTE, &heightMaps.at(i)[0]);
        pglGenerateMipmapEXT(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }
    
	// Create the mipmapped normal map textures
	// Note: the same applies here as the upper comment
    std::vector<std::vector<u8>> normalMaps = m_water.GetNormalMaps();
    
    glGenTextures((GLsizei)m_normalMapsID.size(), &m_normalMapsID[0]);
    for (size_t i = 0; i < normalMaps.size(); i++)
    {
        g_Renderer.BindTexture(i, m_normalMapsID.at(i));
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, waterProperties.at(i).m_resolution, waterProperties.at(i).m_resolution, 0, GL_RGB, GL_UNSIGNED_BYTE, &normalMaps.at(i)[0]);
        pglGenerateMipmapEXT(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }
    
	// Create the variation map texture
    std::vector<float> variationMap = m_water.GetVariationMap();
    glGenTextures(1, &m_variationMapID);
    g_Renderer.BindTexture(1, m_variationMapID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 2048, 2048, 0, GL_RED, GL_FLOAT, &variationMap[0]);// TODO: fix hard coded resolution
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	m_reflectionTexSizeW = round_up_to_pow2(g_Renderer.GetWidth());
    m_reflectionTexSizeH = round_up_to_pow2(g_Renderer.GetHeight());

	// Create reflection texture with Mipmapping
    glGenTextures(1, &m_reflectionID);
    glBindTexture(GL_TEXTURE_2D, m_reflectionID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, (GLsizei)m_reflectionTexSizeW, (GLsizei)m_reflectionTexSizeH, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
    pglGenerateMipmapEXT(GL_TEXTURE_2D);  //Generate num_mipmaps number of mipmaps here.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

    // Create reflection depth texture with Mipmapping
    glGenTextures(1, &m_reflectionDepthBufferID);
    glBindTexture(GL_TEXTURE_2D, m_reflectionDepthBufferID);
    glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, (GLsizei)m_reflectionTexSizeW, (GLsizei)m_reflectionTexSizeH, 0,  GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, NULL);
    pglGenerateMipmapEXT(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);//GL_LINEAR_MIPMAP_LINEAR
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

	// Create reflection frame buffer
	pglGenFramebuffersEXT(1, &m_reflectionFBOID);
	pglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_reflectionFBOID);
	pglFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, m_reflectionID, 0);
	pglFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, m_reflectionDepthBufferID, 0);

	ogl_WarnIfError();

	GLenum status = pglCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
	{
		LOGWARNING("Reflection framebuffer object incomplete: 0x%04X", status);
	}

	// Create refraction texture with Mipmapping
    glGenTextures(1, & m_refractionID);
	glBindTexture(GL_TEXTURE_2D, m_refractionID);
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, (GLsizei)m_reflectionTexSizeW, (GLsizei)m_reflectionTexSizeH, 0,  GL_RGB, GL_UNSIGNED_BYTE, 0);
    pglGenerateMipmapEXT(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    //glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, (float[]){1.0f, 0.0f, 0.0f, 1.0f});


    // Create refraction camera depth texture with Mipmapping
    glGenTextures(1, &m_refractionDepthBufferID);
    glBindTexture(GL_TEXTURE_2D, m_refractionDepthBufferID);
    glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, (GLsizei)m_reflectionTexSizeW, (GLsizei)m_reflectionTexSizeH, 0,  GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, NULL);
    pglGenerateMipmapEXT(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, (float[]){1.0f, 0.0f, 0.0f, 1.0f});

	// Create refraction frame buffer
	pglGenFramebuffersEXT(1, &m_refractionFBOID);
	pglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_refractionFBOID);
	pglFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, m_refractionID, 0);
	pglFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, m_refractionDepthBufferID, 0);

	ogl_WarnIfError();

	status = pglCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
	{
		LOGWARNING("Refraction framebuffer object incomplete: 0x%04X", status);
	}

    // Create entire scene texture with Mipmapping
    glGenTextures(1, & m_entireSceneID);
    glBindTexture(GL_TEXTURE_2D, m_entireSceneID);
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, (GLsizei)m_reflectionTexSizeW, (GLsizei)m_reflectionTexSizeH, 0,  GL_RGB, GL_UNSIGNED_BYTE, 0);
    pglGenerateMipmapEXT(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    //glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, (float[]){1.0f, 0.0f, 0.0f, 1.0f});


    // Create entire camera depth texture with Mipmapping
    glGenTextures(1, &m_entireSceneDepthBufferID);
    glBindTexture(GL_TEXTURE_2D, m_entireSceneDepthBufferID);
    glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, (GLsizei)m_reflectionTexSizeW, (GLsizei)m_reflectionTexSizeH, 0,  GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, NULL);
    pglGenerateMipmapEXT(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, (float[]){1.0f, 0.0f, 0.0f, 1.0f});

    // Create entire frame buffer
    pglGenFramebuffersEXT(1, &m_entireSceneFBOID);
    pglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_entireSceneFBOID);
    pglFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, m_entireSceneID, 0);
    pglFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, m_entireSceneDepthBufferID, 0);

    ogl_WarnIfError();

    status = pglCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
    {
        LOGWARNING("Entire framebuffer object incomplete: 0x%04X", status);
    }

    // TODO: create flow map
    glGenTextures(1, &m_flowMapID);
}

void CGridProjector::UpdateReflectionFarPlane()
{
    CVector3D reflectionCamPos = m_reflectionCam.GetOrientation().GetTranslation();
    CVector3D reflectionLookAt = m_reflectionCam.GetOrientation().GetIn();
    //m_ProjMat * m_Orientation.GetInverse();
    //LOGWARNING("ratio = %f", m_Height / float(std::max(1, m_Width)));
    //LOGWARNING("[W,H] = [%u, %u]", m_Width, m_Height);
    
    //CVector3D camPosition = m_reflectionCam.GetOrientation().GetTranslation();
    //CVector3D camDirection = m_reflectionCam.GetOrientation().GetIn();
    CVector3D frustrumPoint = reflectionCamPos + reflectionLookAt.Normalized() * m_reflectionCam.GetFarPlane();
    CPlane farClipPlane;
    farClipPlane.Set(-reflectionLookAt.Normalized(), frustrumPoint);
    m_reflectionFarClip = farClipPlane;
}

void CGridProjector::UpdateRefractionFarPlane()
{
    CVector3D refractionCamPos = m_refractionCam.GetOrientation().GetTranslation();
    CVector3D refractionLookAt = m_refractionCam.GetOrientation().GetIn();

    CVector3D frustrumPoint = refractionCamPos + refractionLookAt.Normalized() * m_refractionCam.GetFarPlane();
    CPlane farClipPlane;
    farClipPlane.Set(-refractionLookAt.Normalized(), frustrumPoint);
    m_refractionFarClip = farClipPlane;
}
