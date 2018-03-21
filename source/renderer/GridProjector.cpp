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

#include "maths/MathUtil.h"
//#include "maths/Matrix3D.h"
//#include "maths/Vector3D.h"
#include "maths/Vector2D.h"

#include "ps/CLogger.h"
#include "ps/Game.h"
#include "ps/World.h"
//#include "ps/Profile.h"

//#include "renderer/FFTWaterModel.h"
#include "renderer/Renderer.h"
#include "renderer/VertexBuffer.h"
#include "renderer/VertexBufferManager.h"
// TODO: Temp to be removed later
#include "renderer/WaterManager.h"

#include "renderer/GridProjector.h"

//const u16 MAX_ENTITIES_DRAWN = 65535;

GridProjector::GridProjector() : m_model(FFTWaterModel()), m_water(m_model), m_gridVBIndices(0), m_gridVBVertices(0)//, //m_water(FFTWaterModel())//, m_gridVertices(GL_STATIC_DRAW), m_gridIndices(GL_STATIC_DRAW)
{
	m_resolutionX = 128;
	m_resolutionY = 64;
	m_totalResolution = m_resolutionX*m_resolutionY;
    
    //FFTWaterModel model;
    //m_water = Water(model)

	m_time = 0;
	m_PCamera = CCamera();
	//m_gridVBIndices = NULL;
	//m_gridVBVertices = NULL;

	m_Mpiview = CMatrix3D();
    m_Miperspective = CMatrix3D();
	m_Mrange = CMatrix3D();
	m_Mprojector = CMatrix3D();

	//m_position.type = GL_FLOAT;
	//m_position.elems = 3;
	//m_gridVertices.AddAttribute(&m_position);

    // Temporary here
    SetupGrid();
	//SetupMatrices();
    //BuildArrays();

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

	/*
	//float xRatio = 2.0 / (m_resolutionX - 1);
	//float yRatio = 2.0 / (m_resolutionY - 1);
    float xRatio = 200.0 / (m_resolutionX - 1);
    float yRatio = 200.0 / (m_resolutionY - 1);
    //printf("rH: %u, rW: %u\n", m_resolutionY, m_resolutionX);
    //printf("hratio: %f, wratio: %f\n", yRatio, xRatio);
    //printf("dH: %f, dW: %f\n", 2/2.0, 2/2.0);
    
	for (uint j = 0; j < m_resolutionY; j++)
    {
        for (uint i = 0; i <  m_resolutionX; i++)
        {
            //m_vertices.push_back(CVector3D(i * xRatio - 1.0, 1.0 - j * yRatio, 0.0));
            m_vertices.push_back(CVector3D(i * xRatio - 1.0, 10.0, 1.0 - j * yRatio));
            //printf("x:%f y:%f\n", i * xRatio - 1.0, 1.0 - j * yRatio);
        }
    }
	*/
	GenerateVertices();
    
	m_gridVBVertices = g_VBMan.Allocate(sizeof(CVector3D), m_vertices.size(), GL_STATIC_DRAW, GL_ARRAY_BUFFER);

	if (m_gridVBVertices == NULL || m_gridVBVertices == 0)
		LOGERROR("No storage available for water vertices!");
    
    // fails but but not down later, why???
	//m_gridVBVertices->m_Owner->UpdateChunkVertices(m_gridVBVertices, &m_vertices[0]);
    /*
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
	*/
	GenerateIndices();
    
    //printf("indices size: %lu\n", m_indices.size());
	m_gridVBIndices = g_VBMan.Allocate(sizeof(GLuint), m_indices.size(), GL_STATIC_DRAW, GL_ELEMENT_ARRAY_BUFFER);
    
    if (m_gridVBIndices == NULL || m_gridVBIndices == 0)
        LOGERROR("No storage available for water indices!");
    
    m_gridVBVertices->m_Owner->UpdateChunkVertices(m_gridVBVertices, &m_vertices[0]);
    
    m_gridVBIndices->m_Owner->UpdateChunkVertices(m_gridVBIndices, &m_indices[0]);

}

