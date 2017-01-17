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



#include "VolumetricCloud.h"
#include "Simulation.h"
//#include "Camera.h"

using namespace std;

//extern Camera*  gCamera;
//extern D3DXVECTOR3*	g_pvViewpoint;
static const bool g_SSE4On = true;

const int iMaxColorInterval = 1000;

inline float OpticalLength(float fLineDensity)
{
    return OPTICAL_LENGTH_FACTOR * fLineDensity;
}

float Phase(Lumix::Vec3 vIn, Lumix::Vec3 vOut )
{
	vIn.normalize();
	vOut.normalize();
    return (float)(( 1.0 + SQUARE( vIn.x * vOut.x + vIn.y * vOut.y + vIn.z * vOut.z) )* 3.0/4.0);
}

CVolumetricCloud::CVolumetricCloud():
    m_fLength( 0.f ),
    m_fWidth( 0.f ),
    m_fHigh( 0.f ),
    m_fCellSize( 0.f ),
    m_iLength( 0 ),
    m_iWidth( 0 ),
    m_iHigh( 0 ),
    m_fTimeA(0)
{
    //for double buffer implementation
    //m_iColorUpdateInterval[0] = 0;
    //m_iColorUpdateInterval[1] = 1; 
}

bool CVolumetricCloud::Setup( /*LPDIRECT3DDEVICE9 pDevice,*/ Environment *Env, CloudProperties *Cloud )
{
	bool result = true;
    /*HRESULT hr;
    // set the device
    if (pDevice != NULL)
        m_pDevice = pDevice;
    m_pDevice->AddRef();*/

    m_fLength = Cloud->fLength;
    m_fWidth = Cloud->fWidth;
    m_fHigh = Cloud->fHigh;
    m_fCellSize = Cloud->fCellSize;
    m_iLength = (int)( m_fLength/m_fCellSize+0.5 );
    m_iWidth = (int)( m_fWidth/m_fCellSize+0.5 );
    m_iHigh = (int)( m_fHigh/m_fCellSize+0.5 );
    m_fEvolvingSpeed = Cloud->fEvolvingSpeed;
    m_vCloudPos = Cloud->vCloudPos;
    // create texture;
    //StringCchCopy( m_szTextureFile, MAX_PATH, Cloud->szTextureFile ); 

    m_vWindVelocity = Env->vWindVelocity;

    //m_fSunColorIntensity = Env->fSunColorIntensity;
    //m_cSunColor.x = Env->cSunColor.x*m_fSunColorIntensity;
    //m_cSunColor.y = Env->cSunColor.y*m_fSunColorIntensity;
    //m_cSunColor.z = Env->cSunColor.z*m_fSunColorIntensity;
    
    //m_vSunlightDir = Env->vSunlightDir;
	//m_vSunlightDir.normalize();

    //m_vViewpoint = *g_pvViewpoint;

	result &= m_Simulator.Setup( m_iLength, m_iWidth, m_iHigh );
	result &= GenerateCloudParticles();
	//result &= m_CloudShader.Setup( this, (float)1.2*m_fCellSize, 200 );

    m_ParticlePool.m_vWindVelocity = Env->vWindVelocity;


    return true;
}


void CVolumetricCloud::Cleanup()
{
    //m_CloudShader.Cleanup();

    m_ParticlePool.Cleanup();

    m_Simulator.Cleanup();
    
    /*if( m_pDevice )
    {
        m_pDevice->Release();
        m_pDevice = NULL;
    }*/
}


CVolumetricCloud::~CVolumetricCloud()
{
}


/*D3DXCOLOR	CVolumetricCloud::CalculateParticleIncidentColor( CloudParticle* pCloudParticle )
{
    float fDensity;
    float fOpticalDepth;
    float fTransparency;

    D3DXVECTOR3 vCurPt = D3DXVECTOR3((float)pCloudParticle->m_i,(float)pCloudParticle->m_j,(float)pCloudParticle->m_k) 
                        + ( - m_vSunlightDir ) * SAMPLE_LENGTH;

    D3DXCOLOR cIncidentColor = m_cSunColor;
    float fAlpha = 1.5 * SCATTER_FACTOR;
    while( m_Simulator.IsPointInSpace(&vCurPt) )
    {
        fDensity = m_Simulator.GetPointDensity( &vCurPt );
        fOpticalDepth = OpticalLength( fDensity * SAMPLE_LENGTH ); 		
        fTransparency = (float)exp( - fOpticalDepth );
        cIncidentColor *= ( ( 1 - fAlpha ) * fTransparency + fAlpha );
        vCurPt += ( - m_vSunlightDir ) * SAMPLE_LENGTH;
    }
    
    return cIncidentColor;
}*/

