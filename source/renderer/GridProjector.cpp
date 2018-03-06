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

#include "renderer/Renderer.h"
#include "renderer/VertexBuffer.h"
#include "renderer/VertexBufferManager.h"

#include "renderer/GridProjector.h"

GridProjector::GridProjector()
{
	m_resolutionX = 256;
	m_resolutionY = 256;

	m_time = 0;
	m_camera = CCamera();
	m_Grid_VBindices = NULL;
	m_Grid_VBvertices = NULL;

	m_M_projector = CMatrix3D();
	m_M_range = CMatrix3D();

    // Temporary here
    SetupGrid();

}

GridProjector::~GridProjector()
{
	if (m_Grid_VBindices)
		g_VBMan.Release(m_Grid_VBindices);

	if (m_Grid_VBvertices)
		g_VBMan.Release(m_Grid_VBvertices);
}

void GridProjector::SetupGrid()
{
	//std::vector<CVector2D> vertices;

    /*
	double xRatio = 1.0 / (m_resolutionX - 1);
	double yRatio = 1.0 / (m_resolutionY - 1);

	for (int i = 0; i < m_resolutionY; i++)
		for (int j = 0; j <  m_resolutionX; j++)
			m_vertices.push_back(CVector2D(i * xRatio - 0.5, i * yRatio - 0.5));
    //*/
    m_vertices.push_back(CVector2D(-0.5,  0.5));
    m_vertices.push_back(CVector2D( 0.0,  0.5));
    m_vertices.push_back(CVector2D( 0.5,  0.5));
    m_vertices.push_back(CVector2D(-0.5,  0.0));
    m_vertices.push_back(CVector2D( 0.0,  0.0));
    m_vertices.push_back(CVector2D( 0.5,  0.0));
    m_vertices.push_back(CVector2D(-0.5, -0.5));
    m_vertices.push_back(CVector2D( 0.0, -0.5));
    m_vertices.push_back(CVector2D( 0.5, -0.5));
    
	m_Grid_VBvertices = g_VBMan.Allocate(sizeof(CVector2D), m_vertices.size(), GL_STATIC_DRAW, GL_ARRAY_BUFFER);
	m_Grid_VBvertices->m_Owner->UpdateChunkVertices(m_Grid_VBvertices, &m_vertices[0]);

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

	m_Grid_VBindices = g_VBMan.Allocate(sizeof(GLuint), m_indices.size(), GL_STATIC_DRAW, GL_ELEMENT_ARRAY_BUFFER);
	m_Grid_VBindices->m_Owner->UpdateChunkVertices(m_Grid_VBindices, &m_indices[0]);

}

void GridProjector::Render(CShaderProgramPtr& shader)
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
	//glClearColor(0.0f,0.0f, 0.0f,0.0f);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glEnable(GL_DEPTH_TEST);
	//glDepthFunc(GL_ALWAYS);

	if (g_Renderer.m_SkipSubmit)
		return;

	CShaderDefines none;
	//CShaderProgramPtr shad = g_Renderer.GetShaderManager().LoadProgram("glsl/projector", none);
    CShaderProgramPtr& shad = shader;

	//shad->Bind();

	shad->Uniform(str_transform, g_Renderer.GetViewCamera().GetViewProjection());

	//// shad->VertexPointer(...);
    //shad->VertexPointer(3, GL_FLOAT, sizeof(CVector2D), &m_vertices[0]);
    
    CVector2D* base = (CVector2D*) m_Grid_VBvertices->m_Owner->Bind();
    
    shad->VertexAttribPointer(str_vertexPosition, 2, GL_FLOAT, false, sizeof(CVector2D), &base[m_Grid_VBvertices->m_Index].X);// ???
    //shad->VertexPointer(2, GL_FLOAT, sizeof(m_vertices[0]), &m_vertices[0]);
    
    // Call before glDraw(...)
	shad->AssertPointersBound();

    // Segfault here
	//u8* indexBase = m_Grid_VBindices->m_Owner->Bind();
    // <----
    
    // Owner is null...
    //printf("----------------------------%s\n", m_Grid_VBindices->m_Owner->Bind());
    //u8* indexBase = 0;
	//glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, &m_indices[0]);
    u8* indexBase = m_Grid_VBindices->m_Owner->Bind();
	glDrawElements(GL_TRIANGLES, (GLsizei) m_Grid_VBindices->m_Count, GL_UNSIGNED_INT, indexBase + sizeof(u16)*(m_Grid_VBindices->m_Index));

	CVertexBuffer::Unbind();
	//shad->Unbind();

	//glDisable(GL_BLEND);
	//glDepthFunc(GL_LEQUAL);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void GridProjector::PrintTestMessage()
{
    printf("----------------------TestCall-----------------------------\n");
}
