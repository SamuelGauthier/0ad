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

#include "precompiled.h"

#include "renderer/OceanWater.h"



OceanWater::OceanWater(FFTWaterModel waterModel) : m_waterModel(waterModel)
{
}

OceanWater::~OceanWater() {}

void OceanWater::SetWaterHeight(float height)
{
    m_waterBase.Set(CVector3D(0.f, 1.f, 0.f), CVector3D(0.f, height, 0.f));
}

float OceanWater::GetMaxWaterHeight()
{
    // TODO: Temporary, should compute the max from the
    return 5.0;
}

float OceanWater::GetMinWaterHeight()
{
    // Id. as in OceanWater::GetMaxWaterHeight()
    return -5.0;
}

void OceanWater::GenerateWaterWaves()
{
    // TODO: Generate the water Data
    std::tie(m_heightMaps, m_normalMaps) = m_waterModel.GenerateHeightAndNormalMaps();
}