inline __m128 OpticalLength_SSE(__m128 fLineDensity)
{
    //__declspec (align(16)) float f[4] = { OPTICAL_LENGTH_FACTOR, OPTICAL_LENGTH_FACTOR, OPTICAL_LENGTH_FACTOR, OPTICAL_LENGTH_FACTOR };
    __m128 factors = _mm_set1_ps(OPTICAL_LENGTH_FACTOR);	
    return _mm_mul_ps(factors, fLineDensity);
}

// Use SSE to optimize CalculateParticleIncidentColor
/*D3DXCOLOR	CVolumetricCloud::CalculateParticleIncidentColor_SSE( CloudParticle* pCloudParticle )
{	

    float fDensity;	

    __m128 fTransparency_pack;
    __declspec(align(16)) float afTransparency[4];
    __declspec(align(16)) float afOpticalDepth[4];	

    D3DXCOLOR cIncidentColor = m_cSunColor;	

    __m128 color_pack = _mm_loadu_ps(cIncidentColor);

    float fAlpha = 1.5 * SCATTER_FACTOR;
    __m128 fAlpha_pack = _mm_set1_ps(fAlpha);
    __m128 fOne_pack = _mm_set1_ps(1.0);

    __m128 fFactor_pack;	

    D3DXVECTOR3 vCurPt = D3DXVECTOR3((float)pCloudParticle->m_i,(float)pCloudParticle->m_j,(float)pCloudParticle->m_k) 
        + ( - m_vSunlightDir ) * SAMPLE_LENGTH;	

    __m128 fDensity_pack;
    __m128 fCurOpticalDepth_pack;

    int iCount = 0;

    __m128 sample_length = _mm_set1_ps(SAMPLE_LENGTH);	
    __m128 line_density;
    __m128 tmpX, tmpY, tmpZ;

    __m128	vPointXPack = _mm_setzero_ps();
    __m128	vPointYPack = _mm_setzero_ps();
    __m128	vPointZPack = _mm_setzero_ps();

    __m128 tmp_pack;

    while( m_Simulator.IsPointInSpace(&vCurPt) )
    {		
        // add to pack, 4 points per pack
        if (iCount < 4) {				

            // _mm_load_ss(float *p): ret = [*p, 0.0, 0.0, 0.0]
            tmpX = _mm_load_ss(&vCurPt.x);
            tmpY = _mm_load_ss(&vCurPt.y);
            tmpZ = _mm_load_ss(&vCurPt.z);

            // _mm_insert_ps(a, b, ndx): SSE4 instruction, select element of b and insert into a accroding to the ndx            
            // See Intel SSE4 whitepapers for the details
            // Be noticed that intrinsic parameter ndx must be an immediate value so we define macro for easy use.
            // The macro is expanded as: 
            // CALC_PACK(X, index) => vPointXPack = _mm_insert_ps(vPointXPack, tmpX, _MM_MK_INSERTPS_NDX(0, index, 0));			
            
#define CALC_PACK(which, index) vPoint##which##Pack = _mm_insert_ps(vPoint##which##Pack, tmp##which, _MM_MK_INSERTPS_NDX(0, index, 0))

            switch (iCount) {
                case 0:
                    CALC_PACK(X, 0);
                    CALC_PACK(Y, 0);
                    CALC_PACK(Z, 0);
                    break;
                case 1:
                    CALC_PACK(X, 1);
                    CALC_PACK(Y, 1);
                    CALC_PACK(Z, 1);
                    break;
                case 2:
                    CALC_PACK(X, 2);
                    CALC_PACK(Y, 2);
                    CALC_PACK(Z, 2);
                    break;
                case 3:
                    CALC_PACK(X, 3);
                    CALC_PACK(Y, 3);
                    CALC_PACK(Z, 3);
                    break;
            }

            iCount++;
            vCurPt += ( - m_vSunlightDir ) * SAMPLE_LENGTH;		

            // process pack
        } else {
            fDensity_pack = m_Simulator.GetPointDensity_SSE( vPointXPack, vPointYPack, vPointZPack);

            // aligned memory to 16 bytes for _mm_store_ps
            __declspec(align(16)) float fDensity[4];	
            _mm_store_ps(fDensity, fDensity_pack);				

            line_density = _mm_mul_ps(fDensity_pack, sample_length);

            fCurOpticalDepth_pack = OpticalLength_SSE( line_density ); 

            // store to array for exp operation
            _mm_store_ps(afOpticalDepth, fCurOpticalDepth_pack);			

            afTransparency[0] = (float)exp( - afOpticalDepth[0] );
            afTransparency[1] = (float)exp( - afOpticalDepth[1] );
            afTransparency[2] = (float)exp( - afOpticalDepth[2] );
            afTransparency[3] = (float)exp( - afOpticalDepth[3] );

            fTransparency_pack = _mm_load_ps(afTransparency);

            fFactor_pack = lerp_SSE(fAlpha_pack, fOne_pack, fTransparency_pack);			

            tmp_pack = _mm_shuffle_ps(fFactor_pack, fFactor_pack, _MM_SHUFFLE(0, 0, 0, 0));
            color_pack = _mm_mul_ps(color_pack, tmp_pack);

            tmp_pack = _mm_shuffle_ps(fFactor_pack, fFactor_pack, _MM_SHUFFLE(1, 1, 1, 1));
            color_pack = _mm_mul_ps(color_pack, tmp_pack);

            tmp_pack = _mm_shuffle_ps(fFactor_pack, fFactor_pack, _MM_SHUFFLE(2, 2, 2, 2));
            color_pack = _mm_mul_ps(color_pack, tmp_pack);

            tmp_pack = _mm_shuffle_ps(fFactor_pack, fFactor_pack, _MM_SHUFFLE(3, 3, 3, 3));
            color_pack = _mm_mul_ps(color_pack, tmp_pack);

            iCount = 0;

            vPointXPack = _mm_setzero_ps();
            vPointYPack = _mm_setzero_ps();
            vPointZPack = _mm_setzero_ps();
        }		
    }	

    _mm_storeu_ps((FLOAT *)cIncidentColor, color_pack);

    float fOpticalDepth;
    float fTransparency;

    // vCurPt goes back for extra iCount-1 points which are not packed
    vCurPt -= ( - m_vSunlightDir ) * SAMPLE_LENGTH * (float)iCount;
    for (int i=0; i< iCount; i++) 
    {
        fDensity = m_Simulator.GetPointDensity( &vCurPt );
        fOpticalDepth = OpticalLength( fDensity * SAMPLE_LENGTH ); 		

        fTransparency = (float)exp( - fOpticalDepth );
        cIncidentColor *= ( ( 1 - fAlpha ) * fTransparency + fAlpha );
        vCurPt += ( - m_vSunlightDir ) * SAMPLE_LENGTH;
    }

    return cIncidentColor;
}*/

