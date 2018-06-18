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

/*
 * Based on Jerry Tessendorf's paper "Simulating Ocean Water" 2004 but not in
 * real-time. Implementation based on the work of Jiashuo Li et al.,
 *  https://github.com/JiashuoLi/OceanSurface
 */

#include "precompiled.h"

#include "FFTWaterModel.h"

#include <random>
#include <time.h>
#include <math.h>
#include <memory>

#include "ps/CLogger.h"

#define G 9.81
#define SQRT_0_5 0.7071067 // 0.707106781186547524
#define K_VEC(n, m, N, L) CVector2D(2 * M_PI * (n - N / 2) / L, 2 * M_PI * (m  - N / 2) / L)

/**
 * @brief Creates a water model with the given set of properties.
 *
 * @param waterProperties the set of water properties
 */
CFFTWaterModel::CFFTWaterModel(std::vector<SFFTWaterProperties> waterProperties) : m_WaterProperties(waterProperties)
{
}

CFFTWaterModel::~CFFTWaterModel() {}

/**
 * @brief Populates the 2d arrays given in argument with the vector
 *
 * displacment fields and the normal maps.
 * @param heightMaps an empty vector displacment field container
 * @param normalMaps an empty normal map container
 */
void CFFTWaterModel::GenerateHeightAndNormalMaps(VecVecU8* heightMaps, VecVecU8* normalMaps)
{
    heightMaps->clear();
    normalMaps->clear();
    
    for (size_t i = 0; i < m_WaterProperties.size(); i++) {
        u32 resolution = m_WaterProperties.at(i).m_resolution * m_WaterProperties.at(i).m_resolution;
        CFFTWaterModel::VecComplexF h_0 = CFFTWaterModel::VecComplexF(resolution);
        CFFTWaterModel::VecComplexF h_0_star = CFFTWaterModel::VecComplexF(resolution);
        
        ComputePhillipsSpectrum(m_WaterProperties.at(i), &h_0, &h_0_star);
        
        resolution *= 3;
        std::vector<u8> heightMap = std::vector<u8>(resolution);
        std::vector<u8> normalMap = std::vector<u8>(resolution);
        
        GetHeightAndNormalMapAtTime(m_WaterProperties.at(i).m_time, m_WaterProperties.at(i), &h_0, &h_0_star, &heightMap, &normalMap);
        
        heightMaps->push_back(heightMap);
        normalMaps->push_back(normalMap);
    }
}

/**
 * @brief Generates a vector displacement field and normal map given a set
 * of parameters and the given fourier amplitudes. The values are converted
 * into RGB. Erases and populates the \p pixelHeightMap and \p
 * pixelNormalMap
 *
 * @param time the time
 * @param waterProps the water properties
 * @param h_0 the precomputed fourier amplitudes
 * @param h_0_star the complex conjugate of the fourier amplitudes
 * @param pixelHeightMap empty array with size specified /p waterProps
 * @param pixelNormalMap empty array with size specified /p waterProps
 */
