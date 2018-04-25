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
#include <math.h>

//#include "lib/external_libraries/fftw.h"
#include "ps/CLogger.h"
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
    //m_h_tilde = std::vector<std::complex<float>>(length);
    //m_HeightField = std::vector<CVector3D>(length);
    //m_NormalMap = std::vector<CVector3D>(length);
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

void FFTWaterModel::GetHeightMapAtTime(double time, std::vector<u8>* heightMap, std::vector<u8>* normalMap)
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
    
    fftwf_plan p_ifft_height;
    fftwf_plan p_ifft_slope_x;
    fftwf_plan p_ifft_slope_z;
    fftwf_plan p_ifft_D_x;
    fftwf_plan p_ifft_D_z;
    
    u32 resolution = m_WaterProperties.m_Resolution * m_WaterProperties.m_Resolution;
    
    std::complex<float>* slope_x_term = new std::complex<float>[resolution];
    std::complex<float>* slope_z_term = new std::complex<float>[resolution];
    
    std::complex<float>* D_x_term = new std::complex<float>[resolution];
    std::complex<float>* D_z_term = new std::complex<float>[resolution];
    
    std::complex<float>* m_h_tilde = new std::complex<float>[resolution];
    
    //std::vector<CVector3D>* m_HeightField = new std::vector<CVector3D>[resolution * 3];
    //std::vector<CVector3D>* m_NormalMap = new std::vector<CVector3D>[resolution * 3];
    CVector3D* m_HeightField = new CVector3D[resolution * 3];
    CVector3D* m_NormalMap = new CVector3D[resolution * 3];

    
    //std::vector<CVector3D>* heightField = new std::vector<CVector3D>[resolution];
    //std::vector<CVector3D>* normalMap = new std::vector<CVector3D>[resolution];
    
#if 0
    std::complex<float> t = GetHTildeAt(2, 2, time);
    LOGWARNING("t @ 2,2: %f + (%f)i", t.real(), t.imag());
    CVector2D k = K_VEC(2, 2, m_WaterProperties.m_Resolution, m_WaterProperties.m_Width);
    LOGWARNING("k @ 2,2: (%f; %f)", k.X, k.Y);
#endif
    
    for (int i = 0; i < m_WaterProperties.m_Resolution; i++) {
        for (int j = 0; j < m_WaterProperties.m_Resolution; j++) {
            
            int index = j * m_WaterProperties.m_Resolution + i;
            
            m_h_tilde[index] = GetHTildeAt(i, j, time);
            
            CVector2D k = K_VEC(i, j, m_WaterProperties.m_Resolution, m_WaterProperties.m_Width);
            float kLength = k.Length();
            CVector2D kNormalized = kLength == 0 ? k : k.Normalized();
            
            slope_x_term[index] = std::complex<float>(0, k.X) * m_h_tilde[index];
            slope_z_term[index] = std::complex<float>(0, k.Y) * m_h_tilde[index];
            
            D_x_term[index] = std::complex<float>(0, -kNormalized.X) * m_h_tilde[index];
            D_z_term[index] = std::complex<float>(0, -kNormalized.Y) * m_h_tilde[index];
            
#if 0
            if(index == 10){
                std::complex<float> t = GetHTildeAt(i, j, time);
                LOGWARNING("t @ %u, %u: %f + (%f)i", i, j, t.real(), t.imag());
                LOGWARNING("m_h_tilde: %f + (%f)i", m_h_tilde[index].real(), m_h_tilde[index].imag());
                LOGWARNING("slope_x_term: %f + (%f)i", slope_x_term[index].real(), slope_x_term[index].imag());
                LOGWARNING("slope_z_term: %f + (%f)i", slope_z_term[index].real(), slope_z_term[index].imag());
                LOGWARNING("D_x_term: %f + (%f)i", D_x_term[index].real(), D_x_term[index].imag());
                LOGWARNING("D_z_term: %f + (%f)i", D_z_term[index].real(), D_z_term[index].imag());
            }
#endif
        }
    }
    
    in_height = (fftwf_complex*) m_h_tilde;
    in_slope_x = (fftwf_complex*) slope_x_term;
    in_slope_z = (fftwf_complex*) slope_z_term;
    in_D_x = (fftwf_complex*) D_x_term;
    in_D_z = (fftwf_complex*) D_z_term;
    
#if 0
    for (int i = 0; i < m_WaterProperties.m_Resolution; i++) {
        for (int j = 0; j < m_WaterProperties.m_Resolution; j++) {
            int index = j * m_WaterProperties.m_Resolution + i;

            LOGWARNING("in_slope_x: %f + (%f)i", in_slope_x[index][0], in_slope_x[index][1]);
            LOGWARNING("in_slope_z: %f + (%f)i", in_slope_z[index][0], in_slope_z[index][1]);
            LOGWARNING("in_D_x: %f + (%f)i", in_D_x[index][0], in_D_x[index][1]);
            LOGWARNING("in_D_z: %f + (%f)i", in_D_z[index][0], in_D_z[index][1]);

        }
    }
