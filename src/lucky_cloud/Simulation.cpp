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


//#include "dxut.h"
#include "Simulation.h"
#include <cstring>
#include <cmath>

using namespace std;


CSimulationSpace::CSimulationSpace()
{
	memset( this, 0, sizeof( CSimulationSpace ) );
}


CSimulationSpace::~CSimulationSpace()
{
	Cleanup();
}


bool CSimulationSpace::Setup(unsigned int uLength, unsigned int uWidth, unsigned int uHigh)
{
	m_iLength = uLength;
	m_iWidth = uWidth; 
	m_iHeight = uHigh;
	int size = uLength * uWidth * uHigh;
	m_uLastPhaseIndex = 0;
	m_uNextPhaseIndex = 1;
	m_iElapsedSteps = 0;

	if(		NewByteSpace( size, &m_apHumSpace[0] )
		&& 	NewByteSpace( size, &m_apHumSpace[1] )
		&& 	NewByteSpace( size, &m_apCldSpace[0] )
		&& 	NewByteSpace( size, &m_apCldSpace[1] )
		&& 	NewByteSpace( size, &m_apActSpace[0] )
		&& 	NewByteSpace( size, &m_apActSpace[1] )
		&&	NewByteSpace( size, &m_pActFactorSpace )
		&& 	NewFloatSpace( size, &m_apDensitySpace[0] )
		&& 	NewFloatSpace( size, &m_apDensitySpace[1] )

		&& 	NewByteSpace( size, &m_pShapeSpace )
		&& 	NewFloatSpace( size, &m_pHumProbSpace )
		&& 	NewFloatSpace( size, &m_pExtProbSpace )
		&& 	NewFloatSpace( size, &m_pActProbSpace )
		&& 	NewFloatSpace( size, &m_pMidDensitySpace )
		)
	{
		m_uTotalNumCells = size;
		InitProbSpace();
		ShapeVolume();
		CelluarAutomate( m_uNextPhaseIndex );
		CalculateDensity( m_uNextPhaseIndex );
		return true;
	}
	else
		return false;
}

bool CSimulationSpace::NewByteSpace( int size, unsigned char **ppBitSpace )
{
	*ppBitSpace = new unsigned char[size];
	if( *ppBitSpace )
	{
		memset( *ppBitSpace, 0, size*sizeof(unsigned char) );
		return true;
	}
	else
		return false;
}

bool CSimulationSpace::NewFloatSpace(int size, float **ppFloatSpace)
{
	*ppFloatSpace = new float[size];
	if( *ppFloatSpace )
	{
		memset( *ppFloatSpace, 0, size*sizeof(float) );
		return true;
	}
	else
		return false;
}


void CSimulationSpace::Cleanup()
{
	delete[] m_pShapeSpace;
	delete[] m_pHumProbSpace;
	delete[] m_pExtProbSpace;
	delete[] m_pActProbSpace;
	delete[] m_apHumSpace[0];
	delete[] m_apHumSpace[1];
	delete[] m_apCldSpace[0];
	delete[] m_apCldSpace[1];
	delete[] m_apActSpace[0];
	delete[] m_apActSpace[1];
	delete[] m_pActFactorSpace;
	delete[] m_apDensitySpace[0];
	delete[] m_apDensitySpace[1];
	delete[] m_pMidDensitySpace;
}


//-----------------------------------------------------------------------------
// Name: CSimulationSpace::InterpolateDensitySpace( float fAlpha )
// Desc: calculate the current density space by interpolating the density spaces of the last phase and the next phase
//-----------------------------------------------------------------------------
void CSimulationSpace::InterpolateDensitySpace( float fAlpha)
{
	if( fAlpha <= 0.0 ) // fAlpha is the interpolation factor
	{
		m_pCurrentDensitySpace = m_apDensitySpace[m_uLastPhaseIndex];
	}
	else if( fAlpha >= 1.0 ) 
	{       
		// exchange the indexes of the last phase and the next phase;
		m_uLastPhaseIndex = m_uNextPhaseIndex;
		m_uNextPhaseIndex = !m_uLastPhaseIndex;

		// point current DensitySpace to the DensitySpace of last phase
		m_pCurrentDensitySpace = m_apDensitySpace[m_uLastPhaseIndex];

		// Create the next DensitySpace of the next phase
		CelluarAutomate( m_uNextPhaseIndex );
        CalculateDensity( m_uNextPhaseIndex );
	}
 	else
 	{
 		int index;
 
 		for( int i = 0; i < m_iLength; i++ )
 		{
 			for( int j = 0; j < m_iHeight; j++)
 			{
 				for( int k = 0; k < m_iWidth; k++ )
 				{
 					if( ! IsCellInVolume(i,j,k,&index) ) continue;
 					m_pMidDensitySpace[index] = (float)(( 1.0 - fAlpha ) * m_apDensitySpace[m_uLastPhaseIndex][index] 
 												+ fAlpha * m_apDensitySpace[m_uNextPhaseIndex][index]);
 				}
 			}
 		}
 
 		m_pCurrentDensitySpace = m_pMidDensitySpace;	
 	}
}


