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

#include "precompiled.h"

#include "graphics/Terrain.h"
#include "graphics/ShaderManager.h"
#include "graphics/ShaderProgram.h"

#include "lib/ogl.h"
#include "lib/timer.h"

#include "maths/MathUtil.h"
#include "maths/Vector3D.h"
#include "maths/Vector2D.h"

#include "ps/CLogger.h"
#include "ps/Game.h"
#include "ps/World.h"

#include "renderer/Renderer.h"
#include "renderer/VertexBuffer.h"
#include "renderer/VertexBufferManager.h"
// TODO: Temp to be removed later
#include "renderer/WaterManager.h"

#include "renderer/GridProjector.h"

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

GridProjector::GridProjector() : m_model(FFTWaterModel()), m_water(m_model), m_gridVBIndices(0), m_gridVBVertices(0)
{
	m_time = 0.0;
	m_resolutionX = 128;
	m_resolutionY = 256;
	m_totalResolution = m_resolutionX*m_resolutionY;

	m_PCamera = CCamera();

	m_Mpiview = CMatrix3D();
    m_Miperspective = CMatrix3D();
	m_Mrange = CMatrix3D();
	m_Mprojector = CMatrix3D();

    // Temporary here
    SetupGrid();
}

GridProjector::~GridProjector()
{
	if (m_gridVBIndices) g_VBMan.Release(m_gridVBIndices);
	if (m_gridVBVertices) g_VBMan.Release(m_gridVBVertices);

	//m_Grid_indices.FreeBackingStore();
	//m_Grid_vertices.FreeBackingStore();
}

void GridProjector::SetupGrid()
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

}

void GridProjector::GenerateVertices()
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

void GridProjector::GenerateIndices()
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
            //printf("%u, %u, %u\n", j*m_resolutionX + i, (j+1)*m_resolutionX + (i), j*m_resolutionX + (i+1));
			//   *
			//  /|
			// *-*
			m_indices.push_back(j * m_resolutionX + (i + 1));
			m_indices.push_back((j + 1) * m_resolutionX + i);
			m_indices.push_back((j + 1) * m_resolutionX + (i + 1));
            //printf("%u, %u, %u\n", j*m_resolutionX + (i+1), (j+1)*m_resolutionX + i, (j+1)*m_resolutionX + (i+1));
		}

	}
}