#endif
    out_height = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * resolution);
    out_slope_x = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * resolution);
    out_slope_z = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * resolution);
    out_D_x = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * resolution);
    out_D_z = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * resolution);
    
    p_ifft_height = fftwf_plan_dft_2d(m_WaterProperties.m_Resolution, m_WaterProperties.m_Resolution, in_height, out_height, FFTW_BACKWARD, FFTW_ESTIMATE);
    p_ifft_slope_x = fftwf_plan_dft_2d(m_WaterProperties.m_Resolution, m_WaterProperties.m_Resolution, in_slope_x, out_slope_x, FFTW_BACKWARD, FFTW_ESTIMATE);
    p_ifft_slope_z = fftwf_plan_dft_2d(m_WaterProperties.m_Resolution, m_WaterProperties.m_Resolution, in_slope_z, out_slope_z, FFTW_BACKWARD, FFTW_ESTIMATE);
    p_ifft_D_x = fftwf_plan_dft_2d(m_WaterProperties.m_Resolution, m_WaterProperties.m_Resolution, in_D_x, out_D_x, FFTW_BACKWARD, FFTW_ESTIMATE);
    p_ifft_D_z = fftwf_plan_dft_2d(m_WaterProperties.m_Resolution, m_WaterProperties.m_Resolution, in_D_z, out_D_z, FFTW_BACKWARD, FFTW_ESTIMATE);
    
    fftwf_execute(p_ifft_height);
    fftwf_execute(p_ifft_slope_x);
    fftwf_execute(p_ifft_slope_z);
    fftwf_execute(p_ifft_D_x);
    fftwf_execute(p_ifft_D_z);
    
    float min_slope = std::numeric_limits<float>().max();
    float max_slope = -min_slope;
    float min_height = min_slope;
    float max_height = -min_slope;
    
    for (int i = 0; i < m_WaterProperties.m_Resolution; i++) {
        for (int j = 0; j < m_WaterProperties.m_Resolution; j++) {
            int index = i * m_WaterProperties.m_Resolution + j;
            float sign = 1;
            
            if((i + j) % 2) sign = -1;
            
            //if(index < 3)
            //    LOGWARNING("before: %f %f %f", (float)out_height[index][0], (float)*out_D_x[index], (float)*out_D_z[index]);
            
            m_HeightField[index] = CVector3D(
                                             /*(i - m_WaterProperties.m_Resolution / 2) * m_WaterProperties.m_Width / m_WaterProperties.m_Resolution*/ - sign * m_WaterProperties.m_Lambda * out_D_x[index][0],//(n - N / 2) * L_x / N - sign * lambda * out_D_x[index][0],
                                             sign * out_height[index][0],
                                             /*(j - m_WaterProperties.m_Resolution / 2) * m_WaterProperties.m_Width / m_WaterProperties.m_Resolution*/ - sign * m_WaterProperties.m_Lambda * out_D_z[index][0]//(m - M / 2) * L_z / M - sign * lambda * out_D_z[index][0]
                                             );
#if 0

                LOGWARNING("sign, lambda: %f %f", sign, m_WaterProperties.m_Lambda);
                LOGWARNING("hf: %f %f %f", m_HeightField[index].X, m_HeightField[index].Y, m_HeightField[index].Z);
                LOGWARNING("h, Dx, Dz: %f %f %f", (float)out_height[index][0], (float)*out_D_x[index], (float)*out_D_z[index]);
#endif
            
            if(m_HeightField[index].X > max_height) max_height = m_HeightField[index].X;
            if(m_HeightField[index].Y > max_height) max_height = m_HeightField[index].Y;
            if(m_HeightField[index].Z > max_height) max_height = m_HeightField[index].Z;
            if(m_HeightField[index].X < min_height) min_height = m_HeightField[index].X;
            if(m_HeightField[index].Y < min_height) min_height = m_HeightField[index].Y;
            if(m_HeightField[index].Z < min_height) min_height = m_HeightField[index].Z;
            
            m_NormalMap[index] = CVector3D(sign * out_slope_x[index][0],
                                           -1,//-1,
                                           sign * out_slope_z[index][0]).Normalized();
            
            if(m_NormalMap[index].X > max_slope) max_slope = m_NormalMap[index].X;
            if(m_NormalMap[index].Y > max_slope) max_slope = m_NormalMap[index].Y;
            if(m_NormalMap[index].Z > max_slope) max_slope = m_NormalMap[index].Z;
            if(m_NormalMap[index].X < min_slope) min_slope = m_NormalMap[index].X;
            if(m_NormalMap[index].Y < min_slope) min_slope = m_NormalMap[index].Y;
            if(m_NormalMap[index].Z < min_slope) min_slope = m_NormalMap[index].Z;
        }
    }
    
#if 0
    LOGWARNING("max_height, min_height: %f %f", max_height, min_height);
    LOGWARNING("max_slope, min_slope: %f %f", max_slope, min_slope);
