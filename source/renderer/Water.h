/* Copyright (C) 2018 Wildfire Games.
 * This file is part of 0 A.D.
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

#ifndef INCLUDED_WATER
#define INCLUDED_WATER

//#include "lib/ogl.h"
//#include "graphics/TextureManager.h"
#include "maths/Plane.h"
#include "renderer/PhysicalWaterModel.h"
//#include "ps/Shapes.h"

class Water
{
public:

	virtual ~Water() = 0;

    virtual float GetMaxWaterHeight() = 0;
    virtual float GetMinWaterHeight() = 0;

    virtual PhysicalWaterModel& GetPhysicalWaterModel() = 0;
    
    virtual CPlane GetWaterBasePlane() = 0;
};

#endif // INCLUDED_WATER
