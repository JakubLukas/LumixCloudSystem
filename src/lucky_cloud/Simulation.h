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

#pragma once

#include "engine/lumix.h"
#include "engine/vec.h"


// SSE2 and SSE4.1
#include <emmintrin.h>
#include <smmintrin.h>

#include "Utils.inl"

//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
#define SUPPLY_INTERVAL 5	//the number of frames passed supply vapoar 
#define EXTINCT_FACTOR 0.1	
#define fRANDOM			(((float)rand())/RAND_MAX) //this is not thread safe and will not be as random as it could be
#define INDEX(i,j,k)	((i)*m_iHeight*m_iWidth+(j)*m_iWidth+(k))
#define SQUARE(r)		((r)*(r))


class CSimulationSpace
{

public:
	int 			m_iLength;
	int				m_iWidth;
	int				m_iHeight;
	unsigned int	m_uNumCellInVolume;
	unsigned int	m_uTotalNumCells;

protected:
	unsigned char*	m_pShapeSpace;
	float*			m_pHumProbSpace;
	float*			m_pExtProbSpace;
	float*			m_pActProbSpace;

	unsigned int	m_uLastPhaseIndex;
	unsigned int	m_uNextPhaseIndex;
	unsigned char*	m_apHumSpace[2];
	unsigned char*	m_apCldSpace[2];
	unsigned char*	m_apActSpace[2];
	unsigned char*	m_pActFactorSpace;

	float*			m_apDensitySpace[2];
	float*			m_pMidDensitySpace;
	float*			m_pCurrentDensitySpace;
	int				m_iElapsedSteps;

public:
	CSimulationSpace();
	virtual ~CSimulationSpace();
	bool	Setup(unsigned int uLength, unsigned int uWidth, unsigned int uHigh);
	void	Cleanup();
	void	ShapeVolume();
	void	InitProbSpace();

	void	GrowCloud(unsigned int uPhaseIndex);
	void	ExtinctCloud(unsigned int uPhaseIndex);
	void	SupplyVapor(unsigned int uPhaseIndex);

	void	InterpolateDensitySpace(float fAlpha);
	float	GetCellDensity(int i, int j, int k);
	float	GetPointDensity(Lumix::Vec3 *pvPoint);

	bool	IsCellInVolume(int i, int j, int k, int* pIndex = nullptr);
	bool	IsCellInSpace(int i, int j, int k);
	void	InitCellInVolume(int i, int j, int k, float fProbSeed);
	bool	IsPointInSpace(Lumix::Vec3 *pvPoint);
	unsigned int	GetNumCellInVolume() { return m_uNumCellInVolume; }

	#include "Simulation.inl"

protected:
	void			SetByteCell(unsigned char* pByteSpace, int i, int j, int k, unsigned char value);
	unsigned char	GetByteCell(unsigned char* pByteSpace, int i, int j, int k);
	bool			NewByteSpace(int size, unsigned char **ppBitSpace);
	bool			NewFloatSpace(int size, float **ppFloatSpace);
	void			UpdateActFactorSpace(unsigned int uPhaseIndex);
	void			CelluarAutomate(unsigned int uPhaseIndex);
	void			CalculateDensity(unsigned int uPhaseIndex);
};
