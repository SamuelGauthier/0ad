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

#include "OceanWater.h"

#include "maths/Noise.h"
#include "ps/CLogger.h"

// TODO: Remove dependency
#include "renderer/Renderer.h"
#include "renderer/WaterManager.h"


COceanWater::COceanWater(CFFTWaterModel waterModel) : m_waterModel(waterModel)
{
}

COceanWater::~COceanWater() {}

float COceanWater::GetMaxWaterHeight()
{
    // TODO: Temporary, should compute the max from the
    return 5.0f;
}

float COceanWater::GetMinWaterHeight()
{
    // Id. as in COceanWater::GetMaxWaterHeight()
    return -5.0f;
}

float COceanWater::GetWaterHeight()
{
    return -m_waterBase.m_Dist;
}

void COceanWater::UpdateWaterHeight()
{
    // TODO: Remove dependency to WaterManager
    float waterHeight = g_Renderer.GetWaterManager()->m_WaterHeight;
    m_waterBase.Set(CVector3D(0.f, 1.f, 0.f), CVector3D(0.f, waterHeight, 0.f));
}

void COceanWater::GenerateWaterWaves()
{
    m_waterModel.GenerateHeightAndNormalMaps(&m_heightMaps, &m_normalMaps);
}

void COceanWater::GenerateVariationMap()
{
    int size = 2048;
    Noise2D noise(size);

    m_variationMap = std::vector<GLfloat>(size*size);
    
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            //m_variationMap[i*size + j] = (GLfloat) pnoise(i*0.1, j*0.1, size, size);
            m_variationMap[i*size + j] = (GLfloat) noise(i*0.01, j*0.01);
        }
    }
}

void COceanWater::GenerateFlowMap()
{
    // TODO: implement
}