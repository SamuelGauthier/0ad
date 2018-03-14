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

#include "ps/CLogger.h"
#include "ps/Game.h"
#include "ps/World.h"
//#include "ps/Profile.h"

#include "renderer/FFTWaterModel.h"
#include "renderer/Renderer.h"
#include "renderer/VertexBuffer.h"
#include "renderer/VertexBufferManager.h"
//Temp to be removed later
#include "renderer/WaterManager.h"

#include "renderer/GridProjector.h"

//const u16 MAX_ENTITIES_DRAWN = 65535;

GridProjector::GridProjector() : m_gridVBIndices(0), m_gridVBVertices(0), m_gridVertices(GL_STATIC_DRAW), m_gridIndices(GL_STATIC_DRAW), m_water(FFTWaterModel())
{
	m_resolutionX = 128;
	m_resolutionY = 64;
	m_totalResolution = m_resolutionX*m_resolutionY;

	m_time = 0;
	m_camera = CCamera();
	//m_gridVBIndices = NULL;
	//m_gridVBVertices = NULL;

	m_Mpview = CMatrix3D();
    m_Mperspective = CMatrix3D();
	m_Mrange = CMatrix3D();
	m_Mprojector = CMatrix3D();

	m_position.type = GL_FLOAT;
	m_position.elems = 3;
	m_gridVertices.AddAttribute(&m_position);

    // Temporary here
    SetupGrid();
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

	float xRatio = 2.0 / (m_resolutionX - 1);
	float yRatio = 2.0 / (m_resolutionY - 1);
    //printf("rH: %u, rW: %u\n", m_resolutionY, m_resolutionX);
    //printf("hratio: %f, wratio: %f\n", yRatio, xRatio);
    //printf("dH: %f, dW: %f\n", 2/2.0, 2/2.0);
    
	for (uint j = 0; j < m_resolutionY; j++)
    {
        for (uint i = 0; i <  m_resolutionX; i++)
        {
            m_vertices.push_back(CVector3D(i * xRatio - 1.0, 1.0 - j * yRatio, 0.0));
            //printf("x:%f y:%f\n", i * xRatio - 1.0, 1.0 - j * yRatio);
        }
    }
    
	m_gridVBVertices = g_VBMan.Allocate(sizeof(CVector3D), m_vertices.size(), GL_STATIC_DRAW, GL_ARRAY_BUFFER);

	if (m_gridVBVertices == NULL || m_gridVBVertices == 0)
		LOGERROR("No storage available for water vertices!");
    
    // fails but but not down later, why???
	//m_gridVBVertices->m_Owner->UpdateChunkVertices(m_gridVBVertices, &m_vertices[0]);
    
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
    
    //printf("indices size: %lu\n", m_indices.size());
	m_gridVBIndices = g_VBMan.Allocate(sizeof(GLuint), m_indices.size(), GL_STATIC_DRAW, GL_ELEMENT_ARRAY_BUFFER);
    
    if (m_gridVBIndices == NULL || m_gridVBIndices == 0)
        LOGERROR("No storage available for water indices!");
    
    m_gridVBVertices->m_Owner->UpdateChunkVertices(m_gridVBVertices, &m_vertices[0]);
    
    m_gridVBIndices->m_Owner->UpdateChunkVertices(m_gridVBIndices, &m_indices[0]);

}

void GridProjector::SetupMatrices()
{
    //m_Mperspective = g_Renderer.GetViewCamera().GetProjection();
    
    CMatrix3D orientation = g_Renderer.GetViewCamera().GetOrientation();
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
	if (camPosition.Y <= intersection.Y) projPosition.Y = intersection.Y + m_water.GetMaxWaterHeight();

	projDirection = intersection - projPosition;

	m_camera.LookAt(projPosition, projDirection, projUp);

	m_Mpview = m_camera.GetOrientation();
	g_Renderer.GetViewCamera().GetProjection().GetInverse(m_Mperspective);

	// TODO: Compute the m_Mrange matrix

	m_Mprojector = m_Mrange * m_Mperspective * m_Mpview;
}

void GridProjector::Render(CShaderProgramPtr& shader)
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    

	if (g_Renderer.m_SkipSubmit)
		return;

    u8* base = m_gridVBVertices->m_Owner->Bind();
    
    shader->VertexAttribPointer(str_vertexPosition, 3, GL_FLOAT, GL_FALSE, sizeof(CVector3D), base);
    
    shader->AssertPointersBound();
    
    u8* indexBase = m_gridVBIndices->m_Owner->Bind();
    glDrawElements(GL_TRIANGLES, (GLsizei) m_gridVBIndices->m_Count, GL_UNSIGNED_INT, indexBase);

	CVertexBuffer::Unbind();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}
