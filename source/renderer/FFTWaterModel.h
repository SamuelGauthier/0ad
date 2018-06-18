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
#ifndef INCLUDED_FFTWATERMODEL
#define INCLUDED_FFTWATERMODEL

#include <complex>
#include <tuple>
#include "lib/external_libraries/fftw.h" // needs to be here otherwise std::complex does not work

#include "maths/Vector2D.h"
#include "maths/Vector3D.h"
#include "maths/Vector4D.h"

/**
 *  @brief Represents a water model which uses an IFFT to synthesize the waves.
 *  Based on Tessendorf's paper Simulating Ocean Water" 2004
 */
class CFFTWaterModel
{
public:

	using TupleVecComplexF = std::tuple<std::vector<std::complex<float>>, std::vector<std::complex<float>>>;
	using TupleVecU8 = std::tuple<std::vector<u8>, std::vector<u8>>;
	using TupleVecVecU8 = std::tuple<std::vector<std::vector<u8>>, std::vector<std::vector<u8>>>;
    using VecVecU8 = std::vector<std::vector<u8>>;
	using VecComplexF = std::vector<std::complex<float>>;

    /**
     * @brief Represents the different parameters used for water generation
     */
    struct SFFTWaterProperties
    {
        float m_amplitude;
        u16 m_windSpeed;
        CVector2D m_windDirection;
        float m_lambda;
        float m_L;
        u16 m_resolution;
        u16 m_width;
		double m_time;

        /** Creates a parameter container for the water */
		SFFTWaterProperties(float amplitude = 3e-7f, u16 windSpeed = 30,
                        CVector2D windDirection = CVector2D(1.0f, 1.0f),
                        float lambda = 1.0f, float l = 0.1f, u16 resolution = 1024,
                        u16 width = 2000, double time = 0.0) :
			m_amplitude{ amplitude }, m_windSpeed{ windSpeed },
            m_windDirection{ windDirection.Normalized() },
        	m_lambda{ lambda }, m_L{ l }, m_resolution{ resolution },
            m_width{ width }, m_time{ time } {}
    };
    
    /**
     * @brief Creates a water model with the given set of properties.
     * @param waterProperties the set of water properties
     */
	CFFTWaterModel(std::vector<SFFTWaterProperties> waterProperties);

	~CFFTWaterModel();

    /**
     * @brief Populates the 2d arrays given in argument with the vector
     * displacment fields and the normal maps.
     * @param heightMaps an empty vector displacment field container
     * @param normalMaps an empty normal map container
     */
    void GenerateHeightAndNormalMaps(VecVecU8* heightMaps, VecVecU8* normalMaps);

    /**
     * @return returns the set of water properties.
     */
    std::vector<SFFTWaterProperties> GetWaterProperties() { return m_WaterProperties; }

private:

    std::vector<SFFTWaterProperties> m_WaterProperties;
	std::vector<u8> m_VariationMap;

    /**
     * @brief Generates a vector displacement field and normal map given a set
     * of parameters and the given fourier amplitudes. The values are converted
     * into RGB. Erases and populates the \p pixelHeightMap and \p
     * pixelNormalMap
     * @param time the time
     * @param waterProps the water properties
     * @param h_0 the precomputed fourier amplitudes
     * @param h_0_star the complex conjugate of the fourier amplitudes
     * @param pixelHeightMap empty array with size specified /p waterProps
     * @param pixelNormalMap empty array with size specified /p waterProps
     */
    void GetHeightAndNormalMapAtTime(double time, SFFTWaterProperties waterProps, VecComplexF* h_0, VecComplexF* h_0_star, std::vector<u8>* pixelHeightMap, std::vector<u8>* pixelNormalMap);

    /**
     * @brief Precomputes Phillips' spectrum given a set of parameters.
     * Populates the \p h_0 and \p h_0_star
     * @param waterProps the water properties
     * @param h_0 empty array which will be populated.
     * @param h_0_star empty array which will be populated.
     */
	void ComputePhillipsSpectrum(SFFTWaterProperties waterProps, VecComplexF* h_0, VecComplexF* h_0_star);

    /**
     * @brief Computes the sum of two Fourier amplitudes at a point (\p n, \p m) 
     * @param n width index
     * @param m height index
     * @param time the time
     * @param waterProperty the water property
     * @param h_0 the precomputed Fourier amplitudes
     * @param h_0_star the complex conjugate of the Fourier amplitudes
     *
     * @return The sum of the two Fourier amplitudes
     */
	std::complex<float> GetHTildeAt(u16 n, u16 m, double time, SFFTWaterProperties waterProperty, VecComplexF* h_0, VecComplexF* h_0_star);
};

#endif // INCLUDED_FFTWATERMODEL