/*D3DXCOLOR	CVolumetricCloud::CalculateParticleScatteringColor( CloudParticle* pCloudParticle )
{
    UINT i = pCloudParticle->m_i;
    UINT j = pCloudParticle->m_j;
    UINT k = pCloudParticle->m_k;
    D3DXCOLOR cScatteringColor;

    float fDensity = m_Simulator.GetCellDensity( i,j,k );

    if( fDensity < MIN_DENSITY )
    {
        pCloudParticle->m_bVisible = false;
        cScatteringColor = D3DXCOLOR(0,0,0,0);
        pCloudParticle->m_cScatteringColor = cScatteringColor;
        return cScatteringColor;
    }
    else
    {
        pCloudParticle->m_bVisible = true;

        D3DXCOLOR cIncidentColor;

        if (g_SSE4On == true)
          // use SSE to optimize CalculateParticleIncidentColor
            cIncidentColor = CalculateParticleIncidentColor_SSE( pCloudParticle );
        else
            cIncidentColor = CalculateParticleIncidentColor( pCloudParticle );

        float fOpticalDepth = OpticalLength( fDensity );
        float fTransparency = (float)exp( - fOpticalDepth );

        D3DXVECTOR3 vViewDir = *(pCloudParticle->GetPosition()) - m_vViewpoint;
        cScatteringColor = cIncidentColor
                           * ( (float)(( 1 - fTransparency ) * Phase( m_vSunlightDir, vViewDir) * SCATTER_FACTOR) );

        cScatteringColor.a = 1 - fTransparency; //particle's alpha value = 1 - m_fAttenuation

        pCloudParticle->m_cScatteringColor = cScatteringColor;

        return cScatteringColor;
    }
}*/