void CFFTWaterModel::GetHeightAndNormalMapAtTime(double time, SFFTWaterProperties waterProps, VecComplexF* h_0, VecComplexF* h_0_star, std::vector<u8>* pixelHeightMap, std::vector<u8>* pixelNormalMap)
{
    fftwf_complex* in_height;
    fftwf_complex* in_slope_x;
    fftwf_complex* in_slope_z;
    fftwf_complex* in_D_x;
    fftwf_complex* in_D_z;
    
    fftwf_complex* out_height;
    fftwf_complex* out_slope_x;
    fftwf_complex* out_slope_z;
    fftwf_complex* out_D_x;
    fftwf_complex* out_D_z;
    
    fftwf_plan p_ifft_height;
    fftwf_plan p_ifft_slope_x;
    fftwf_plan p_ifft_slope_z;
    fftwf_plan p_ifft_D_x;
    fftwf_plan p_ifft_D_z;
    
    u32 resolution = waterProps.m_resolution * waterProps.m_resolution;
    
    if(pixelHeightMap->size() < resolution || pixelNormalMap->size() < resolution) return;
    
    std::vector<std::complex<float>> slope_x_term(resolution);
    std::vector<std::complex<float>> slope_z_term(resolution);

    std::vector<std::complex<float>> D_x_term(resolution);
    std::vector<std::complex<float>> D_z_term(resolution);
    
    std::vector<std::complex<float>> h_tilde(resolution);

    std::vector<CVector3D> heightField(resolution);
    std::vector<CVector3D> normalMap(resolution);

    // Populate and prepare the arrays for the IFFT
    for (int i = 0; i < waterProps.m_resolution; i++) {
        for (int j = 0; j < waterProps.m_resolution; j++) {
            
            int index = j * waterProps.m_resolution + i;
            
            h_tilde[index] = GetHTildeAt(i, j, time, waterProps, h_0, h_0_star);
            
            CVector2D k = K_VEC(i, j, waterProps.m_resolution, waterProps.m_width);
            float kLength = k.Length();
            CVector2D kNormalized = kLength == 0 ? k : k.Normalized();
            
            slope_x_term[index] = std::complex<float>(0, k.X) * h_tilde[index];
            slope_z_term[index] = std::complex<float>(0, k.Y) * h_tilde[index];
            
            D_x_term[index] = std::complex<float>(0, -kNormalized.X) * h_tilde[index];
            D_z_term[index] = std::complex<float>(0, -kNormalized.Y) * h_tilde[index];
        }
    }
    
    in_height = reinterpret_cast<fftwf_complex*>(&h_tilde[0]);
    in_slope_x = reinterpret_cast<fftwf_complex*>(&slope_x_term[0]);
    in_slope_z = reinterpret_cast<fftwf_complex*>(&slope_z_term[0]);
    in_D_x = reinterpret_cast<fftwf_complex*>(&D_x_term[0]);
    in_D_z = reinterpret_cast<fftwf_complex*>(&D_z_term[0]);
    
    out_height = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * resolution);
    out_slope_x = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * resolution);
    out_slope_z = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * resolution);
    out_D_x = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * resolution);
    out_D_z = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * resolution);
    
    p_ifft_height = fftwf_plan_dft_2d(waterProps.m_resolution, waterProps.m_resolution, in_height, out_height, FFTW_BACKWARD, FFTW_ESTIMATE);
    p_ifft_slope_x = fftwf_plan_dft_2d(waterProps.m_resolution, waterProps.m_resolution, in_slope_x, out_slope_x, FFTW_BACKWARD, FFTW_ESTIMATE);
    p_ifft_slope_z = fftwf_plan_dft_2d(waterProps.m_resolution, waterProps.m_resolution, in_slope_z, out_slope_z, FFTW_BACKWARD, FFTW_ESTIMATE);
    p_ifft_D_x = fftwf_plan_dft_2d(waterProps.m_resolution, waterProps.m_resolution, in_D_x, out_D_x, FFTW_BACKWARD, FFTW_ESTIMATE);
    p_ifft_D_z = fftwf_plan_dft_2d(waterProps.m_resolution, waterProps.m_resolution, in_D_z, out_D_z, FFTW_BACKWARD, FFTW_ESTIMATE);
    
	// Compute the IFFT
    fftwf_execute(p_ifft_height);
    fftwf_execute(p_ifft_slope_x);
    fftwf_execute(p_ifft_slope_z);
    fftwf_execute(p_ifft_D_x);
    fftwf_execute(p_ifft_D_z);
    
    float min_height_x = std::numeric_limits<float>().max();
    float max_height_x = -min_height_x;
    float min_height_y = std::numeric_limits<float>().max();
    float max_height_y = -min_height_y;
    float min_height_z = std::numeric_limits<float>().max();
    float max_height_z = -min_height_z;
    float min_slope_x = std::numeric_limits<float>().max();
    float max_slope_x = -min_slope_x;
    float min_slope_z = std::numeric_limits<float>().max();
    float max_slope_z = -min_slope_z;
    
    // Build the vector displacement field
    for (int i = 0; i < waterProps.m_resolution; i++) {
        for (int j = 0; j < waterProps.m_resolution; j++) {
            int index = j * waterProps.m_resolution + i;
            float sign = 1;
            
            if((i + j) % 2) sign = -1;
            
            heightField[index] = CVector3D(- sign * waterProps.m_lambda * out_D_x[index][0],
                                             sign * out_height[index][0],
                                           - sign * waterProps.m_lambda * out_D_z[index][0]);
            
            if(heightField[index].X > max_height_x) max_height_x = heightField[index].X;
            if(heightField[index].Y > max_height_y) max_height_y = heightField[index].Y;
            if(heightField[index].Z > max_height_z) max_height_z = heightField[index].Z;
            if(heightField[index].X < min_height_x) min_height_x = heightField[index].X;
            if(heightField[index].Y < min_height_y) min_height_y = heightField[index].Y;
            if(heightField[index].Z < min_height_z) min_height_z = heightField[index].Z;
            
            normalMap[index] = CVector3D(sign * out_slope_x[index][0],
                                           -1,
                                         sign * out_slope_z[index][0]).Normalized();
            
            if(normalMap[index].X > max_slope_x) max_slope_x = normalMap[index].X;
            if(normalMap[index].Z > max_slope_z) max_slope_z = normalMap[index].Z;
            if(normalMap[index].X < min_slope_x) min_slope_x = normalMap[index].X;
            if(normalMap[index].Z < min_slope_z) min_slope_z = normalMap[index].Z;
        }
    }
    
	// Transform form floating point values to pixel data (unsigned byte)
    for (int i = 0; i < waterProps.m_resolution; i++) {
        for (int j = 0; j < waterProps.m_resolution; j++) {
            int index = j * waterProps.m_resolution + i;
            int index2 = 3*index;
            
            pixelHeightMap->at(index2) = (u8) round((heightField[index].X - min_height_x) * 255.0 / (max_height_x - min_height_x));
            pixelHeightMap->at(index2 + 1) = (u8) round((heightField[index].Y - min_height_y) * 255.0 / (max_height_y - min_height_y));
            pixelHeightMap->at(index2 + 2) = (u8) round((heightField[index].Z - min_height_z) * 255.0 / (max_height_z - min_height_z));

			pixelNormalMap->at(index2) = (u8)round((normalMap[index].X - min_slope_x) * 255.0 / (max_slope_x -min_slope_x));
            pixelNormalMap->at(index2 + 1) = (u8) round((normalMap[index].Z - min_slope_z) * 255.0 / (max_slope_z - min_slope_z));
            pixelNormalMap->at(index2 + 2) = (u8) 255.0;
            
        }
    }
    
    fftwf_destroy_plan(p_ifft_height);
    fftwf_destroy_plan(p_ifft_slope_x);
    fftwf_destroy_plan(p_ifft_slope_z);
    fftwf_destroy_plan(p_ifft_D_x);
    fftwf_destroy_plan(p_ifft_D_z);
    
	fftw_free(out_height);
    fftw_free(out_slope_x);
    fftw_free(out_slope_z);
    fftw_free(out_D_x);
    fftw_free(out_D_z);
}

