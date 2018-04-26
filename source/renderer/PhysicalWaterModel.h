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

#ifndef INCLUDED_PHYSICALWATERMODEL
#define INCLUDED_PHYSICALWATERMODEL

#include "lib/res/handle.h"
#include "graphics/TextureManager.h"

class PhysicalWaterModel
{
public:
	virtual ~PhysicalWaterModel() {}

	//virtual void Update(double time, CVector4D& point) = 0;
	virtual void GetHeightMapAtTime(double time, std::vector<u8>* heightMap, std::vector<u8>* normalMap) = 0;
	virtual std::vector<u8> GetHeightMapAtLevel(u8 level) = 0;
	virtual std::vector<u8> GetNormalMapAtLevel(u8 level) = 0;
	virtual std::vector<u8> GetVariationMap() = 0;
	virtual void GenerateHeightMaps() = 0;

	float GetMaxHeight() { return m_maxHeight; }
	float GetMinHeight() { return m_minHeight; }
	void SetMaxHeight(float maxHeight) { m_minHeight = maxHeight; }
	void SetMinHeight(float minHeight) { m_minHeight = minHeight; }

private:
	float m_maxHeight;
	float m_minHeight;
};

#endif // !INCLUDED_PHYSICALWATERMODEL

