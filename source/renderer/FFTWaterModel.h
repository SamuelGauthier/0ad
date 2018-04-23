/* Copyright (C) 2017 Wildfire Games.
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

#include <complex.h>
#include "lib/external_libraries/fftw.h"

#include "maths/Vector2D.h"
#include "maths/Vector3D.h"
#include "maths/Vector4D.h"

#include "graphics/TextureManager.h"

#include "renderer/PhysicalWaterModel.h"

class FFTWaterModel : public PhysicalWaterModel
{
public:
    struct WaterProperties
    {
        float m_Amplitude;
        u16 m_WindSpeed;
        CVector2D m_WindDirection;
        float m_Lambda;
        float m_L;
        u16 m_Resolution;
        u16 m_Width;
        WaterProperties(float amplitude, u16 windSpeed, CVector2D windDirection, float lambda, float l, u16 resolution, u16 width) : m_Amplitude{amplitude}, m_WindSpeed{windSpeed}, m_WindDirection{windDirection.Normalized()}, m_Lambda{lambda}, m_L{l}, m_Resolution{resolution}, m_Width{width} {}
    };
    
	FFTWaterModel(WaterProperties waterProperties);
	~FFTWaterModel();

	void Update(double time, CVector4D& point);
    void GetHeightMapAtTime(double time, std::vector<u8> heightMap, std::vector<u8> normalMap);
	CTexturePtr GetHeightMapAtLevel(int level);
	void GenerateHeightMaps();
	void FFTTest();




private:
    WaterProperties m_WaterProperties;
	CTexturePtr m_HeightMaps[60];
    std::vector<std::complex<float>> m_h_0;
    std::vector<std::complex<float>> m_h_0_star;
    //std::vector<std::complex<float>> m_h_tilde;
    //std::vector<CVector3D> m_HeightField;
    //std::vector<CVector3D> m_NormalMap;

    void PrecomputePhillipsSpectrum();
	std::complex<float> GetHTildeAt(u16 n, u16 m, double time);
};
