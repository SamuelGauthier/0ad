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

#include <random>
#include <time.h>

#include "lib/external_libraries/fftw.h"

#include "math.h"
#include "renderer/Renderer.h"

#include "FFTWaterModel.h"

#define G 9.81
#define SQRT_0_5 0.7071067 // 0.707106781186547524
#define K_VEC(n, m, N, L) CVector2D(2 * M_PI * (n - N / 2) / L, 2 * M_PI * (m  - N / 2) / L)

FFTWaterModel::FFTWaterModel(WaterProperties waterProperties) : m_WaterProperties(waterProperties)
{
    u32 length = m_WaterProperties.m_Resolution*m_WaterProperties.m_Resolution;
    m_h_0 = std::vector<std::complex<float>>(length);
    m_h_0_star = std::vector<std::complex<float>>(length);
    m_h_tilde = std::vector<std::complex<float>>(length);
    PrecomputePhillipsSpectrum();
	// TODO: temporary value
	SetMaxHeight(5.0f);
	SetMinHeight(-5.0f);
}

FFTWaterModel::~FFTWaterModel() {}

void FFTWaterModel::Update(double time, CVector4D& point)
{
	point.Z += sin(5 * time + point.X + point.Y);// / 5;
}

void FFTWaterModel::GetHeightMapAtTime(double time, std::vector<u8> heightMap, std::vector<u8> normalMap)
{
    fftwf_complex *in_height;
    fftwf_complex *in_slope_x;
    fftwf_complex *in_slope_z;
    fftwf_complex *in_D_x;
    fftwf_complex *in_D_z;
    
    fftwf_complex *out_height;
    fftwf_complex *out_slope_x;
    fftwf_complex *out_slope_z;
    fftwf_complex *out_D_x;
    fftwf_complex *out_D_z;
    
    fftwf_plan p_height;
    fftwf_plan p_slope_x;
    fftwf_plan p_slope_z;
    fftwf_plan p_D_x;
    fftwf_plan p_D_z;
    
    u32 resolution = m_WaterProperties.m_Resolution * m_WaterProperties.m_Resolution;
    
    std::complex<float>* slope_x_term = new std::complex<float>[resolution];
    std::complex<float>* slope_z_term = new std::complex<float>[resolution];
    
    std::complex<float>* D_x_term = new std::complex<float>[resolution];
    std::complex<float>* D_z_term = new std::complex<float>[resolution];
    
    std::vector<CVector3D>* m_HeightField = new std::<CVector3D>
    
    for (int i = 0; i < m_WaterProperties.m_Resolution; i++) {
        for (int j = 0; j < m_WaterProperties.m_Resolution; j++) {
            
            int index = j * m_WaterProperties.m_Resolution + i;
            
            m_h_tilde[index] = GetHTildeAt(i, j, time);
            
            CVector2D k = K_VEC(i, j, m_WaterProperties.m_Resolution, m_WaterProperties.m_Width);
            k.Normalize();
            
            slope_x_term[index] = std::complex<float>(0, k.X) * m_h_tilde[index];
            slope_z_term[index] = std::complex<float>(0, k.Y) * m_h_tilde[index];
            
            D_x_term[index] = std::complex<float>(0, -k.X) * m_h_tilde[index];
            D_z_term[index] = std::complex<float>(0, -k.Y) * m_h_tilde[index];
        }
    }
    
    in_height = (fftwf_complex*) &m_h_tilde;
    in_slope_x = (fftwf_complex*) slope_x_term;
    in_slope_z = (fftwf_complex*) slope_z_term;
    in_D_x = (fftwf_complex*) D_x_term;
    in_D_z = (fftwf_complex*) D_z_term;
    
    out_height = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * resolution);
    out_slope_x = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * resolution);
    out_slope_z = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * resolution);
    out_D_x = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * resolution);
    out_D_z = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * resolution);
    
    p_height = fftwf_plan_dft_2d(m_WaterProperties.m_Resolution, m_WaterProperties.m_Resolution, in_height, out_height, FFTW_BACKWARD, FFTW_ESTIMATE);
    p_slope_x = fftwf_plan_dft_2d(m_WaterProperties.m_Resolution, m_WaterProperties.m_Resolution, in_slope_x, out_slope_x, FFTW_BACKWARD, FFTW_ESTIMATE);
    p_slope_z = fftwf_plan_dft_2d(m_WaterProperties.m_Resolution, m_WaterProperties.m_Resolution, in_slope_z, out_slope_z, FFTW_BACKWARD, FFTW_ESTIMATE);
    p_D_x = fftwf_plan_dft_2d(m_WaterProperties.m_Resolution, m_WaterProperties.m_Resolution, in_D_x, out_D_x, FFTW_BACKWARD, FFTW_ESTIMATE);
    p_D_z = fftwf_plan_dft_2d(m_WaterProperties.m_Resolution, m_WaterProperties.m_Resolution, in_D_z, out_D_z, FFTW_BACKWARD, FFTW_ESTIMATE);
    
    fftwf_execute(p_height);
    fftwf_execute(p_slope_x);
    fftwf_execute(p_slope_z);
    fftwf_execute(p_D_x);
    fftwf_execute(p_D_z);
    
    float min_slope = std::numeric_limits<float>().max();
    float max_slope = -min_slope;
    float min_height = min_slope;
    float max_height = -min_slope;
    
    for (int i = 0; i < m_WaterProperties.m_Resolution; i++) {
        for (int j = 0; j < m_WaterProperties.m_Resolution; j++) {
            int index = i * m_WaterProperties.m_Resolution + j;
            float sign = 1;
            
            if((i + j) % 2) sign = -1;
            
            m_HeightField[index] = CVector3D(
                                             sign * m_WaterProperties.m_Lambda * out_D_x[index][0],//(n - N / 2) * L_x / N - sign * lambda * out_D_x[index][0],
                                             sign * out_height[index][0],
                                             sign * m_WaterProperties.m_Lambda * out_D_z[index][0]//(m - M / 2) * L_z / M - sign * lambda * out_D_z[index][0]
                                             );
            
            if(m_HeightField[index].X > max_height) max_height = m_HeightField[index].X;
            if(m_HeightField[index].Y > max_height) max_height = m_HeightField[index].Y;
            if(m_HeightField[index].Z > max_height) max_height = m_HeightField[index].Z;
            if(m_HeightField[index].X < min_height) min_height = m_HeightField[index].X;
            if(m_HeightField[index].Y < min_height) min_height = m_HeightField[index].Y;
            if(m_HeightField[index].Z < min_height) min_height = m_HeightField[index].Z;
            
            m_NormalMap[index] = CVector3D(sign * out_slope_x[index][0],
                                           1,//-1,
                                           sign * out_slope_z[index][0]).Normalized();
            
            if(m_NormalMap[index].X > max_slope) max_slope = m_NormalMap[index].X;
            if(m_NormalMap[index].Y > max_slope) max_slope = m_NormalMap[index].Y;
            if(m_NormalMap[index].Z > max_slope) max_slope = m_NormalMap[index].Z;
            if(m_NormalMap[index].X < min_slope) min_slope = m_NormalMap[index].X;
            if(m_NormalMap[index].Y < min_slope) min_slope = m_NormalMap[index].Y;
            if(m_NormalMap[index].Z < min_slope) min_slope = m_NormalMap[index].Z;
        }
    }
    
    for (int i = 0; i < m_WaterProperties.m_Resolution; i++) {
        for (int j = 0; j < m_WaterProperties.m_Resolution; j++) {
            int index = i * m_WaterProperties.m_Resolution + j;
            int index2 = 3*index;
            
            m_HeightField[index].X = round((m_HeightField[index].X - min_height) * 255 / (max_height - min_height));
            m_HeightField[index].Y = round((m_HeightField[index].Y - min_height) * 255 / (max_height - min_height));
            m_HeightField[index].Z = round((m_HeightField[index].Z - min_height) * 255 / (max_height - min_height));
            
            heightMap[index2] = (u8) m_HeightField[index].X;
            heightMap[index2 + 1] = (u8) m_HeightField[index].Y;
            heightMap[index2 + 2] = (u8) m_HeightField[index].Z;
            
            m_NormalMap[index].X = round((m_NormalMap[index].X - min_slope) * 255 / (max_slope - min_slope));
            m_NormalMap[index].Y = round((m_NormalMap[index].Y - min_slope) * 255 / (max_slope - min_slope));
            m_NormalMap[index].Z = round((m_NormalMap[index].Z - min_slope) * 255 / (max_slope - min_slope));
            
            normalMap[index2] = (u8) m_NormalMap[index].X;
            normalMap[index2 + 1] = (u8) m_NormalMap[index].Y;
            normalMap[index2 + 2] = (u8) m_NormalMap[index].Z;
        }
    }
    
    fftwf_destroy_plan(p_height);
    fftwf_destroy_plan(p_slope_x);
    fftwf_destroy_plan(p_slope_z);
    fftwf_destroy_plan(p_D_x);
    fftwf_destroy_plan(p_D_z);
    
    SAFE_ARRAY_DELETE(slope_x_term);//delete[] slope_x_term;
    SAFE_ARRAY_DELETE(slope_z_term);//delete[] slope_z_term;
    SAFE_ARRAY_DELETE(D_x_term);//delete[] D_x_term;
    SAFE_ARRAY_DELETE(D_z_term);//delete[] D_z_term;
    fftw_free(out_height);
    fftw_free(out_slope_x);
    fftw_free(out_slope_z);
    fftw_free(out_D_x);
    fftw_free(out_D_z);
    
}

