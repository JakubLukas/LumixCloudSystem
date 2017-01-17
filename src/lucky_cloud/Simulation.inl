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


// Pack version of GetCellDensity
__m128 CSimulationSpace::GetCellDensity_SSE( __m128i i_pack, __m128i j_pack, __m128i k_pack )
{			
	// Related code in GetCellDensity:
	// int index = (i)*m_uHigh*m_uWidth+(j)*m_uWidth+(k)

	// _mm_set1_epi32(i): ret = [i, i, i, i];
	__m128i m_uHigh_pack = _mm_set1_epi32(m_iHeight);
	__m128i m_uWidth_pack = _mm_set1_epi32(m_iWidth);

	// _mm_mullo_epi32(a, b): SSE4 instruction. 
	// packed integer 32-bit multiplication with truncation of upper halves of results
	// See Intel SSE4 whitepapers for the details
	// _mm_add_ps(a, b): ret = [a0+b0, a1+b1, a2+b2, a3+b3]
	__m128i t1 =  _mm_mullo_epi32(i_pack, m_uHigh_pack);
	t1 =  _mm_mullo_epi32(t1, m_uWidth_pack);
	__m128i t2 =  _mm_mullo_epi32(j_pack, m_uWidth_pack);
	__m128i index_pack =  _mm_add_epi32(t1, t2);
	index_pack = _mm_add_epi32(index_pack, k_pack);

	// Related code in GetCellDensity:
	// if( ( index < 0 )||( index >= m_uTotalNumCells ) ) return 0;
	// return pSimulationSpace->m_pCurrentDensitySpace[index];

	// zero = [0, 0, 0, 0]
	// neg_one = [-1, -1, -1, -1]
	__m128i zero = _mm_set1_epi32(0);
	__m128i neg_one = _mm_set1_epi32(-1);

	// neg_index_pack = [-index_pack0, -index_pack1, -index_pack2, -index_pack3)
	__m128i neg_index_pack = _mm_mullo_epi32(index_pack, neg_one);
	__m128i m_uTotalNumCells_pack = _mm_set1_epi32(m_uTotalNumCells);		

	// r1 || ra || rb : (-index > 0 )||( index > m_uTotalNumCells) || (index == m_uTotalNumCells)
	
	// _mm_cmpgt_epi32:
	// ret = [
	//    (a3 > b3) ? 0xffff : 0x0
	//    (a2 > b2) ? 0xffff : 0x0
	//    (a1 > b1) ? 0xffff : 0x0
	//    (a0 > b0) ? 0xffff : 0x0
	// ]
	// _mm_cmpeq_epi32:
	// ret = [
	//    (a3 = b3) ? 0xffff : 0x0
	//    (a2 = b2) ? 0xffff : 0x0
	//    (a1 = b1) ? 0xffff : 0x0
	//    (a0 = b0) ? 0xffff : 0x0
	// ]		
	__m128i r1 = _mm_cmpgt_epi32 (neg_index_pack, zero);
	__m128i ra = _mm_cmpgt_epi32 (index_pack, m_uTotalNumCells_pack);
	__m128i rb = _mm_cmpeq_epi32 (index_pack, m_uTotalNumCells_pack);	
	// _mm_or_si128(a, b): ret = a | b
	__m128i r2 = _mm_or_si128(ra, rb);
	__m128i r = _mm_or_si128(r1, r2);
	
	int index;
	int condition;
	__m128 tmp;
	// _mm_setzero_ps: ret = [0.0, 0.0, 0.0, 0.0]
	__m128 ret = _mm_setzero_ps();	

	// define macro to calculate return value; INDEX should be immediate number
	// return 0 or pSimulationSpace->m_pCurrentDensitySpace[index] for each element of the pack
	// _mm_extract_epi32(src, ndx): SSE4 instruction, extract dword value from src accroding to ndx
	// _mm_insert_ps(a, b, ndx): SSE4 instruction, select element of b and insert into a accroding to the ndx
	// See Intel SSE4 whitepapers for the details	
#define CALC_RETURN_VALUE(INDEX) \
	condition = _mm_extract_epi32(r, INDEX);	\
	if (condition == 0 ) {	\
	index = _mm_extract_epi32(index_pack, INDEX); 	\
	tmp = _mm_load_ss(&m_pCurrentDensitySpace[index]);	\
	} else {	\
	tmp = _mm_setzero_ps();	\
	}	\
	ret = _mm_insert_ps(ret, tmp, _MM_MK_INSERTPS_NDX(0, INDEX, 0));
	
	CALC_RETURN_VALUE(0);
	CALC_RETURN_VALUE(1);
	CALC_RETURN_VALUE(2);
	CALC_RETURN_VALUE(3);

	return ret;
}

