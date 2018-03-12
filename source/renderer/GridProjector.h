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
#include "maths/Vector3D.h"
#include "maths/Vector2D.h"

#include "renderer/VertexArray.h"
#include "renderer/VertexBuffer.h"
#include "renderer/ProjectionSystem.h"

class GridProjector : public ProjectionSystem
{
public:
	uint m_resolutionX;
	uint m_resolutionY;
	u32 m_totalResolution;

public:
	GridProjector();
	~GridProjector();

	void Render(CShaderProgramPtr& shader) override;
	void SetupGrid();
	void BuildArrays();

private:
	double m_time;
	CCamera m_camera;
	CVertexBuffer::VBChunk* m_gridVBIndices;
	CVertexBuffer::VBChunk* m_gridVBVertices;

	VertexIndexArray m_gridIndices;

	VertexArray m_gridVertices;
	VertexArray::Attribute m_position;

	CMatrix3D m_M_projector;
	CMatrix3D m_M_range;

	std::vector<GLuint> m_indices;
	std::vector<CVector3D> m_vertices;

};

#endif // !INCLUDED_GRIDPROJECTOR
