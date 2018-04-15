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

#include "math.h"

#include "renderer/Renderer.h"

#include "FFTWaterModel.h"

FFTWaterModel::FFTWaterModel()
{
	// TODO: temporary value
	SetMaxHeight(5.0f);
	SetMinHeight(-5.0f);
	//m_maxHeight = 5.0f;
	//m_minHeight = -5.0f;
	//SetMaxHeight(5.0f);
	//SetMinHeight(-5.0f);

	// TODO: make the update useless and pass a heightmap texture to the GPU
}

FFTWaterModel::~FFTWaterModel() {}

void FFTWaterModel::Update(double time, CVector4D& point)
{
	point.Z += sin(5 * time + point.X + point.Y);// / 5;
}

Handle FFTWaterModel::GetHeightMapAtTime(double time)
{
	return NULL;
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

