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

#include "precompiled.h"

#include "FFTWaterModel.h"

#include <random>
#include <time.h>
#include <math.h>

//#include "ps/CLogger.h"
//#include "renderer/Renderer.h"

#define G 9.81
#define SQRT_0_5 0.7071067 // 0.707106781186547524
#define K_VEC(n, m, N, L) CVector2D(2 * M_PI * (n - N / 2) / L, 2 * M_PI * (m  - N / 2) / L)

CFFTWaterModel::CFFTWaterModel(std::vector<SFFTWaterProperties> waterProperties) : m_WaterProperties(waterProperties)
{
}

CFFTWaterModel::~CFFTWaterModel() {}

CFFTWaterModel::TupleVecU8 CFFTWaterModel::GetHeightAndNormalMapAtTime(double time, SFFTWaterProperties waterProps, VecComplexF* h_0, VecComplexF* h_0_star)
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
    
    std::complex<float>* slope_x_term = new std::complex<float>[resolution];
    std::complex<float>* slope_z_term = new std::complex<float>[resolution];
    
    std::complex<float>* D_x_term = new std::complex<float>[resolution];
    std::complex<float>* D_z_term = new std::complex<float>[resolution];
    
    std::complex<float>* h_tilde = new std::complex<float>[resolution];
    
    CVector3D* heightField = new CVector3D[resolution * 3];
    CVector3D* normalMap = new CVector3D[resolution * 3];

	std::vector<u8> pixelHeightMap = std::vector<u8>(resolution * 3);
	std::vector<u8> pixelNormalMap = std::vector<u8>(resolution * 3);

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
    
    in_height = (fftwf_complex*) h_tilde;
    in_slope_x = (fftwf_complex*) slope_x_term;
    in_slope_z = (fftwf_complex*) slope_z_term;
    in_D_x = (fftwf_complex*) D_x_term;
    in_D_z = (fftwf_complex*) D_z_term;
    
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
    
    float min_slope = std::numeric_limits<float>().max();
    float max_slope = -min_slope;
    float min_height = min_slope;
    float max_height = -min_slope;
    
    for (int i = 0; i < waterProps.m_resolution; i++) {
        for (int j = 0; j < waterProps.m_resolution; j++) {
            int index = i * waterProps.m_resolution + j;
            float sign = 1;
            
            if((i + j) % 2) sign = -1;
            
            heightField[index] = CVector3D(- sign * waterProps.m_lambda * out_D_x[index][0],
                                             sign * out_height[index][0],
                                           - sign * waterProps.m_lambda * out_D_z[index][0]);
            
            if(heightField[index].X > max_height) max_height = heightField[index].X;
            if(heightField[index].Y > max_height) max_height = heightField[index].Y;
            if(heightField[index].Z > max_height) max_height = heightField[index].Z;
            if(heightField[index].X < min_height) min_height = heightField[index].X;
            if(heightField[index].Y < min_height) min_height = heightField[index].Y;
            if(heightField[index].Z < min_height) min_height = heightField[index].Z;
            
            normalMap[index] = CVector3D(sign * out_slope_x[index][0],
                                           -1,
                                           sign * out_slope_z[index][0]).Normalized();
            
            if(normalMap[index].X > max_slope) max_slope = normalMap[index].X;
            if(normalMap[index].Y > max_slope) max_slope = normalMap[index].Y;
            if(normalMap[index].Z > max_slope) max_slope = normalMap[index].Z;
            if(normalMap[index].X < min_slope) min_slope = normalMap[index].X;
            if(normalMap[index].Y < min_slope) min_slope = normalMap[index].Y;
            if(normalMap[index].Z < min_slope) min_slope = normalMap[index].Z;
        }
    }
    
	// Transform form floating point values to pixel data (unsigned byte)
    for (int i = 0; i < waterProps.m_resolution; i++) {
        for (int j = 0; j < waterProps.m_resolution; j++) {
            int index = i * waterProps.m_resolution + j;
            int index2 = 3*index;
            
            pixelHeightMap[index2] = (u8) round((heightField[index].X - min_height) * 255 / (max_height - min_height));
            pixelHeightMap[index2 + 1] = (u8) round((heightField[index].Y - min_height) * 255 / (max_height - min_height));
            pixelHeightMap[index2 + 2] = (u8) round((heightField[index].Z - min_height) * 255 / (max_height - min_height));

            pixelNormalMap[index2] = (u8) round((normalMap[index].X - min_slope) * 255 / (max_slope - min_slope));
            pixelNormalMap[index2 + 1] = (u8) round((normalMap[index].Y - min_slope) * 255 / (max_slope - min_slope));
            pixelNormalMap[index2 + 2] = (u8) round((normalMap[index].Z - min_slope) * 255 / (max_slope - min_slope));
            
        }
    }
    
    fftwf_destroy_plan(p_ifft_height);
    fftwf_destroy_plan(p_ifft_slope_x);
    fftwf_destroy_plan(p_ifft_slope_z);
    fftwf_destroy_plan(p_ifft_D_x);
    fftwf_destroy_plan(p_ifft_D_z);
    
    SAFE_ARRAY_DELETE(slope_x_term);
    SAFE_ARRAY_DELETE(slope_z_term);
    SAFE_ARRAY_DELETE(D_x_term);
    SAFE_ARRAY_DELETE(D_z_term);
    SAFE_ARRAY_DELETE(h_tilde);
    SAFE_ARRAY_DELETE(heightField);
    SAFE_ARRAY_DELETE(normalMap);
    fftw_free(out_height);
    fftw_free(out_slope_x);
    fftw_free(out_slope_z);
    fftw_free(out_D_x);
    fftw_free(out_D_z);

	return std::make_tuple(pixelHeightMap, pixelNormalMap);
}

