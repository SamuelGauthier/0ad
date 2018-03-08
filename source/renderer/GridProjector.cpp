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

GridProjector::GridProjector() : m_gridIndices(GL_STATIC_DRAW), m_gridVertices(GL_DYNAMIC_DRAW)
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
    //SetupGrid();
	BuildArrays();

}

GridProjector::~GridProjector()
{
	if (m_gridVBIndices)
		g_VBMan.Release(m_gridVBIndices);

	if (m_gridVBVertices)
		g_VBMan.Release(m_gridVBVertices);

	//m_Grid_indices.FreeBackingStore();
	//m_Grid_vertices.FreeBackingStore();
}

void GridProjector::SetupGrid()
{
	//m_gridVBVertices.SetNumVertices(MAX_ENTITIES_DRAWN);
	//m_gridVBIndices.SetNumVertices(MAX_ENTITIES_DRAWN);
	//m_gridVBVertices.Layout();
	//m_gridVBIndices.Layout();

	//std::vector<CVector2D> vertices;

    /*
	double xRatio = 1.0 / (m_resolutionX - 1);
	double yRatio = 1.0 / (m_resolutionY - 1);

	for (int i = 0; i < m_resolutionY; i++)
		for (int j = 0; j <  m_resolutionX; j++)
			m_vertices.push_back(CVector2D(i * xRatio - 0.5, i * yRatio - 0.5));
    //*/
    m_vertices.push_back(CVector3D(-0.5,  0.5, 0.0));
    m_vertices.push_back(CVector3D( 0.0,  0.5, 0.0));
    m_vertices.push_back(CVector3D( 0.5,  0.5, 0.0));
    m_vertices.push_back(CVector3D(-0.5,  0.0, 0.0));
    m_vertices.push_back(CVector3D( 0.0,  0.0, 0.0));
    m_vertices.push_back(CVector3D( 0.5,  0.0, 0.0));
    m_vertices.push_back(CVector3D(-0.5, -0.5, 0.0));
    m_vertices.push_back(CVector3D( 0.0, -0.5, 0.0));
    m_vertices.push_back(CVector3D( 0.5, -0.5, 0.0));
    
	m_gridVBVertices = g_VBMan.Allocate(sizeof(CVector3D), m_vertices.size(), GL_STATIC_DRAW, GL_ARRAY_BUFFER);

	if (m_gridVBVertices == NULL)
		LOGERROR("No storage available for water vertices!");

	m_gridVBVertices->m_Owner->UpdateChunkVertices(m_gridVBVertices, &m_vertices[0]);

	//std::vector<GLuint> indices;

    /*
	for (int i = 0; i < m_resolutionY; i++)
	{
		for (int j = 0; j < m_resolutionX; j++)
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
    //*/
    
    m_indices.push_back(1);m_indices.push_back(0);m_indices.push_back(3);
    m_indices.push_back(1);m_indices.push_back(3);m_indices.push_back(4);
    m_indices.push_back(2);m_indices.push_back(1);m_indices.push_back(4);
    m_indices.push_back(2);m_indices.push_back(4);m_indices.push_back(5);
    m_indices.push_back(4);m_indices.push_back(3);m_indices.push_back(6);
    m_indices.push_back(4);m_indices.push_back(6);m_indices.push_back(7);
    m_indices.push_back(5);m_indices.push_back(4);m_indices.push_back(7);
    m_indices.push_back(5);m_indices.push_back(7);m_indices.push_back(8);

	m_gridVBIndices = g_VBMan.Allocate(sizeof(GLuint), m_indices.size(), GL_STATIC_DRAW, GL_ELEMENT_ARRAY_BUFFER);
	m_gridVBIndices->m_Owner->UpdateChunkVertices(m_gridVBIndices, &m_indices[0]);

}

void GridProjector::Render(CShaderProgramPtr& shader)
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
	glClearColor(0.0f,0.0f, 0.0f,0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);

	if (g_Renderer.m_SkipSubmit)
		return;

	//CShaderDefines none;
	//CShaderProgramPtr shad = g_Renderer.GetShaderManager().LoadProgram("glsl/projector", none);
    CShaderProgramPtr& shad = shader;

	//shad->Bind();

	//shad->Uniform(str_transform, g_Renderer.GetViewCamera().GetViewProjection());

	//// shad->VertexPointer(...);
    //shad->VertexPointer(3, GL_FLOAT, sizeof(CVector2D), &m_vertices[0]);
    
	// Null pointer??? it is (u8) 0
    //u8* vertexBase = m_gridVBVertices->m_Owner->Bind();
    //u8* indexBase = m_gridVBIndices->m_Owner->Bind();
	//LOGWARNING("%p\n", (void*)vertexBase);
	//LOGWARNING("%p\n", (void*)indexBase);
    
    //shad->VertexAttribPointer(str_vertexPosition, 3, GL_FLOAT, false, sizeof(CVector3D), &m_vertices[0]);// ???
    //shad->VertexAttribPointer(str_vertexPosition, 3, GL_FLOAT, false, sizeof(CVector3D), &vertexBase[m_Grid_VBvertices->m_Index]);// ???
	//u8* add = m_Grid_VBvertices->m_Owner->GetBindAddress();
	//LOGERROR("-------------------->x:%d\n", &base[m_Grid_VBvertices->m_Index].X);
	//LOGERROR("-------------------->x:%p\n", (void*)add);
    //shad->VertexPointer(2, GL_FLOAT, sizeof(m_vertices[0]), &m_vertices[0]);
    // Call before glDraw(...)
	//shad->AssertPointersBound();

    // Segfault here
	//u8* indexBase = m_Grid_VBindices->m_Owner->Bind();
    // <----
    
    // Owner is null...
    //printf("----------------------------%s\n", m_Grid_VBindices->m_Owner->Bind());
    //u8* indexBase = 0;
	//glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, &m_indices[0]);

	//glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, &m_vertices[0]);
	//glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, NULL);
    //u8* indexBase = m_Grid_VBindices->m_Owner->Bind();
	//glDrawElements(GL_TRIANGLES, (GLsizei) m_Grid_VBindices->m_Count, GL_UNSIGNED_INT, NULL);

	//CVertexBuffer::Unbind();
	//shad->Unbind();

	//u8* base = m_gridVertices.Bind();
	//GLsizei stride = (GLsizei)m_gridVertices.GetStride();

	//u8* indexBase = m_gridIndices.Bind();

	//shad->VertexPointer(3, GL_FLOAT, stride, base + m_position.offset);

	//shad->AssertPointersBound();
	//glDrawElements(GL_TRIANGLES, (GLsizei)m_gridIndices.GetNumVertices(), GL_UNSIGNED_SHORT, indexBase);

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

	double xRatio = 1.0 / (m_resolutionX - 1);
	double yRatio = 1.0 / (m_resolutionY - 1);

	for (u16 i = 0; i < m_resolutionY; i++)
	{
		for (u16 j = 0; j < m_resolutionX; j++)
		{
			*position = CVector3D(i * xRatio - 0.5, i * yRatio - 0.5, 0.1);
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

	for (u16 i = 0; i < m_resolutionY - 1; i++)
	{
		for (u16 j = 0; j < m_resolutionX - 1; j++)
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
	m_gridIndices.FreeBackingStore();
	/*
	*/
}