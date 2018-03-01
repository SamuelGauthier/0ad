/* Copyright (C) 2017 Wildfire Games.
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

#include "renderer/Renderer.h"
#include "renderer/VertexBuffer.h"
#include "renderer/VertexBufferManager.h"

#include "GridProjector.h"

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

	double xRatio = 1.0 / (m_resolutionX - 1);
	double yRatio = 1.0 / (m_resolutionY - 1);

	for (size_t i = 0; i < m_resolutionY; i++)
		for (size_t j = 0; j <  m_resolutionX; j++)
			m_vertices.push_back(CVector2D(i * xRatio - 0.5, i * yRatio - 0.5));

	m_Grid_VBvertices = g_VBMan.Allocate(sizeof(CVector2D), m_vertices.size(), GL_STATIC_DRAW, GL_ARRAY_BUFFER);
	m_Grid_VBvertices->m_Owner->UpdateChunkVertices(m_Grid_VBvertices, &m_vertices[0]);

	//std::vector<GLuint> indices;

	for (size_t i = 0; i < m_resolutionY; i++)
	{
		for (size_t j = 0; j < m_resolutionX; j++)
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

	m_Grid_VBindices = g_VBMan.Allocate(sizeof(GLuint), m_indices.size(), GL_STATIC_DRAW, GL_ELEMENT_ARRAY_BUFFER);
	m_Grid_VBindices->m_Owner->UpdateChunkVertices(m_Grid_VBindices, &m_indices[0]);

}

void GridProjector::Render(CCamera& camera)
{
	glClearColor(0.0f,0.0f, 0.0f,0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	//glDepthFunc(GL_ALWAYS);

	if (g_Renderer.m_SkipSubmit)
		return;

	CShaderDefines none;
	CShaderProgramPtr shad = g_Renderer.GetShaderManager().LoadProgram("glsl/projector", none);

	shad->Bind();

	shad->Uniform(str_transform, g_Renderer.GetViewCamera().GetViewProjection());

	// shad->VertexPointer(...);
	
	// Call before glDraw(...)
	shad->AssertPointersBound();

	u8* indexBase = m_Grid_VBindices->m_Owner->Bind();
	glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, indexBase + sizeof(u32)*(m_Grid_VBindices->m_Index));

	CVertexBuffer::Unbind();
	shad->Unbind();

	glDisable(GL_BLEND);
	glDepthFunc(GL_LEQUAL);
}