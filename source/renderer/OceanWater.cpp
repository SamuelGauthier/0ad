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

// Taken from https://gamedev.stackexchange.com/questions/23625/how-do-you-generate-tileable-perlin-noise
// And from http://staffwww.itn.liu.se/~stegu/aqsis/aqsis-newnoise/noise1234.cpp,
// http://staffwww.itn.liu.se/~stegu/aqsis/aqsis-newnoise/noise1234.h
// This is the new and improved, C(2) continuous interpolant
#define FADE(t) ( t * t * t * ( t * ( t * 6 - 15 ) + 10 ) )

#define FASTFLOOR(x) ( ((x)>0) ? ((int)x) : ((int)x-1 ) )
#define LERP(t, a, b) ((a) + (t)*((b)-(a)))

u8 COceanWater::perm[] = {151,160,137,91,90,15,
    131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
    190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
    88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
    77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
    102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
    135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
    5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
    223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
    129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
    251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
    49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
    138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,
    151,160,137,91,90,15,
    131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
    190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
    88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
    77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
    102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
    135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
    5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
    223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
    129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
    251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
    49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
    138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
};

COceanWater::COceanWater(CFFTWaterModel waterModel) : m_waterModel(waterModel)
{
}

COceanWater::~COceanWater() {}

//void COceanWater::SetWaterHeight(float height)
//{
//    m_waterBase.Set(CVector3D(0.f, 1.f, 0.f), CVector3D(0.f, height, 0.f));
//}

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
    return m_waterBase.m_Dist;
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
	// TODO: implement
    /*
    int size = 2048;
    Noise2D noise(size);
    
    m_variationMap = std::vector<GLfloat>(size*size);
    
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            m_variationMap[i*size + j] = (GLfloat) noise(i*0.1, j*0.1);
        }
    }
    
    LOGWARNING("%f", m_variationMap[0]);
    LOGWARNING("%f", m_variationMap[200]);
    LOGWARNING("%f", m_variationMap[456]);
    LOGWARNING("%f", m_variationMap[123]);
    LOGWARNING("%f", m_variationMap[1230]);
    */
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

float COceanWater::grad(int hash, float x, float y) {
    int h = hash & 7;      // Convert low 3 bits of hash code
    float u = h<4 ? x : y;  // into 8 simple gradient directions,
    float v = h<4 ? y : x;  // and compute the dot product with (x,y).
    return ((h&1)? -u : u) + ((h&2)? -2.0*v : 2.0*v);
}

//---------------------------------------------------------------------
/** 2D float Perlin periodic noise.
 */
float COceanWater::pnoise(float x, float y, int px, int py)
{
    int ix0, iy0, ix1, iy1;
    float fx0, fy0, fx1, fy1;
    float s, t, nx0, nx1, n0, n1;
    
    ix0 = FASTFLOOR( x ); // Integer part of x
    iy0 = FASTFLOOR( y ); // Integer part of y
    fx0 = x - ix0;        // Fractional part of x
    fy0 = y - iy0;        // Fractional part of y
    fx1 = fx0 - 1.0f;
    fy1 = fy0 - 1.0f;
    ix1 = (( ix0 + 1 ) % px) & 0xff;  // Wrap to 0..px-1 and wrap to 0..255
    iy1 = (( iy0 + 1 ) % py) & 0xff;  // Wrap to 0..py-1 and wrap to 0..255
    ix0 = ( ix0 % px ) & 0xff;
    iy0 = ( iy0 % py ) & 0xff;
    
    t = FADE( fy0 );
    s = FADE( fx0 );
    
    nx0 = grad(perm[ix0 + perm[iy0]], fx0, fy0);
    nx1 = grad(perm[ix0 + perm[iy1]], fx0, fy1);
    n0 = LERP( t, nx0, nx1 );
    
    nx0 = grad(perm[ix1 + perm[iy0]], fx1, fy0);
    nx1 = grad(perm[ix1 + perm[iy1]], fx1, fy1);
    n1 = LERP(t, nx0, nx1);
    
    return 0.507f * ( LERP( s, n0, n1 ) );
}

