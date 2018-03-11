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

#include "graphics/ShaderManager.h"
#include "graphics/ShaderProgram.h"

#include "lib/ogl.h"

#include "ps/CLogger.h"
//#include "ps/Profile.h"

#include "renderer/Renderer.h"
#include "renderer/VertexBuffer.h"
#include "renderer/VertexBufferManager.h"

#include "renderer/GridProjector.h"

//const u16 MAX_ENTITIES_DRAWN = 65535;

GridProjector::GridProjector() : m_gridIndices(0), m_gridVertices(0)
{
	m_resolutionX = 3;
	m_resolutionY = 3;
	m_totalResolution = m_resolutionX*m_resolutionY;

	m_time = 0;
	m_camera = CCamera();
	m_gridVBIndices = NULL;
	m_gridVBVertices = NULL;

	m_M_projector = CMatrix3D();
	m_M_range = CMatrix3D();

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

	double xRatio = 2.0 / (m_resolutionX - 1);
	double yRatio = 2.0 / (m_resolutionY - 1);
    printf("rH: %hu, rW: %hu\n", m_resolutionY, m_resolutionX);
    printf("hratio: %f, wratio: %f\n", yRatio, xRatio);
    printf("dH: %f, dW: %f\n", 2/2.0, 2/2.0);

    
    
	for (int j = 0; j < m_resolutionY; j++)
    {
        for (int i = 0; i <  m_resolutionX; i++)
        {
            m_vertices.push_back(CVector3D(i * xRatio - 1, 1 - j * yRatio, 0.0));
            printf("x:%f y:%f\n", i * xRatio - 1, 1 - j * yRatio);
        }
    }
    
	m_gridVBVertices = g_VBMan.Allocate(sizeof(CVector3D), m_vertices.size(), GL_STATIC_DRAW, GL_ARRAY_BUFFER);

	if (m_gridVBVertices == NULL || m_gridVBVertices == 0)
		LOGERROR("No storage available for water vertices!");

	m_gridVBVertices->m_Owner->UpdateChunkVertices(m_gridVBVertices, &m_vertices[0]);

	//std::vector<GLuint> indices;

	for (int j = 0; j < (m_resolutionY - 1); j++)
	{
		for (int i = 0; i < (m_resolutionX - 1); i++)
		{
			// *-*
			// |/
			// *
			m_indices.push_back(j * m_resolutionX + i);
			m_indices.push_back((j + 1) * m_resolutionX + i);
			m_indices.push_back(j * m_resolutionX + (i + 1));
            printf("%d, %d, %d\n", j*m_resolutionX + i, (j+1)*m_resolutionX + (i), j*m_resolutionX + (i+1));
			//   *
			//  /|
			// *-*
			m_indices.push_back(j * m_resolutionX + (i + 1));
			m_indices.push_back((j + 1) * m_resolutionX + i);
			m_indices.push_back((j + 1) * m_resolutionX + (i + 1));
            printf("%d, %d, %d\n", j*m_resolutionX + (i+1), (j+1)*m_resolutionX + i, (j+1)*m_resolutionX + (i+1));
		}

	}
    
	m_gridVBIndices = g_VBMan.Allocate(sizeof(u32), m_indices.size(), GL_STATIC_DRAW, GL_ELEMENT_ARRAY_BUFFER);
    
    if (m_gridVBIndices == NULL || m_gridVBIndices == 0)
        LOGERROR("No storage available for water indices!");
    
    m_gridVBIndices->m_Owner->UpdateChunkVertices(m_gridVBIndices, &m_indices[0]);

}

void GridProjector::Render(CShaderProgramPtr& shader)
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
	//glClearColor(0.0f,0.0f, 0.0f,0.0f);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);

    ///*
	if (g_Renderer.m_SkipSubmit)
		return;

    CVector3D* base = (CVector3D*) m_gridVBVertices->m_Owner->Bind();
    //printf("x:%f y:%f\n", m_vertices[0].X, m_vertices[0].Y);
    
    
    //shader->VertexAttribPointer(str_vertexPosition, 3, GL_FLOAT, GL_FALSE, sizeof(CVector3D), &base[m_gridVBVertices->m_Index]);
    shader->VertexAttribPointer(str_vertexPosition, 3, GL_FLOAT, GL_FALSE, sizeof(CVector3D), &m_vertices[0]);
    
    shader->AssertPointersBound();
    
    u8* indexBase = m_gridVBIndices->m_Owner->Bind();
    glDrawElements(GL_TRIANGLES, m_gridVBIndices->m_Count, GL_UNSIGNED_SHORT, indexBase + sizeof(u16)*(m_gridVBIndices->m_Index));
    

	//CVertexBuffer::Unbind();
	//shad->Unbind();

	glDisable(GL_BLEND);
	glDepthFunc(GL_LEQUAL);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void GridProjector::BuildArrays()
{
	//PROFILE("water build");

	m_gridVertices.SetNumVertices(m_totalResolution);
	m_gridVertices.Layout();

	VertexArrayIterator<CVector3D> position = m_position.GetIterator<CVector3D>();

	double xRatio = 2.0 / (m_resolutionX - 1);
	double yRatio = 2.0 / (m_resolutionY - 1);

	for (u16 j = 0; j < m_resolutionX; j++)
	{
		for (u16 i = 0; i < m_resolutionY; i++)
		{
			*position = CVector3D(i * xRatio - 1, 1 - j * yRatio, 0.1);
			++position;
		}
	}

	m_gridVertices.Upload();
	//m_gridVertices.FreeBackingStore();

	u16 totalIndices = (m_resolutionX - 1) * (m_resolutionY - 1) * 6;
	m_gridIndices.SetNumVertices(totalIndices);
	LOGWARNING("%u", totalIndices);
	m_gridIndices.Layout();

	VertexArrayIterator<u16> index = m_gridIndices.GetIterator();

	for (u16 j = 0; j < m_resolutionX - 1; j++)
	{
		for (u16 i = 0; i < m_resolutionY - 1; i++)
		{
			// *-*
			// |/
			// *
			*index = (j + 0) * m_resolutionX + (i + 0); index++;
			*index = (j + 1) * m_resolutionX + (i + 0); index++;
			*index = (j + 0) * m_resolutionX + (i + 1); index++;
			LOGWARNING("indices A: %u, %u, %u", (j + 0) * m_resolutionX + (i + 0), (j + 1) * m_resolutionX + (i + 0), (j + 0) * m_resolutionX + (i + 1));

			//   *
			//  /|
			// *-*
			*index = (j + 0) * m_resolutionX + (i + 1); index++;
			*index = (j + 1) * m_resolutionX + (i + 0); index++;
			*index = (j + 1) * m_resolutionX + (i + 1); index++;
			LOGWARNING("indices B: %u, %u, %u",  (j + 0) * m_resolutionX + (i + 1), (j + 1) * m_resolutionX + (i + 0), (j + 1) * m_resolutionX + (i + 1));
		}
	}

	m_gridIndices.Upload();
	//m_gridIndices.FreeBackingStore();
	/*
	*/
}
