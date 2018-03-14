/* Copyright (C) 2018 Wildfire Games.
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

#include "renderer/FFTWaterModel.h"
#include "renderer/Renderer.h"
#include "renderer/Water.h"
#include "renderer/WaterManager.h"

Water::Water(const PhysicalWaterModel& waterModel) : m_waterModel{ waterModel } {
	m_heightfield = 0;
	m_area = 0;
	// Temporary
	m_height = 5.0f;
	// nullPtr m_height = g_Renderer.GetWaterManager()->m_WaterHeight;
	m_color = CColor();
	m_tint = CColor();

	//m_base.Set(normal, point)
	// nullPtr m_base.Set(CVector3D(0.f, 1.f, 0.f), CVector3D(0.f, g_Renderer.GetWaterManager()->m_WaterHeight, 0.f));
	//m_base
}

Water::~Water() {}

float Water::GetMaxWaterHeight() { return m_waterModel.m_maxHeight; }