void GridProjector::UpdateMatrices()
{
	PROFILE3_GPU("update matrices");
    // TODO: Temporary here, don't know where to put else for know
	//CPlane water = m_water.GetBasePlane();
    m_water.m_base.Set(CVector3D(0.f, 1.f, 0.f), CVector3D(0.f, g_Renderer.GetWaterManager()->m_WaterHeight, 0.f));
    //water.Set(CVector3D(0.f, 1.f, 0.f), CVector3D(0.f, g_Renderer.GetWaterManager()->m_WaterHeight, 0.f));
    
    //m_Mperspective = g_Renderer.GetViewCamera().GetProjection();
    
	CCamera g_RCamera = g_Renderer.GetViewCamera();
    CMatrix3D invV = g_RCamera.GetOrientation();
	CVector3D camPosition = invV.GetTranslation();// (invV._14, invV._24, invV._34);
	CVector3D camDirection = invV.GetIn();// (invV._13, invV._23, invV._33);
	CVector3D camUp = invV.GetUp();// (invV._12, invV._22, invV._32);

	CVector3D projPosition = camPosition;
	CVector3D projDirection = camDirection;
	CVector3D projUp = camUp;

	CVector3D intersection;

	bool gotWater = m_water.m_base.FindRayIntersection(camPosition, camDirection, &intersection);
	//bool gotWater = water.FindRayIntersection(camPosition, camDirection, &intersection);

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
    
	CPlane water = m_water.m_base;
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
	//LOGWARNING("Transformed points:\n");
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
    
	// Something fishy here
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

void GridProjector::ComputeIntersection(std::vector<CVector4D>& cam_frustrum, std::vector<CVector4D>& span_buffer, CPlane& maxWater, CPlane& minWater, int startIndice, int endIndice)
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

void GridProjector::Render(CShaderProgramPtr& shader)
{
	if (g_Renderer.m_SkipSubmit)
		return;

#if DEBUG_LEGEND
	LOGWARNING("[S] Screen space [W] world space");
#endif

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	m_time = (float)timer_Time();
	UpdateMatrices();
	//UpdatePoints();

    u8* base = m_gridVBVertices->m_Owner->Bind();

	shader->VertexAttribPointer(str_vertexPosition, 4, GL_FLOAT, GL_FALSE, sizeof(CVector4D), base);

#if DEBUG_RENDER_WORLD_POS
    CVector3D worldPos = g_Renderer.GetViewCamera().GetWorldCoordinates(512, 384);
    LOGWARNING("[W] pos: (%f, %f, %f)", worldPos.X, worldPos.Y, worldPos.Z);
#endif
    shader->Uniform(str_transform, g_Renderer.GetViewCamera().GetViewProjection());
	shader->Uniform(str_projector, m_Mprojector);
	shader->Uniform(str_waterNormal, m_water.m_base.m_Norm);
	shader->Uniform(str_waterD, m_water.m_base.m_Dist);
    shader->Uniform(str_time, m_time);
	//shader->Uniform(str_waterNormal, m_water.GetBasePlane().m_Norm);
	//shader->Uniform(str_waterD, m_water.GetBasePlane().m_Dist);

    shader->AssertPointersBound();
    
    u8* indexBase = m_gridVBIndices->m_Owner->Bind();
    glDrawElements(GL_TRIANGLES, (GLsizei) m_gridVBIndices->m_Count, GL_UNSIGNED_INT, indexBase);
    
	CVertexBuffer::Unbind();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

}

void GridProjector::UpdatePoints()
{
	PROFILE3_GPU("update points");
	CVector4D start;
	CVector4D end;
    CVector4D transformed;
	bool doIntersect;
	CVector3D intersection;
	//CPlane water = m_water.m_base;
    CMatrix3D g_IPV;
    CMatrix3D g_PV = g_Renderer.GetViewCamera().GetViewProjection();
    g_PV.GetInverse(g_IPV);
    CMatrix3D p_IPV;
    m_Mprojector.GetInverse(p_IPV);

	GenerateVertices();

	for (size_t i = 0; i < m_vertices.size(); i++)
	{
		start = end = m_vertices[i];
		start.Z = 1.0; // Z and Y are exchanged
		end.Z = -1.0;

#if DEBUG_UPDATE_POINTS
		if (i == 0)
		{
			//LOGWARNING("before transformation");
			LOGWARNING("[S] start 0: (%f, %f, %f, %f)", start.X, start.Y, start.Z, start.W);
			LOGWARNING("[S] end 0: (%f, %f, %f, %f)", end.X, end.Y, end.Z, end.W);
		}
#endif

		transformed = m_Mprojector.Transform(start);
		start = transformed / transformed.W;
        transformed = m_Mprojector.Transform(end);
		end = transformed / transformed.W;

#if DEBUG_UPDATE_POINTS
		if (i == 0)
		{
			//LOGWARNING("after transformation");
			LOGWARNING("[W] start 0: (%f, %f, %f, %f)", start.X, start.Y, start.Z, start.W);
			LOGWARNING("[W] end 0: (%f, %f, %f, %f)", end.X, end.Y, end.Z, end.W);
		}
#endif

		doIntersect = m_water.m_base.FindLineSegIntersection(CVector3D(start.X, start.Y, start.Z), CVector3D(end.X, end.Y, end.Z), &intersection);
		//doIntersect = m_water.GetBasePlane().FindLineSegIntersection(CVector3D(start.X, start.Y, start.Z), CVector3D(end.X, end.Y, end.Z), &intersection);
#if DEBUG_UPDATE_POINTS
		if (i == 0)
		{
			LOGWARNING("Does intersect: %s", doIntersect ? "true" : "false");
		}
#endif
		if (doIntersect)
		{
#if DEBUG_UPDATE_POINTS_INTERSECT_INFO
			LOGWARNING("[W] intersetction 0: (%f, %f, %f)", intersection.X, intersection.Y, intersection.Z);
#endif
			transformed = CVector4D(intersection.X, intersection.Y, intersection.Z, 1.0);
			m_model.Update(timer_Time(), transformed);
			m_vertices[i] = transformed;
		}
	}
#if DEBUG_UPDATE_POINTS
	LOGWARNING("[W] point 0: (%f, %f, %f, %f)", m_vertices[0].X, m_vertices[0].Y, m_vertices[0].Z, m_vertices[0].W);
#endif

    m_gridVBVertices->m_Owner->UpdateChunkVertices(m_gridVBVertices, &m_vertices[0]);
}