CTexturePtr FFTWaterModel::GetHeightMapAtLevel(int level)
{
	// TODO: for the moment like this
	//return m_HeightMaps[level % ARRAY_SIZE(m_HeightMaps)];
	return m_HeightMaps[level];
}

void FFTWaterModel::GenerateHeightMaps()
{
    wchar_t pathname[PATH_MAX];
    
    for (int i = 0; i < 3; i++) {
        swprintf_s(pathname, ARRAY_SIZE(pathname), L"art/textures/terrain/types/water/heightmap%1d.png", i+1);
        CTextureProperties textureProps(pathname);
        textureProps.SetWrap(GL_REPEAT);
        CTexturePtr texture = g_Renderer.GetTextureManager().CreateTexture(textureProps);
        texture->Prefetch();
        m_HeightMaps[i] = texture;
    }
    
}

void FFTWaterModel::PrecomputePhillipsSpectrum()
{
    
    float L = (m_WaterProperties.m_WindSpeed * m_WaterProperties.m_WindSpeed) / G;
    float L2 = L * L;
    float l = 0.1;
    
    std::default_random_engine generator;
    std::normal_distribution<float> normal_dist;
    generator.seed(std::time(nullptr));
    
    for (u16 i = 0; i < m_WaterProperties.m_Resolution; i++) {
        for (u16 j = 0; j < m_WaterProperties.m_Resolution; j++) {
            u32 index = i * m_WaterProperties.m_Resolution + j;
            CVector2D k = K_VEC(i, j, m_WaterProperties.m_Resolution, m_WaterProperties.m_Width);
            CVector2D normK = k.Normalized();
            float lengthK2 = k.LengthSquared();
            
            float factor = m_WaterProperties.m_Amplitude * exp(-lengthK2 * l * l) *
            exp(-1 /(lengthK2 * L2)) / (lengthK2 * lengthK2);
            float p = factor * pow(normK.Dot(m_WaterProperties.m_WindDirection), 2);
            
            float xi_r = normal_dist(generator);
            float xi_i = normal_dist(generator);
            
            m_h_0[index] = (float)SQRT_0_5 * std::complex<float>(xi_r, xi_i) * sqrt(p);
            
            p = factor * pow(-normK.Dot(m_WaterProperties.m_WindDirection), 2);
            m_h_0_star[index] = (float)SQRT_0_5 * std::complex<float>(xi_r, -xi_i) * sqrt(p);
        }
    }
}

std::complex<float> FFTWaterModel::GetHTildeAt(u16 n, u16 m, double time)
{
    int index = n * m_WaterProperties.m_Resolution + m;
    CVector2D k = K_VEC(n, m, m_WaterProperties.m_Resolution, m_WaterProperties.m_Width);
    float w = sqrt(k.Length() * G);
    
    std::complex<float> term1 = m_h_0[index] * exp(std::complex<float>(0.0f, w * time));
    std::complex<float> term2 = m_h_0_star[index] * exp(std::complex<float>(0.0f, -w * time));
    return term1 + term2;
}

void FFTWaterModel::FFTTest()
{
    fftwf_complex *in, *out;
    fftwf_plan p;
    long N = 1;
    in = (fftwf_complex*) fftw_malloc(sizeof(fftwf_complex) * N);
    out = (fftwf_complex*) fftw_malloc(sizeof(fftwf_complex) * N);
    p = fftwf_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
    fftwf_execute(p); /* repeat as needed */
    fftwf_destroy_plan(p);
    fftw_free(in); fftw_free(out);
}