// Pack version of GetPointDensity
__m128 CSimulationSpace::GetPointDensity_SSE( __m128 vPointXPack, __m128 vPointYPack, __m128 vPointZPack)
{
	__m128i i_pack;
	__m128i j_pack;
	__m128i k_pack;
	__m128 r_pack;
	__m128 s_pack;
	__m128 t_pack;

	// Get useful variables packs from input packs
	GetVarPacks_SSE(vPointXPack, vPointYPack, vPointZPack, &i_pack, &j_pack, &k_pack, &r_pack, &s_pack, &t_pack);

	// one = [1, 1, 1, 1)
	__m128i one = _mm_set1_epi32(1);

	// Get cell density packs
	__m128 d0_pack = GetCellDensity_SSE( i_pack,  j_pack,  k_pack );	
	__m128 d1_pack = GetCellDensity_SSE( i_pack,  j_pack,  _mm_add_epi32(k_pack, one));
	__m128 d2_pack = GetCellDensity_SSE( i_pack,  _mm_add_epi32(j_pack, one), k_pack  );
	__m128 d3_pack = GetCellDensity_SSE( i_pack,  _mm_add_epi32(j_pack, one), _mm_add_epi32(k_pack, one));
	__m128 d4_pack = GetCellDensity_SSE( _mm_add_epi32(i_pack, one),j_pack,  k_pack  );
	__m128 d5_pack = GetCellDensity_SSE( _mm_add_epi32(i_pack, one),j_pack,  _mm_add_epi32(k_pack, one));
	__m128 d6_pack = GetCellDensity_SSE( _mm_add_epi32(i_pack, one),_mm_add_epi32(j_pack, one), k_pack  );
	__m128 d7_pack = GetCellDensity_SSE( _mm_add_epi32(i_pack, one),_mm_add_epi32(j_pack, one), _mm_add_epi32(k_pack, one));

	__m128 z01_pack;
	__m128 z23_pack;
	__m128 z45_pack;
	__m128 z67_pack;

	__m128 x0145_pack;
	__m128 x2367_pack;		

	// interpolate densities

	// z01 = (d1 - d0)*t + d0;	
	z01_pack = lerp_SSE (d0_pack, d1_pack, t_pack);

	// z23 = (d3 - d2)*t + d2;
	z23_pack = lerp_SSE (d2_pack, d3_pack, t_pack);

	// z45 = (d5 - d4)*t + d4;
	z45_pack = lerp_SSE (d4_pack, d5_pack, t_pack);

	// z67 = (d7 - d6)*t + d6;	
	z67_pack = lerp_SSE (d6_pack, d7_pack, t_pack);

	// x0145 = (z45 - z01)*r + z01;
	x0145_pack = lerp_SSE(z01_pack, z45_pack, r_pack);

	// x2367 = (z67 - z23)*r + z23;
	x2367_pack = lerp_SSE(z23_pack, z67_pack, r_pack);	

	// result = ((x2367 - x0145)*s + x0145);
	return lerp_SSE(x0145_pack, x2367_pack, s_pack);

}
