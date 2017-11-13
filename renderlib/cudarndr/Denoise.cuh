#include "Scene.h"

#include "Utilities.cuh"
#include "CudaUtilities.h"

#define KRNL_DENOISE_BLOCK_W		8
#define KRNL_DENOISE_BLOCK_H		8
#define KRNL_DENOISE_BLOCK_SIZE	KRNL_DENOISE_BLOCK_W * KRNL_DENOISE_BLOCK_H

DEV float lerpf(float a, float b, float c){
	return a + (b - a) * c;
}

DEV float vecLen(float4 a, float4 b)
{
    return ((b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y) + (b.z - a.z) * (b.z - a.z));
}

KERNEL void KrnlDenoise(CCudaView* pView)
{
	const int X 	= blockIdx.x * blockDim.x + threadIdx.x;
	const int Y		= blockIdx.y * blockDim.y + threadIdx.y;

	if (X >= gFilmWidth || Y >= gFilmHeight)
		return;

    const float x = (float)X + 0.5f;
    const float y = (float)Y + 0.5f;

	const float4 clr00 = tex2D(gTexRunningEstimateRgba, x, y);
	
	if (gDenoiseEnabled && gDenoiseLerpC > 0.0f && gDenoiseLerpC < 1.0f)
	{
        float			fCount		= 0;
        float			SumWeights	= 0;
        float3			clr			= { 0, 0, 0 };
        		
        for (float i = -gDenoiseWindowRadius; i <= gDenoiseWindowRadius; i++)
		{
            for (float j = -gDenoiseWindowRadius; j <= gDenoiseWindowRadius; j++)
            {
                const float4 clrIJ = tex2D(gTexRunningEstimateRgba, x + j, y + i);
                const float distanceIJ = vecLen(clr00, clrIJ);

                const float weightIJ = __expf(-(distanceIJ * gDenoiseNoise + (i * i + j * j) * gDenoiseInvWindowArea));

                clr.x += clrIJ.x * weightIJ;
                clr.y += clrIJ.y * weightIJ;
                clr.z += clrIJ.z * weightIJ;

                SumWeights += weightIJ;

                fCount += (weightIJ > gDenoiseWeightThreshold) ? gDenoiseInvWindowArea : 0;
            }
		}
		
		SumWeights = 1.0f / SumWeights;

		clr.x *= SumWeights;
		clr.y *= SumWeights;
		clr.z *= SumWeights;

		const float LerpQ = (fCount > gDenoiseLerpThreshold) ? gDenoiseLerpC : 1.0f - gDenoiseLerpC;

		clr.x = lerpf(clr.x, clr00.x, LerpQ);
		clr.y = lerpf(clr.y, clr00.y, LerpQ);
		clr.z = lerpf(clr.z, clr00.z, LerpQ);

		CColorRgbLdr cx(255 * clr.x, 255 * clr.y, 255 * clr.z);
		pView->m_DisplayEstimateRgbLdr.Set(cx, X, Y);
    }
	else
	{
		const CColorRgbaLdr RGBA = pView->m_EstimateRgbaLdr.Get(X, Y);
		CColorRgbLdr cx(RGBA.r, RGBA.g, RGBA.b);
		pView->m_DisplayEstimateRgbLdr.Set(cx, X, Y);
	}
}

void Denoise(CScene* pScene, CScene* pDevScene, CCudaView* pDevView)
{
	const dim3 KernelBlock(KRNL_DENOISE_BLOCK_W, KRNL_DENOISE_BLOCK_H);
	const dim3 KernelGrid((int)ceilf((float)pScene->m_Camera.m_Film.m_Resolution.GetResX() / (float)KernelBlock.x), (int)ceilf((float)pScene->m_Camera.m_Film.m_Resolution.GetResY() / (float)KernelBlock.y));

	KrnlDenoise<<<KernelGrid, KernelBlock>>>(pDevView);
	cudaDeviceSynchronize();
	HandleCudaKernelError(cudaGetLastError(), "Noise Reduction");
}
