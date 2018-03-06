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
#include "maths/Vector2D.h"

#include "renderer/VertexBuffer.h"
#include "renderer/ProjectionSystem.h"

class GridProjector : public ProjectionSystem
{
public:
	int m_resolutionX;
	int m_resolutionY;

public:
	GridProjector();
	~GridProjector();

	void Render(CShaderProgramPtr& shader) override;
	void SetupGrid();
    void PrintTestMessage();

private:
	double m_time;
	CCamera m_camera;
	CVertexBuffer::VBChunk* m_Grid_VBindices;
	CVertexBuffer::VBChunk* m_Grid_VBvertices;

	CMatrix3D m_M_projector;
	CMatrix3D m_M_range;

	std::vector<u32> m_indices;
	std::vector<CVector2D> m_vertices;

};

#endif // !INCLUDED_GRIDPROJECTOR