#endif
    
    for (int i = 0; i < m_WaterProperties.m_Resolution; i++) {
        for (int j = 0; j < m_WaterProperties.m_Resolution; j++) {
            int index = i * m_WaterProperties.m_Resolution + j;
            int index2 = 3*index;
            
            //if(index < 3)
            //    LOGWARNING("before: %f %f %f", m_HeightField[index].X, m_HeightField[index].Y, m_HeightField[index].Z);
            
            m_HeightField[index].X = round((m_HeightField[index].X - min_height) * 255 / (max_height - min_height));
            m_HeightField[index].Y = round((m_HeightField[index].Y - min_height) * 255 / (max_height - min_height));
            m_HeightField[index].Z = round((m_HeightField[index].Z - min_height) * 255 / (max_height - min_height));
            
#if 0
            if(index == 120){
                LOGWARNING("[In FFTWaterModel] x, y, z: %f %f %f", m_HeightField[index].X, m_HeightField[index].Y, m_HeightField[index].Z);
                LOGWARNING("[In FFTWaterModel] u8 x, y, z: %u %u %u", (u8)m_HeightField[index].X, (u8)m_HeightField[index].Y, (u8)m_HeightField[index].Z);
            }

#endif
            /*
            heightMap[index2] = (u8) m_HeightField[index].X;
            heightMap[index2 + 1] = (u8) m_HeightField[index].Y;
            heightMap[index2 + 2] = (u8) m_HeightField[index].Z;
             */
            (*heightMap)[index2] = (u8) m_HeightField[index].X;
            (*heightMap)[index2 + 1] = (u8) m_HeightField[index].Y;
            (*heightMap)[index2 + 2] = (u8) m_HeightField[index].Z;

#if 0
            if(index == 120){
                LOGWARNING("[In FFTWaterModel] u8 x, y, z: %u %u %u", (*heightMap)[index2], (*heightMap)[index2 + 1], (*heightMap)[index2 + 2]);
            }
            
#endif
            
#if 0
            LOGWARNING("x, y, z: %u %u %u", heightMap[index2], heightMap[index2 + 1], heightMap[index2 + 2]);
#endif
            
            m_NormalMap[index].X = round((m_NormalMap[index].X - min_slope) * 255 / (max_slope - min_slope));
            m_NormalMap[index].Y = round((m_NormalMap[index].Y - min_slope) * 255 / (max_slope - min_slope));
            m_NormalMap[index].Z = round((m_NormalMap[index].Z - min_slope) * 255 / (max_slope - min_slope));
            /*
            normalMap[index2] = (u8) m_NormalMap[index].X;
            normalMap[index2 + 1] = (u8) m_NormalMap[index].Y;
            normalMap[index2 + 2] = (u8) m_NormalMap[index].Z;
             */
            (*normalMap)[index2] = (u8) m_NormalMap[index].X;
            (*normalMap)[index2 + 1] = (u8) m_NormalMap[index].Y;
            (*normalMap)[index2 + 2] = (u8) m_NormalMap[index].Z;
            
            //if(index < 3)
            //    LOGWARNING("after: %u %u %u", heightMap[index2], heightMap[index2+1], heightMap[index2+2]);
        }
    }
    
    fftwf_destroy_plan(p_ifft_height);
    fftwf_destroy_plan(p_ifft_slope_x);
    fftwf_destroy_plan(p_ifft_slope_z);
    fftwf_destroy_plan(p_ifft_D_x);
    fftwf_destroy_plan(p_ifft_D_z);
    
    SAFE_ARRAY_DELETE(slope_x_term);//delete[] slope_x_term;
    SAFE_ARRAY_DELETE(slope_z_term);//delete[] slope_z_term;
    SAFE_ARRAY_DELETE(D_x_term);//delete[] D_x_term;
    SAFE_ARRAY_DELETE(D_z_term);//delete[] D_z_term;
    SAFE_ARRAY_DELETE(m_h_tilde);
    SAFE_ARRAY_DELETE(m_HeightField);
    SAFE_ARRAY_DELETE(m_NormalMap);
    fftw_free(out_height);
    fftw_free(out_slope_x);
    fftw_free(out_slope_z);
    fftw_free(out_D_x);
    fftw_free(out_D_z);
    
//    LOGWARNING("[In FFTWaterModel] h at 0: %u, %u, %u", heightMap->at(0), heightMap->at(1), heightMap->at(2));
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
            
            if(lengthK2 == 0.0f)
            {
                m_h_0[index] = 0.0f;
                m_h_0_star[index] = 0.0f;
                continue;
            }


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
    
#if 0
    if(n == 2 && m == 2){
        LOGWARNING("w @ 2,2: %f", w);
        LOGWARNING("k @ 2,2: (%f; %f)", k.X, k.Y);
        LOGWARNING("term1: %u and %f + (%f)i", index, term1.real(), term1.imag());
        LOGWARNING("term2: %u and %f; (%f)i", index, term2.real(), term2.imag());
        LOGWARNING("m_h_0: %u and %f; (%f)i", index, m_h_0[index].real(), m_h_0[index].imag());
        LOGWARNING("m_h_0_star: %u and %f; (%f)i", index, m_h_0_star[index].real(), m_h_0_star[index].imag());
                   
    }
#endif
    
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