/**
 * @brief Precomputes Phillips' spectrum given a set of parameters.
 *
 * Populates the \p h_0 and \p h_0_star
 * @param waterProps the water properties
 * @param h_0 empty array which will be populated.
 * @param h_0_star empty array which will be populated.
 */
void CFFTWaterModel::ComputePhillipsSpectrum(SFFTWaterProperties waterProp, VecComplexF* h_0, VecComplexF* h_0_star)
{
    float L = (waterProp.m_windSpeed * waterProp.m_windSpeed) / G;
    float L2 = L * L;
    float l = 0.1f;

    std::default_random_engine generator;
    std::normal_distribution<float> normal_dist;
    generator.seed(std::time(nullptr));
    
    for (u16 i = 0; i < waterProp.m_resolution; i++) {
        for (u16 j = 0; j < waterProp.m_resolution; j++) {
            u32 index = i * waterProp.m_resolution + j;
            CVector2D k = K_VEC(i, j, waterProp.m_resolution, waterProp.m_width);
            CVector2D normK = k.Normalized();
            float lengthK2 = k.LengthSquared();
            
            if(lengthK2 == 0.0f)
            {
                h_0->at(index) = 0.0f;
                
                h_0_star->at(index) = 0.0f;
                continue;
            }

            float factor = waterProp.m_amplitude * exp(-lengthK2 * l * l) *
            exp(-1 /(lengthK2 * L2)) / (lengthK2 * lengthK2);
            float p = factor * pow(normK.Dot(waterProp.m_windDirection), 2);

            float xi_r = normal_dist(generator);
            float xi_i = normal_dist(generator);
                
            h_0->at(index) = (float)SQRT_0_5 * std::complex<float>(xi_r, xi_i) * sqrt(p);
                
            p = factor * pow(-normK.Dot(waterProp.m_windDirection), 2);
            
            h_0_star->at(index) = (float)SQRT_0_5 * std::complex<float>(xi_r, -xi_i) * sqrt(p);

        }
    }
}

/**
 * @brief Computes the sum of two Fourier amplitudes at a point (\p n, \p m) 
 *
 * @param n width index
 * @param m height index
 * @param time the time
 * @param waterProperty the water property
 * @param h_0 the precomputed Fourier amplitudes
 * @param h_0_star the complex conjugate of the Fourier amplitudes
 *
 * @return The sum of the two Fourier amplitudes
 */
std::complex<float> CFFTWaterModel::GetHTildeAt(u16 n, u16 m, double time, SFFTWaterProperties waterProperty, CFFTWaterModel::VecComplexF* h_0, CFFTWaterModel::VecComplexF* h_0_star)
{
    int index = n * waterProperty.m_resolution + m;
    CVector2D k = K_VEC(n, m, waterProperty.m_resolution, waterProperty.m_width);
    float w = sqrt(k.Length() * G);
    
    std::complex<float> term1 = h_0->at(index) * exp(std::complex<float>(0.0f, w * time));
    std::complex<float> term2 = h_0_star->at(index) * exp(std::complex<float>(0.0f, -w * time));
    
    return term1 + term2;
}
