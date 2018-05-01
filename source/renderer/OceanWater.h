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

#ifndef INCLUDED_OCEANWATER
#define INCLUDED_OCEANWATER

#include "maths/Plane.h"

#include "renderer/FFTWaterModel.h"

class OceanWater
{
public:
    
    OceanWater(FFTWaterModel waterModel);
    ~OceanWater();
    
    void GenerateWaterWaves();
    
    float GetMaxWaterHeight();
    float GetMinWaterHeight();

    std::vector<std::vector<u8>> GetHeightMaps () { return m_heightMaps; }
    std::vector<std::vector<u8>> GetNormalMaps () { return m_normalMaps; }
    std::vector<u8> GetVariationMap () { return m_variationMap; }
    std::vector<u8> GetFlowMap () { return m_flowMap; }
    
    CPlane GetWaterBase() { return m_waterBase; }
    void SetWaterHeight(float height);
    
    FFTWaterModel GetWaterModel() { return m_waterModel; }
    
private:
    
    void GenerateVariationMap();
    void GenerateFlowMap();
    
    std::vector<GLuint> m_heightMapsIDs;
    std::vector<GLuint> m_normalMapsIDs;
    GLuint m_variationMapID;
    GLuint m_flowMapID;
    
    std::vector<std::vector<u8>> m_heightMaps;
    std::vector<std::vector<u8>> m_normalMaps;
    std::vector<u8> m_variationMap;
    std::vector<u8> m_flowMap;
    
    CPlane m_waterBase;
    FFTWaterModel m_waterModel;
};

#endif //INCLUDED_OCEANWATER
