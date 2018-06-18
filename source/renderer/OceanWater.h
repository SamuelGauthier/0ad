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

#include "lib/ogl.h"
#include "maths/Plane.h"

#include "renderer/FFTWaterModel.h"

/**
 * Class which holds the vector displacment maps, the normal maps, the variation
 * map.
 */
class COceanWater
{
public:

    /**
     * @brief Constructs an OceanWater with a given water model.
     * @param waterModel the water model
     */ 
    COceanWater(CFFTWaterModel waterModel);

    /**
     * @brief Destructor
     */
    ~COceanWater();
    
    /**
     * @return Returns the maximum height of the water
     */
    float GetMaxWaterHeight();

    /**
     * @return Returns the minimum height of the water
     */
    float GetMinWaterHeight();

    /**
     * @return Returns height of the water
     */
    float GetWaterHeight();
    
    /**
     * @brief Updates the base height of the water
     */
    void UpdateWaterHeight();

    /**
     * @brief Creates the vector displacement fields and normal maps.
     */
    void GenerateWaterWaves();

    /**
     * @brief Generates a variation map.
     */
    void GenerateVariationMap();

    /**
     * @return Returns all the vector displacment fields of the water.
     */
    std::vector<std::vector<u8>> GetVectorDisplacementFields() { return m_vectorDisplacementFields; }

    /**
     * @return Returns all the normal maps of the water.
     */
    std::vector<std::vector<u8>> GetNormalMaps () { return m_normalMaps; }

    /**
     * @return Returns the variation map.
     */
    std::vector<GLfloat> GetVariationMap () { return m_variationMap; }

    /**
     * @return Returns the plane description of the water.
     */
    CPlane GetWaterBase() { return m_waterBase; }

    /**
     * @return Returns the water model.
     */
    CFFTWaterModel GetWaterModel() { return m_waterModel; }
    
private:
    
    std::vector<std::vector<u8>> m_vectorDisplacementFields;
    std::vector<std::vector<u8>> m_normalMaps;
    std::vector<GLfloat> m_variationMap;
    
    CPlane m_waterBase;
    CFFTWaterModel m_waterModel;
    
};

#endif //INCLUDED_OCEANWATER
