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
