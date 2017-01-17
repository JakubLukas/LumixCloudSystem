//-------------------------------------------------------------------------------------
//
// Copyright 2009 Intel Corporation
// All Rights Reserved
//
// Permission is granted to use, copy, distribute and prepare derivative works of this
// software for any purpose and without fee, provided, that the above copyright notice
// and this statement appear in all copies.  Intel makes no representations about the
// suitability of this software for any purpose.  THIS SOFTWARE IS PROVIDED "AS IS."
// INTEL SPECIFICALLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, AND ALL LIABILITY,
// INCLUDING CONSEQUENTIAL AND OTHER INDIRECT DAMAGES, FOR THE USE OF THIS SOFTWARE,
// INCLUDING LIABILITY FOR INFRINGEMENT OF ANY PROPRIETARY RIGHTS, AND INCLUDING THE
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  Intel does not
// assume any responsibility for any errors which may appear in this software nor any
// responsibility to update it.
//
//--------------------------------------------------------------------------------------
// DXUT was adapted from the Microsoft DirectX SDK(November 2008)
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// The skybox is free downloaded from :
//   http://en.pudn.com/downloads119/sourcecode/others/detail508412_en.html
//-------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------
#pragma once


// Get some useful variable packs from point position packs
inline void GetVarPacks_SSE(__m128 vPointXPack, __m128 vPointYPack, __m128 vPointZPack, __m128i* i_pack, __m128i* j_pack, __m128i* k_pack, 
							__m128* r_pack, __m128* s_pack, __m128* t_pack)
{	
	// _mm_cvttps_epi32(a): ret = [(int)a0, (int)a1, (int)a2, (int)a3]
	*i_pack =  _mm_cvttps_epi32(vPointXPack);
	*j_pack =  _mm_cvttps_epi32(vPointYPack);
	*k_pack =  _mm_cvttps_epi32(vPointZPack);
	
	// _mm_cvtepi32_ps(a): ret = [(float)a0, (float)a1, (float)a2, (float)a3]
	// _mm_sub_ps(a, b): ret = [a0-b0, a1-b1, a2-b2, a3-b3]
	__m128 tmp;

	tmp = _mm_cvtepi32_ps(*i_pack);

	*r_pack = _mm_sub_ps( vPointXPack, tmp);

	tmp = _mm_cvtepi32_ps(*j_pack);

	*s_pack = _mm_sub_ps( vPointYPack, tmp);

	tmp = _mm_cvtepi32_ps(*k_pack);

	*t_pack = _mm_sub_ps( vPointZPack, tmp);
}

// Pack version of linear interpolation 
inline __m128 lerp_SSE(__m128 a_pack, __m128 b_pack, __m128 t_pack)
{		
	__m128 tmp;

	// ret = (b - a)*t + a;
	// _mm_sub_ps(a, b): ret = [a0-b0, a1-b1, a2-b2, a3-b3]
	// _mm_mul_ps(a, b): ret = [a0*b0, a1*b1, a2*b2, a3*b3]
	// _mm_add_ps(a, b): ret = [a0+b0, a1+b1, a2+b2, a3+b3]
	tmp = _mm_sub_ps(b_pack, a_pack);
	tmp = _mm_mul_ps(tmp, t_pack);
	return _mm_add_ps(tmp, a_pack);
}