float CSimulationSpace::GetPointDensity(Lumix::Vec3 *pvPoint )
{
	int i = (int)pvPoint->x;
	int j = (int)pvPoint->y;
	int k = (int)pvPoint->z;
	float r = pvPoint->x - i;
	float s = pvPoint->y - j;
	float uPhaseIndex = pvPoint->z - k;
	
	// get the densities of 8 points around the point.
	float d0 = GetCellDensity( i,  j,  k  );
	float d1 = GetCellDensity( i,  j,  k+1);
	float d2 = GetCellDensity( i,  j+1,k  );
	float d3 = GetCellDensity( i,  j+1,k+1);
	float d4 = GetCellDensity( i+1,j,  k  );
	float d5 = GetCellDensity( i+1,j,  k+1);
	float d6 = GetCellDensity( i+1,j+1,k  );
	float d7 = GetCellDensity( i+1,j+1,k+1);

	// interpolate densities
	float z01 = (d1 - d0)*uPhaseIndex + d0;
	float z23 = (d3 - d2)*uPhaseIndex + d2;
	float z45 = (d5 - d4)*uPhaseIndex + d4;
	float z67 = (d7 - d6)*uPhaseIndex + d6;
	float x0145 = (z45 - z01)*r + z01;
	float x2367 = (z67 - z23)*r + z23;
	float result = ((x2367 - x0145)*s + x0145);
	return result;
}


bool CSimulationSpace::IsPointInSpace(Lumix::Vec3 *pvPoint )
{
	if( ( pvPoint->x < 0 )|| ( pvPoint->x >= m_iLength )
		|| ( pvPoint->y < 0 ) || ( pvPoint->y >= m_iHeight )
		|| ( pvPoint->z < 0 ) || ( pvPoint->z >= m_iWidth ) )
		return false;
	else
		return true;
}

bool CSimulationSpace::IsCellInSpace(int i, int j, int k )
{
	if( ( i < 0 )|| ( i >= m_iLength )
		|| ( j < 0 ) || ( j >= m_iHeight )
		|| ( k < 0 ) || ( k >= m_iWidth ) )
		return false;
	else
		return true;
}


bool CSimulationSpace::IsCellInVolume(int i, int j, int k, int* pIndex )
{
	if( !IsCellInSpace(i,j,k) )
		return false;
	else
	{
		int index = INDEX(i,j,k);
		if( m_pShapeSpace[index] )
		{
			if( pIndex )
				*pIndex = index;
			return true;
		}
		else		
			return false;
	}
}


float CSimulationSpace::GetCellDensity( int i, int j, int k )
{
	int index = INDEX( i, j, k );
	if( ( index < 0 )||( index >= (int)m_uTotalNumCells ) ) return 0;
	return m_pCurrentDensitySpace[index];
}


