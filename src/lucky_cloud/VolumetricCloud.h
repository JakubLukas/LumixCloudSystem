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


//#include <d3d9.h>
//#include <d3dx9.h>

#include "engine/vec.h"


#include "Simulation.h"
//#include "CloudShader.h"
#include "CloudParticle.h"

#define SAMPLE_LENGTH			0.5f 
#define OPTICAL_LENGTH_FACTOR	1.5f // determine the white and black contrast.more big more contrast.
#define SCATTER_FACTOR			0.6f // According to the scattering model used, to have the intensity of scattering color less than the incident color,
                                     // the maximum of SCATTER_FACTOR should be 2/3.
#define MIN_DENSITY				0.05f

struct Environment
{
	Lumix::Vec3			vWindVelocity;
	//Lumix::Vec3			vSunlightDir;
	//Lumix::Vec3			cSunColor;
    //float               fSunColorIntensity;
};

struct CloudProperties
{
    float		fLength;
    float		fWidth;
    float		fHigh;
    float		fCellSize;
    float		fEvolvingSpeed;
	Lumix::Vec3 vCloudPos;
    //WCHAR		szTextureFile[MAX_PATH];
};


class CVolumetricCloud
{
public:
    float				m_fLength, m_fWidth, m_fHigh;// actual size of cloud in world space
    float				m_fCellSize; // size of cell in world space.
    int 				m_iLength, m_iWidth, m_iHigh;
	Lumix::Vec3			m_vCloudPos; // the coordinates of the left-down corner 
	Lumix::Vec3			m_vWindVelocity;
	//Lumix::Vec3			m_vViewpoint;
	//Lumix::Vec3			m_vSunlightDir;
	//Lumix::Vec3			m_cSunColor;
    //WCHAR				m_szTextureFile[MAX_PATH];
    //LPDIRECT3DTEXTURE9	m_pTexture; // the cloud texture
    double				m_fTimeA;
    float				m_fEvolvingSpeed;
    //LPDIRECT3DDEVICE9	m_pDevice;
    CParticlePool		m_ParticlePool;
    //float               m_fSunColorIntensity;
    //int                 m_iColorUpdateInterval[2];

    float			 	m_fViewDistances;
    double              m_fTime;

protected:
    CSimulationSpace	m_Simulator;
    //CCloudShader		m_CloudShader;
    
public:
    CVolumetricCloud();
    ~CVolumetricCloud();
    bool Setup( /*LPDIRECT3DDEVICE9 pDevice,*/ Environment *Env, CloudProperties *Cloud );
    void Cleanup();
    void AdvanceTime(double fTime, int interval);
    //void Render();

    bool GenerateCloudParticles();
    //void CleanupCloudParticles();
    //void UpdateCloudParticleColors();
    //void SortCloudParticles(Lumix::Vec3 &vLookDir);
    void UpdateCloudPosition(double fTime);

    void SetEvolvingSpeed(float speed);
    //void SetSunColor(float r, float g, float b,float i);
    //void SetSunColorIntensity(float i);
    void SetWindVelocity(float v);

    float	GetViewDistance(){return m_fViewDistances;};
    //void	UpdateViewDistance();

protected:
	//Lumix::Vec3	CalculateParticleIncidentColor( CloudParticle* pCloudParticle );
	//Lumix::Vec3	CalculateParticleIncidentColor_SSE( CloudParticle* pCloudParticle );
	//Lumix::Vec3	CalculateParticleScatteringColor( CloudParticle* pCloudParticle );
};
