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
#include <tuple>
#include "lib/external_libraries/fftw.h"

#include "maths/Vector2D.h"
#include "maths/Vector3D.h"
#include "maths/Vector4D.h"

#include "graphics/TextureManager.h"

#include "renderer/PhysicalWaterModel.h"

using TupleVecComplexF = std::tuple<std::vector<std::complex<float>>, std::vector<std::complex<float>>>;
using TupleVecU8 = std::tuple<std::vector<u8>, std::vector<u8>>;

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
		double m_Time;
		WaterProperties(float amplitude = 3e-7, u16 windSpeed = 30, CVector2D windDirection = CVector2D(1, 1),
			float lambda = 1, float l = 0.1, u16 resolution = 1024, u16 width = 2000, double time = 0.0) :
			m_Amplitude{ amplitude }, m_WindSpeed{ windSpeed }, m_WindDirection{ windDirection.Normalized() },
			m_Lambda{ lambda }, m_L{ l }, m_Resolution{ resolution }, m_Width{ width }, m_Time{ time } {}
    };
    
	//FFTWaterModel(WaterProperties waterProperty);
	FFTWaterModel(std::vector<WaterProperties> waterProperties);
	~FFTWaterModel();

    void GetHeightMapAtTime(double time, std::vector<u8>* heightMap, std::vector<u8>* normalMap);
	std::vector<u8> GetHeightMapAtLevel(u8 level);
	std::vector<u8> GetNormalMapAtLevel(u8 level);
	std::vector<u8> GetVariationMap() { return m_VariationMap; }
	void GenerateHeightMaps();

private:
    WaterProperties m_WaterProperty;
    std::vector<WaterProperties> m_WaterProperties;
	std::vector<std::vector<u8>> m_HeightMaps;
	std::vector<std::vector<u8>> m_NormalMaps;
	std::vector<u8> m_VariationMap;
	
    std::vector<std::complex<float>> m_h_0;
    std::vector<std::complex<float>> m_h_0_star;

	TupleVecU8 GetHeightAndNormalMapAtTime(double time, WaterProperties waterProps);
	TupleVecComplexF ComputePhillipsSpectrum(WaterProperties waterProps);

	std::complex<float> GetHTildeAt(u16 n, u16 m, double time);
};