CFFTWaterModel::TupleVecVecU8 CFFTWaterModel::GenerateHeightAndNormalMaps()
{
    std::vector<std::vector<u8>> heightMaps;
    std::vector<std::vector<u8>> normalMaps;
    
    for (size_t i = 0; i < m_WaterProperties.size(); i++) {
        u32 resolution = m_WaterProperties.at(i).m_resolution * m_WaterProperties.at(i).m_resolution;
        CFFTWaterModel::VecComplexF h_0 = CFFTWaterModel::VecComplexF(resolution);
        CFFTWaterModel::VecComplexF h_0_star = CFFTWaterModel::VecComplexF(resolution);
        
        std::tie(h_0, h_0_star) = ComputePhillipsSpectrum(m_WaterProperties.at(i));
        
        resolution *= 3;
        std::vector<u8> heightMap = std::vector<u8>(resolution);
        std::vector<u8> normalMap = std::vector<u8>(resolution);
        
        std::tie(heightMap, normalMap) = GetHeightAndNormalMapAtTime(m_WaterProperties.at(i).m_time, m_WaterProperties.at(i), &h_0, &h_0_star);
        
        heightMaps.push_back(heightMap);
        normalMaps.push_back(normalMap);
    }
    
    return std::make_tuple(heightMaps, normalMaps);
}

CFFTWaterModel::TupleVecComplexF CFFTWaterModel::ComputePhillipsSpectrum(SFFTWaterProperties waterProp)
{
    float L = (waterProp.m_windSpeed * waterProp.m_windSpeed) / G;
    float L2 = L * L;
    float l = 0.1f;

	CFFTWaterModel::VecComplexF h0 = CFFTWaterModel::VecComplexF(waterProp.m_resolution * waterProp.m_resolution);
	CFFTWaterModel::VecComplexF h0star = CFFTWaterModel::VecComplexF(waterProp.m_resolution * waterProp.m_resolution);

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
                h0[index] = 0.0f;
                h0star[index] = 0.0f;
                continue;
            }

            float factor = waterProp.m_amplitude * exp(-lengthK2 * l * l) *
            exp(-1 /(lengthK2 * L2)) / (lengthK2 * lengthK2);
            float p = factor * pow(normK.Dot(waterProp.m_windDirection), 2);

            float xi_r = normal_dist(generator);
            float xi_i = normal_dist(generator);
                
            h0[index] = (float)SQRT_0_5 * std::complex<float>(xi_r, xi_i) * sqrt(p);
                
            p = factor * pow(-normK.Dot(waterProp.m_windDirection), 2);
            h0star[index] = (float)SQRT_0_5 * std::complex<float>(xi_r, -xi_i) * sqrt(p);

        }
    }

	return std::make_tuple(h0, h0star);
}

std::complex<float> CFFTWaterModel::GetHTildeAt(u16 n, u16 m, double time, SFFTWaterProperties waterProperty, CFFTWaterModel::VecComplexF* h_0, CFFTWaterModel::VecComplexF* h_0_star)
{
    int index = n * waterProperty.m_resolution + m;
    CVector2D k = K_VEC(n, m, waterProperty.m_resolution, waterProperty.m_width);
    float w = sqrt(k.Length() * G);
    
    std::complex<float> term1 = h_0->at(index) * exp(std::complex<float>(0.0f, w * time));
    std::complex<float> term2 = h_0_star->at(index) * exp(std::complex<float>(0.0f, -w * time));
    
    return term1 + term2;
}