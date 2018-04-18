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

#include <complex>

#include "maths/Vector2D.h"
#include "maths/Vector4D.h"

#include "graphics/TextureManager.h"

#include "renderer/PhysicalWaterModel.h"

class FFTWaterModel : public PhysicalWaterModel
{
public:
	FFTWaterModel();
	~FFTWaterModel();

	void Update(double time, CVector4D& point);
	Handle GetHeightMapAtTime(double time);
	CTexturePtr GetHeightMapAtLevel(int level);
	void GenerateHeightMaps();
	void FFTTest();

	struct WaterProperties
	{
		double m_Amplitude;
		u16 m_WindSpeed;
		double m_Lambda;
		u16 m_Resolution;
		u16 m_Width;
	};


private:
	CTexturePtr m_HeightMaps[60];

	std::complex<float> PhillipsSpectrum(CVector2D k);
};