bool CVolumetricCloud::GenerateCloudParticles()
{
    bool ret;
    ret = m_ParticlePool.Setup( this, m_Simulator.GetNumCellInVolume() );
    if( !ret ) return false;
    for( int i = 0; i < m_iLength; i++ )
    {
        for( int j = 0; j < m_iHigh; j++)
        {
            for( int k = 0; k < m_iWidth; k++ )
            {
                if( m_Simulator.IsCellInVolume(i,j,k) )
                {
                    ret = m_ParticlePool.AddParticle(i,j,k);
                    if( !ret ) return false;
                }
            }
        }
    }
    return true;
}

/*void CVolumetricCloud::UpdateCloudParticleColors()
{
    CParticleEnumerator Enumerator( &m_ParticlePool );
    CloudParticle *pCurParticle = Enumerator.NextParticle();
    while( pCurParticle )
    {
        CalculateParticleScatteringColor( pCurParticle );
        pCurParticle = Enumerator.NextParticle();
    }
}*/

/*void CVolumetricCloud::SortCloudParticles(D3DXVECTOR3 &vLookDir)
{
    m_ParticlePool.SortbyViewDistances(vLookDir);
}*/


void CVolumetricCloud::UpdateCloudPosition(double fTime)
{
    if (m_fTime == 0.0)  m_fTime = fTime; //first frame

    Lumix::Vec3 vDisplacement = m_vWindVelocity * (float)(fTime-m_fTime);
    m_vCloudPos += vDisplacement;
    m_ParticlePool.UpdateParticlePositions( fTime );

    //Change the cloud pos to let it loop moving
    if (m_vCloudPos.z < -1000)
		m_vCloudPos.z += 1500;

    m_fTime = fTime;
}

/*void CVolumetricCloud::Render()
{
    m_CloudShader.Render();
}*/


void CVolumetricCloud::AdvanceTime(double fTime, int interval)
{
    //Double buffer: switch buffer index between 0 and 1. 
    m_ParticlePool.m_iCurrentBuffer = 1 - m_ParticlePool.m_iCurrentBuffer;

    //m_vViewpoint = gCamera->pos();

    // change cloud density
    if (m_fEvolvingSpeed != 1.0) // if not pause evolving
    {
        float fAlpha = (float)( fTime - m_fTimeA ) / m_fEvolvingSpeed;
        m_Simulator.InterpolateDensitySpace( fAlpha);
        if( ( fTime < m_fTimeA )|| ( fTime > (m_fTimeA + m_fEvolvingSpeed)))
        {
            m_fTimeA = fTime;
        }
    }

    UpdateCloudPosition( fTime );
    //SortCloudParticles((D3DXVECTOR3 &)(gCamera->look()));    

    //if ((m_iColorUpdateInterval[m_ParticlePool.m_iCurrentBuffer]%interval) == 0)
        //UpdateCloudParticleColors();
    //m_iColorUpdateInterval[m_ParticlePool.m_iCurrentBuffer]++;
    //if (m_iColorUpdateInterval[m_ParticlePool.m_iCurrentBuffer] == iMaxColorInterval) 
        //m_iColorUpdateInterval[m_ParticlePool.m_iCurrentBuffer] = 0;
}

void CVolumetricCloud::SetEvolvingSpeed(float Speed)
{
    m_fEvolvingSpeed = Speed;
}

/*void CVolumetricCloud::SetSunColor(float r, float g, float b, float i)
{
    m_cSunColor.r = r * i;
    m_cSunColor.g = g * i;
    m_cSunColor.b = b * i;
}*/

void CVolumetricCloud::SetWindVelocity(float v)
{
    m_vWindVelocity.z = v;
    m_ParticlePool.m_vWindVelocity.z = v;
}

/*void CVolumetricCloud::UpdateViewDistance()
{
    Lumix::Vec3 relation_pos = m_vCloudPos+ Lumix::Vec3((float)(m_fLength/2.0),(float)(m_fHigh/2.0),(float)(m_fWidth/2.0)) - m_vViewpoint;
    m_fViewDistances = sqrt( D3DXVec3Dot(&relation_pos, &relation_pos) );
}*/