//-----------------------------------------------------------------------------
// Name: CSimulationSpace::CalculateDensity
// Desc: calculate every cell's density in the simluation space.
//		The density distribution of clouds in the real world is continuous from 0 to 1. 
//		The distribution obtained from the simulation,however, has only two values, that is, 0 or 1. 
//		Therefore, the function calculates continuous distribution by smoothing the binary distribution, or two-valued distribution of CldSpace
//-----------------------------------------------------------------------------
void CSimulationSpace::CalculateDensity(unsigned int uPhaseIndex )
{
	int	index;
	for( int i = 0; i < m_iLength; i++ )
	{
		for( int j = 0; j < m_iHeight; j++)
		{
			for( int k = 0; k < m_iWidth; k++ )
			{
				if( !IsCellInVolume(i,j,k,&index) ) continue;
				m_apDensitySpace[uPhaseIndex][index] = 0;

				// accumulate the binary cloud values of the cells surrouding the current cell(i,j,k) and itself.
				for(int p = i-1; p<=i+1; p++)
					for(int q = j-1; q<=j+1; q++)
						for(int r = k-1; r<=k+1; r++)
								m_apDensitySpace[uPhaseIndex][index] += GetByteCell( m_apCldSpace[uPhaseIndex], p, q, r);
				m_apDensitySpace[uPhaseIndex][index] /= 27.0f; // 27 is the number of all iterations of above nesting loop.
			}
		}
	}
}


//-----------------------------------------------------------------------------
// Name: CSimulationSpace::CelluarAutomate(UINT uPhaseIndex)
// Desc: update the simulation space.
//-----------------------------------------------------------------------------
void CSimulationSpace::CelluarAutomate(unsigned int uPhaseIndex)
{
	ExtinctCloud(!uPhaseIndex);

	GrowCloud(uPhaseIndex);

	if( m_iElapsedSteps == 0 )
	{
		SupplyVapor(uPhaseIndex);
		m_iElapsedSteps = SUPPLY_INTERVAL;
	}
	else
		m_iElapsedSteps--;
}


//-----------------------------------------------------------------------------
// Name: CSimulationSpace::GrowCloud(UINT uPhaseIndex)
// Desc: Use Celluar Automation rules to envolve cloud
//-----------------------------------------------------------------------------
void CSimulationSpace::GrowCloud(unsigned int uPhaseIndex)
{
	int index;

	UpdateActFactorSpace(!uPhaseIndex);

	for( int i = 0; i < m_iLength; i++ )
	{
		for( int j = 0; j < m_iHeight; j++)
		{
			for( int k = 0; k < m_iWidth; k++ )
			{
				if( !IsCellInVolume(i,j,k,&index) ) continue;
				m_apActSpace[uPhaseIndex][index] = ( ! m_apActSpace[!uPhaseIndex][index] ) && ( m_apHumSpace[!uPhaseIndex][index] ) && ( m_pActFactorSpace[index] );
				m_apHumSpace[uPhaseIndex][index] = m_apHumSpace[!uPhaseIndex][index] && ( ! m_apActSpace[!uPhaseIndex][index] );
				m_apCldSpace[uPhaseIndex][index] = m_apCldSpace[!uPhaseIndex][index] || m_apActSpace[!uPhaseIndex][index];
			}
		}
	}
}


void CSimulationSpace::ExtinctCloud(unsigned int uPhaseIndex )
{
	int index;

	for( int i = 0; i < m_iLength; i++ )
	{
		for( int j = 0; j < m_iHeight; j++)
		{
			for( int k = 0; k < m_iWidth; k++ )
			{
				if( !IsCellInVolume(i,j,k,&index) ) continue;
				m_apCldSpace[uPhaseIndex][index] = m_apCldSpace[uPhaseIndex][index] && ( fRANDOM > m_pExtProbSpace[index] );
			}
		}
	}
}

void CSimulationSpace::SupplyVapor(unsigned int uPhaseIndex )
{
	int index;

	for( int i = 0; i < m_iLength; i++ )
	{
		for( int j = 0; j < m_iHeight; j++)
		{
			for( int k = 0; k < m_iWidth; k++ )
			{
				if( !IsCellInVolume(i,j,k,&index) ) continue;
				m_apHumSpace[uPhaseIndex][index] = m_apHumSpace[uPhaseIndex][index] || ( fRANDOM < m_pHumProbSpace[index] );
				m_apActSpace[uPhaseIndex][index] = m_apHumSpace[uPhaseIndex][index] && ( m_apActSpace[uPhaseIndex][index] || ( fRANDOM < m_pActProbSpace[index] ) );
				
			}
		}
	}
}

