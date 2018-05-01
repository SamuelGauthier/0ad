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
#include "lib/external_libraries/fftw.h" // needs to be here otherwise std::complex does not work

#include "maths/Vector2D.h"
#include "maths/Vector3D.h"
#include "maths/Vector4D.h"

#include "graphics/TextureManager.h"

#include "renderer/PhysicalWaterModel.h"

using TupleVecComplexF = std::tuple<std::vector<std::complex<float>>, std::vector<std::complex<float>>>;
using TupleVecU8 = std::tuple<std::vector<u8>, std::vector<u8>>;
using TupleVecVecU8 = std::tuple<std::vector<std::vector<u8>>, std::vector<std::vector<u8>>>;
using VecComplexF = std::vector<std::complex<float>>;

class FFTWaterModel
{
public:
    struct FFTWaterProperties
    {
        float m_amplitude;
        u16 m_windSpeed;
        CVector2D m_windDirection;
        float m_lambda;
        float m_L;
        u16 m_resolution;
        u16 m_width;
		double m_time;
		FFTWaterProperties(float amplitude = 3e-7, u16 windSpeed = 30,
                        CVector2D windDirection = CVector2D(1, 1),
                        float lambda = 1, float l = 0.1, u16 resolution = 1024,
                        u16 width = 2000, double time = 0.0) :
			m_amplitude{ amplitude }, m_windSpeed{ windSpeed },
            m_windDirection{ windDirection.Normalized() },
        	m_lambda{ lambda }, m_L{ l }, m_resolution{ resolution },
            m_width{ width }, m_time{ time } {}
    };
    
	FFTWaterModel(std::vector<FFTWaterProperties> waterProperties);
	~FFTWaterModel();

    TupleVecVecU8 GenerateHeightAndNormalMaps();
    
    std::vector<FFTWaterProperties> GetWaterProperties() { return m_WaterProperties; }

private:

    std::vector<FFTWaterProperties> m_WaterProperties;
	std::vector<u8> m_VariationMap;

	TupleVecU8 GetHeightAndNormalMapAtTime(double time, FFTWaterProperties waterProps, VecComplexF* h_0, VecComplexF* h_0_star);
	TupleVecComplexF ComputePhillipsSpectrum(FFTWaterProperties waterProps);

	std::complex<float> GetHTildeAt(u16 n, u16 m, double time, FFTWaterProperties waterProperty, VecComplexF* h_0, VecComplexF* h_0_star);
};