void GridProjector::GenerateVertices()
{
	m_vertices.clear();

    float xRatio = 200.0 / (m_resolutionX - 1);
    float yRatio = 200.0 / (m_resolutionY - 1);
    
	for (uint j = 0; j < m_resolutionY; j++)
        for (uint i = 0; i <  m_resolutionX; i++)
            m_vertices.push_back(CVector3D(i * xRatio - 1.0, 10.0, 1.0 - j * yRatio));
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
    m_water.m_base.Set(CVector3D(0.f, 1.f, 0.f), CVector3D(0.f, g_Renderer.GetWaterManager()->m_WaterHeight, 0.f));
    
    //m_Mperspective = g_Renderer.GetViewCamera().GetProjection();
    
	CCamera camera = g_Renderer.GetViewCamera();
    CMatrix3D orientation = camera.GetOrientation();
	CVector3D camPosition = orientation.GetTranslation();// (orientation._14, orientation._24, orientation._34);
	CVector3D camDirection = orientation.GetIn();// (orientation._13, orientation._23, orientation._33);
	CVector3D camUp = orientation.GetUp();// (orientation._12, orientation._22, orientation._32);

	CVector3D projPosition = camPosition;
	CVector3D projDirection = camDirection;
	CVector3D projUp = camUp;

	CVector3D intersection;

	bool gotWater = m_water.m_base.FindRayIntersection(camPosition, camDirection, &intersection);

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

	m_PCamera.LookAt(projPosition, projDirection, projUp);
	m_PCamera.SetProjection(camera.GetNearPlane(), camera.GetFarPlane(), camera.GetFOV());
	m_PCamera.UpdateFrustum();

	m_Mpiview = m_PCamera.GetOrientation();
	g_Renderer.GetViewCamera().GetProjection().GetInverse(m_Miperspective);

	//LOGWARNING("Near plane at: %f", g_Renderer.GetViewCamera().GetNearPlane());
	//LOGWARNING("Far plane at: %f", g_Renderer.GetViewCamera().GetFarPlane());
	//LOGWARNING("Projector Near plane at: %f", m_camera.GetNearPlane());
	//LOGWARNING("Projector Far plane at: %f", m_camera.GetFarPlane());

    
	// TODO: Compute the m_Mrange matrix

    // Project frustrum corner points into world coordinates
    
    CPlane water = m_water.m_base;
    std::vector<CVector4D> span_buffer;
	std::vector<CVector4D> cam_frustrum;
	cam_frustrum.push_back(CVector4D(1, 1, 1, 1));
	cam_frustrum.push_back(CVector4D(1, 1, -1, 1));
	cam_frustrum.push_back(CVector4D(1, -1, 1, 1));
	cam_frustrum.push_back(CVector4D(1, -1, -1, 1));
	cam_frustrum.push_back(CVector4D(-1, 1, 1, 1));
	cam_frustrum.push_back(CVector4D(-1, 1, -1, 1));
	cam_frustrum.push_back(CVector4D(-1, -1, 1, 1));
	cam_frustrum.push_back(CVector4D(-1, -1, -1, 1));


	CMatrix3D invP; // = g_Renderer.GetViewCamera().GetProjection();
	g_Renderer.GetViewCamera().GetProjection().GetInverse(invP);

	CMatrix3D invPV = orientation * invP;
    
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
    
    // Check the intersections of the camera frustrum planes and the max/min water planes
    CPlane maxWater;
    CPlane minWater;
    // d = -(ax0 +by0+cz0)
    maxWater.Set(water.m_Norm, CVector3D(0, -water.m_Dist + maxWaterHeight, 0));
    minWater.Set(water.m_Norm, CVector3D(0, -water.m_Dist + minWaterHeight, 0));

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

    // Project the intersections onto the water plane
    CVector3D start;
    CVector3D direction = -water.m_Norm;
    CVector3D projection;
    CMatrix3D viewProjection = m_PCamera.GetViewProjection();
    //CVector2D maxP = CVector2D(-FLT_MAX, -FLT_MAX);
    //CVector2D minP = CVector2D(FLT_MAX, FLT_MAX);
    CVector2D current;
	float x_max = span_buffer[0].X;
	float x_min = span_buffer[0].X;
	float y_max = span_buffer[0].Y;
	float y_min = span_buffer[0].Y;
    
    for (size_t i = 0; i < span_buffer.size(); i++)
	{
        start = CVector3D(span_buffer[i].X, span_buffer[i].Y, span_buffer[i].Z);
        water.FindRayIntersection(start, direction, &projection);
        //LOGWARNING("projected point: (%f, %f, %f)", projection.X, projection.Y, projection.Z);
        
        // Transform the points into projector space
        span_buffer[i] = viewProjection.Transform(CVector4D(projection.X, projection.Y, projection.Z, 1));
        current = CVector2D(span_buffer[i].X, span_buffer[i].Y);
        LOGWARNING("span buffer: (%f, %f)", span_buffer[i].X, span_buffer[i].Y);
		if (current.X > x_max) x_max = current.X;
		if (current.X < x_min) x_min = current.X;
		if (current.Y > y_max) y_max = current.Y;
		if (current.Y < y_min) y_min = current.Y;
        //if(current.X >= maxP.X && current.Y >= maxP.Y) maxP = current;
        //if(current.X <= maxP.X && current.Y <= maxP.Y) minP = current;
    }
	LOGWARNING("x_max: %f, x_min: %f, y_max: %f, y_min: %f", x_max, x_min, y_max, y_min);

	//LOGWARNING("float max: (%f, %f)", FLT_MAX, FLT_MAX);
	//LOGWARNING("float min: (%f, %f)", -FLT_MAX, -FLT_MAX);
	////
	//LOGWARNING("min: (%f, %f)", minP.X, minP.Y);
	//LOGWARNING("max: (%f, %f)", maxP.X, maxP.Y);
    
    // Create Mrange matrix
	//m_Mrange.SetIdentity();
    m_Mrange = CMatrix3D(x_max - x_min, 0, 0, x_min,
                         0, y_max - y_min, 0, y_min,
                         0, 0, 1, 0,
                         0, 0, 0, 1);

	m_Mprojector = m_Mrange * m_Miperspective * m_Mpiview;
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
        LOGWARNING("maxInter: (%f, %f,%f)", intMax.X, intMax.Y, intMax.Z);
    }
    if (minIntersect)
    {
        span_buffer.push_back(CVector4D(intMin.X, intMin.Y, intMin.Z, 1));
        LOGWARNING("minInter: (%f, %f,%f)", intMin.X, intMin.Y, intMin.Z);
    }
}