void CSimulationSpace::UpdateActFactorSpace(unsigned int uPhaseIndex)
{
	for( int i = 0; i < m_iLength; i++ )
	{
		for( int j = 0; j < m_iHeight; j++)
		{
			for( int k = 0; k < m_iWidth; k++ )
			{
				if( !IsCellInVolume(i,j,k) ) continue;
				unsigned char FAct =   GetByteCell( m_apActSpace[uPhaseIndex],i+1,j,k )	
							||GetByteCell( m_apActSpace[uPhaseIndex],i,j+1,k )
							||GetByteCell( m_apActSpace[uPhaseIndex],i,j,k+1 )
							||GetByteCell( m_apActSpace[uPhaseIndex],i-1,j,k )
							||GetByteCell( m_apActSpace[uPhaseIndex],i,j-1,k )
							||GetByteCell( m_apActSpace[uPhaseIndex],i,j,k-1 )
							||GetByteCell( m_apActSpace[uPhaseIndex],i-2,j,k )
							||GetByteCell( m_apActSpace[uPhaseIndex],i+2,j,k )
							||GetByteCell( m_apActSpace[uPhaseIndex],i,j-2,k )
							||GetByteCell( m_apActSpace[uPhaseIndex],i,j+2,k )
							||GetByteCell( m_apActSpace[uPhaseIndex],i,j,k-2 );
				
				SetByteCell( m_pActFactorSpace,i,j,k,FAct);
			}
		}
	}
}


void CSimulationSpace::SetByteCell(unsigned char* pByteSpace, int i, int j, int k, unsigned char value )
{
	if( !IsCellInSpace( i,j,k ) ) return;
	pByteSpace[ INDEX(i,j,k)] = value;
}


unsigned char CSimulationSpace::GetByteCell(unsigned char* pByteSpace, int i, int j, int k )
{
	if( !IsCellInSpace( i,j,k ) ) return 0;
	return pByteSpace[INDEX( i, j, k )];
}


//-----------------------------------------------------------------------------
// Name: CSimulationSpace::ShapeVolume()
// Desc: build the ellipsoid volume of cloud
//-----------------------------------------------------------------------------
void CSimulationSpace::ShapeVolume()
{
	double cenX = m_iLength/2.0;
	double cenY = m_iHeight/2.0;
	double cenZ = m_iWidth/2.0;
	double distance;

	m_uNumCellInVolume = 0;
	float fProbSeed;
	for( int i = 0; i < m_iLength; i++ )
	{
		for( int j = 0; j < m_iHeight; j++)
		{
			for( int k = 0; k < m_iWidth; k++ )
			{
					distance = SQUARE(i-cenX) / SQUARE(cenX) 
							 + SQUARE(j-cenY) / SQUARE(cenY) 
							 + SQUARE(k-cenZ) / SQUARE(cenZ);
					if( distance < 1.0 )
					{
						fProbSeed = (float)exp( - distance );
						InitCellInVolume( i, j, k, fProbSeed );
					}
			}
		}
	}
}


void CSimulationSpace::InitProbSpace()
{
	int fHigh = m_iHeight-1;
	for( int i = 0; i < m_iLength; i++ )
	{
		for( int j = 0; j < m_iHeight; j++)
		{
			for( int k = 0; k < m_iWidth; k++ )
			{
				m_pExtProbSpace[INDEX(i,j,k)] = (float)exp( -( fHigh-j )*EXTINCT_FACTOR );
			}
		}
	}
}


//-----------------------------------------------------------------------------
// Name: CSimulationSpace::InitCellInVolume( int i, int j, int k, float fProbSeed )
// Desc: set the probablities of every cell.
//-----------------------------------------------------------------------------
void CSimulationSpace::InitCellInVolume( int i, int j, int k, float fProbSeed )
{
	if( !IsCellInSpace( i,j,k ) ) return;
	int index = INDEX(i,j,k);
	if( m_pShapeSpace[index] == false )
	{
		m_pShapeSpace[index] = true;
		m_uNumCellInVolume++;
	}
	float fCurExtProb =  (float)0.2 * ( 1 - fProbSeed );
	float fCurHumProb =  (float)0.1 * fProbSeed;
	float fCurActProb =	 (float)0.001 * fProbSeed;

	m_pExtProbSpace[index] *= fCurExtProb;
	m_pHumProbSpace[index] = m_pHumProbSpace[index]*(1-fCurHumProb)+fCurHumProb;
	m_pActProbSpace[index] = m_pActProbSpace[index]*(1-fCurActProb)+fCurActProb;
}

