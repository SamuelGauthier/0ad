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

#ifndef INCLUDED_GRIDPROJECTOR
#define INCLUDED_GRIDPROJECTOR

#include "graphics/Camera.h"
#include "VertexBuffer.h"

#include "ProjectionSystem.h"

class GridProjector : ProjectionSystem
{
public:
	GridProjector();
	~GridProjector();

	void Render(CCamera camera) override;

private:
	double m_time;
	CCamera m_camera;
	CVertexBuffer m_vertices;

};

#endif // !INCLUDED_GRIDPROJECTOR