void GridProjector::Render(CShaderProgramPtr& shader)
{
	if (g_Renderer.m_SkipSubmit)
		return;

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	UpdateMatrices();
	UpdatePoints();

    u8* base = m_gridVBVertices->m_Owner->Bind();
    

     shader->VertexAttribPointer(str_vertexPosition, 3, GL_FLOAT, GL_FALSE, sizeof(CVector3D), base);
    
    CVector3D worldPos = g_Renderer.GetViewCamera().GetWorldCoordinates(512, 384);
    LOGWARNING("wPos: (%f, %f, %f)", worldPos.X, worldPos.Y, worldPos.Z);
    //shader->Uniform(str_transform, g_Renderer.GetViewCamera().GetViewProjection());
	//
    shader->Uniform(str_transform, m_PCamera.GetViewProjection());

    shader->AssertPointersBound();
    
    u8* indexBase = m_gridVBIndices->m_Owner->Bind();
    glDrawElements(GL_TRIANGLES, (GLsizei) m_gridVBIndices->m_Count, GL_UNSIGNED_INT, indexBase);
    
	CVertexBuffer::Unbind();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

}

void GridProjector::UpdatePoints()
{
	PROFILE3_GPU("update points");
	CVector3D start;
	CVector3D end;
	bool doIntersect;
	CVector3D intersection;
	//CPlane water = m_water.m_base;

	GenerateVertices();

	for (size_t i = 0; i < m_vertices.size(); i++)
	{
		start = end = m_vertices[i];
		start.Z = 1.0;
		end.Z = -1.0;
		m_Mprojector.Transform(start);
		m_Mprojector.Transform(end);

		doIntersect = m_water.m_base.FindLineSegIntersection(start, end, &intersection);
		if (doIntersect)
			m_vertices[i] = intersection;
		//m_gridVertices[i]*
	}

	//CMatrix3D
	/*
	for (size_t i = 0; i < m_vertices.size(); i++)
	{
		g_Renderer.GetViewCamera().GetViewProjection().GetInverse();
	}
	//*/

    m_gridVBVertices->m_Owner->UpdateChunkVertices(m_gridVBVertices, &m_vertices[0]);
}
