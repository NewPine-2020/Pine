/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#include <string.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>  // size_t
#include "type.h"
#include "yc11xx.h"
#include "nr_aec_common.h"
#define DR_MP3_IMPLEMENTATION

#define DR_WAV_IMPLEMENTATION



#define WEBRTC_SPL_LSHIFT_W32(x, c)     ((x) << (c))
#define WEBRTC_SPL_RSHIFT_W16(x, c)     ((x) >> (c))
#define WEBRTC_SPL_RSHIFT_W32(x, c)     ((x) >> (c))
#define WEBRTC_SPL_LSHIFT_W16(x, c)     ((x) << (c))

#define WEBRTC_SPL_RSHIFT_U16(x, c)     ((uint16_t)(x) >> (c))
#define WEBRTC_SPL_LSHIFT_U16(x, c)     ((uint16_t)(x) << (c))
#define WEBRTC_SPL_RSHIFT_U32(x, c)     ((uint32_t)(x) >> (c))
#define WEBRTC_SPL_LSHIFT_U32(x, c)     ((uint32_t)(x) << (c))

#define WEBRTC_SPL_MUL(a, b) \
    ((int32_t) ((int32_t)(a) * (int32_t)(b)))

#define WEBRTC_SPL_ABS_W32(a) \
    (((int32_t)a >= 0) ? ((int32_t)a) : -((int32_t)a))

#define WEBRTC_SPL_UMUL(a, b) \
    ((uint32_t) ((uint32_t)(a) * (uint32_t)(b)))

#define WEBRTC_SPL_UMUL_16_16(a, b) \
    ((uint32_t) (uint16_t)(a) * (uint16_t)(b))

#define WEBRTC_SPL_MUL_16_16_RSFT_WITH_ROUND(a, b, c) \
    ((WEBRTC_SPL_MUL_16_16(a, b) + ((int32_t) \
                                  (((int32_t)1) << ((c) - 1)))) >> (c))

#define WEBRTC_SPL_MUL_16_16(a, b) \
    ((int32_t) (((int16_t)(a)) * ((int16_t)(b))))

#define WEBRTC_SPL_MUL_16_U16(a, b) \
    ((int32_t)(int16_t)(a) * (uint16_t)(b))

#define WEBRTC_SPL_UMUL_32_16(a, b) \
    ((uint32_t) ((uint32_t)(a) * (uint16_t)(b)))

#define WEBRTC_SPL_SHIFT_W16(x, c) \
    (((c) >= 0) ? ((x) << (c)) : ((x) >> (-(c))))
#define WEBRTC_SPL_SHIFT_W32(x, c) \
    (((c) >= 0) ? ((x) << (c)) : ((x) >> (-(c))))

#define WEBRTC_SPL_MUL_16_16_RSFT(a, b, c) \
    (WEBRTC_SPL_MUL_16_16(a, b) >> (c))

#define WEBRTC_SPL_ABS_W16(a) \
    (((int16_t)a >= 0) ? ((int16_t)a) : -((int16_t)a))

#define WEBRTC_SPL_MEMCPY_W16(v1, v2, length) \
  memcpy(v1, v2, (length) * sizeof(int16_t))

#define WEBRTC_SPL_ADD_SAT_W16(a, b)    WebRtcSpl_AddSatW16(a, b)

#define WEBRTC_SPL_MUL_32_16(a, b)      ((a) * (b))

#define WEBRTC_SPL_UDIV(a, b) \
    ((uint32_t) ((uint32_t)(a) / (uint32_t)(b)))

#define WEBRTC_SPL_UMUL_32_16_RSFT16(a, b) \
    ((uint32_t) ((uint32_t)(a) * (uint16_t)(b)) >> 16)

#define WEBRTC_SPL_SAT(a, b, c)         (b > a ? a : b < c ? c : b)

#define WEBRTC_SPL_MIN(A, B)        (A < B ? A : B)  // Get min value
#define WEBRTC_SPL_MAX(A, B)        (A > B ? A : B)  // Get max value


#define WEBRTC_SPL_DIV(a, b) \
    ((int32_t) ((int32_t)(a) / (int32_t)(b)))






#define WEBRTC_SPL_SQRT_ITER(N)                 \
  try1 = root + (1 << (N));                     \
  if (value >= try1 << (N))                     \
  {                                             \
    value -= try1 << (N);                       \
    root |= 2 << (N);                           \
  }

#define ANAL_BLOCKL_MAX         256 /* Max analysis block length */
#define HALF_ANAL_BLOCKL        129 /* Half max analysis block length + 1 */
#define SIMULT                  3
#define END_STARTUP_LONG        200
#define END_STARTUP_SHORT       50
#define FACTOR_Q16              2621440 /* 40 in Q16 */
#define FACTOR_Q7               5120 /* 40 in Q7 */
#define FACTOR_Q7_STARTUP       1024 /* 8 in Q7 */
#define WIDTH_Q8                3 /* 0.01 in Q8 (or 25 ) */

 /* PARAMETERS FOR NEW METHOD */
#define DD_PR_SNR_Q11           2007 /* ~= Q11(0.98) DD update of prior SNR */
#define ONE_MINUS_DD_PR_SNR_Q11 41 /* DD update of prior SNR */
#define SPECT_FLAT_TAVG_Q14     4915 /* (0.30) tavg parameter for spectral flatness measure */
#define SPECT_DIFF_TAVG_Q8      77 /* (0.30) tavg parameter for spectral flatness measure */
#define PRIOR_UPDATE_Q14        1638 /* Q14(0.1) Update parameter of prior model */
#define NOISE_UPDATE_Q8         26 /* 26 ~= Q8(0.1) Update parameter for noise */

/* Probability threshold for noise state in speech/noise likelihood. */
#define ONE_MINUS_PROB_RANGE_Q8 205 /* 205 ~= Q8(0.8) */
#define HIST_PAR_EST            100 /* Histogram size for estimation of parameters */

/* FEATURE EXTRACTION CONFIG  */
/* Bin size of histogram */
#define BIN_SIZE_LRT            10
/* Scale parameters: multiply dominant peaks of the histograms by scale factor to obtain. */
/* Thresholds for prior model */
#define FACTOR_1_LRT_DIFF       6 /* For LRT and spectral difference (5 times bigger) */
/* For spectral_flatness: used when noise is flatter than speech (10 times bigger). */
#define FACTOR_2_FLAT_Q10       922
/* Peak limit for spectral flatness (varies between 0 and 1) */
#define THRES_PEAK_FLAT         24 /* * 2 * BIN_SIZE_FLAT_FX */
/* Limit on spacing of two highest peaks in histogram: spacing determined by bin size. */
#define LIM_PEAK_SPACE_FLAT_DIFF    4 /* * 2 * BIN_SIZE_DIFF_FX */
/* Limit on relevance of second peak */
#define LIM_PEAK_WEIGHT_FLAT_DIFF   2
#define THRES_FLUCT_LRT         10240 /* = 20 * inst->modelUpdate; fluctuation limit of LRT feat. */
/* Limit on the max and min values for the feature thresholds */
#define MAX_FLAT_Q10            38912 /*  * 2 * BIN_SIZE_FLAT_FX */
#define MIN_FLAT_Q10            4096 /*  * 2 * BIN_SIZE_FLAT_FX */
#define MAX_DIFF                100 /* * 2 * BIN_SIZE_DIFF_FX */
#define MIN_DIFF                16 /* * 2 * BIN_SIZE_DIFF_FX */
/* Criteria of weight of histogram peak  to accept/reject feature */
#define THRES_WEIGHT_FLAT_DIFF  154 /*(int)(0.3*(inst->modelUpdate)) for flatness and difference */

#define STAT_UPDATES            9 /* Update every 512 = 1 << 9 block */
#define ONE_MINUS_GAMMA_PAUSE_Q8    13 /* ~= Q8(0.05) Update for conservative noise estimate */
#define GAMMA_NOISE_TRANS_AND_SPEECH_Q8 3 /* ~= Q8(0.01) Update for transition and noise region */

typedef struct NsxHandleT NsxHandle;

enum {
	kMaxFFTOrder = 9
};



typedef struct NsxInst_t_ {
	uint32_t                fs;

	const int16_t* window;
	int16_t                 analysisBuffer[ANAL_BLOCKL_MAX];
	int16_t                 synthesisBuffer[ANAL_BLOCKL_MAX];
	uint16_t                noiseSupFilter[HALF_ANAL_BLOCKL];
	uint16_t                overdrive; /* Q8 */
	uint16_t                denoiseBound; /* Q14 */
	const int16_t* factor2Table;
	int16_t                 noiseEstLogQuantile[SIMULT * HALF_ANAL_BLOCKL];
	int16_t                 noiseEstDensity[SIMULT * HALF_ANAL_BLOCKL];
	int16_t                 noiseEstCounter[SIMULT];
	int16_t                 noiseEstQuantile[HALF_ANAL_BLOCKL];

	int                     anaLen;
	int                     anaLen2;
	int                     magnLen;
	int                     aggrMode;
	int                     stages;
	int                     initFlag;
	int                     gainMap;

	int32_t                 maxLrt;
	int32_t                 minLrt;
	// Log LRT factor with time-smoothing in Q8.
	int32_t                 logLrtTimeAvgW32[HALF_ANAL_BLOCKL];
	int32_t                 featureLogLrt;
	int32_t                 thresholdLogLrt;
	int16_t                 weightLogLrt;

	uint32_t                featureSpecDiff;
	uint32_t                thresholdSpecDiff;
	int16_t                 weightSpecDiff;

	uint32_t                featureSpecFlat;
	uint32_t                thresholdSpecFlat;
	int16_t                 weightSpecFlat;

	// Conservative estimate of noise spectrum.
	int32_t                 avgMagnPause[HALF_ANAL_BLOCKL];
	uint32_t                magnEnergy;
	uint32_t                sumMagn;
	uint32_t                curAvgMagnEnergy;
	uint32_t                timeAvgMagnEnergy;
	uint32_t                timeAvgMagnEnergyTmp;

	uint32_t                whiteNoiseLevel;  // Initial noise estimate.
	// Initial magnitude spectrum estimate.
	uint32_t                initMagnEst[HALF_ANAL_BLOCKL];
	// Pink noise parameters:
	int32_t                 pinkNoiseNumerator;  // Numerator.
	int32_t                 pinkNoiseExp;  // Power of freq.
	int                     minNorm;  // Smallest normalization factor.
	int                     zeroInputSignal;  // Zero input signal flag.

	// Noise spectrum from previous frame.
	uint32_t                prevNoiseU32[HALF_ANAL_BLOCKL];
	// Magnitude spectrum from previous frame.
	uint16_t                prevMagnU16[HALF_ANAL_BLOCKL];
	// Prior speech/noise probability in Q14.
	int16_t                 priorNonSpeechProb;

	int                     blockIndex;  // Frame index counter.
	// Parameter for updating or estimating thresholds/weights for prior model.
	int                     modelUpdate;
	int                     cntThresUpdate;

	// Histograms for parameter estimation.
	int16_t                 histLrt[HIST_PAR_EST];
	int16_t                 histSpecFlat[HIST_PAR_EST];
	int16_t                 histSpecDiff[HIST_PAR_EST];

	// Quantities for high band estimate.
	int16_t                 dataBufHBFX[ANAL_BLOCKL_MAX];  // Q0

	int                     qNoise;
	int                     prevQNoise;
	int                     prevQMagn;
	int                     blockLen10ms;

	int16_t                 real[ANAL_BLOCKL_MAX];
	int16_t                 imag[ANAL_BLOCKL_MAX];
	int32_t                 energyIn;
	int                     scaleEnergyIn;
	int                     normData;

	struct RealFFT real_fft;
} NsxInst_t;

#ifdef __cplusplus
extern "C"
{
#endif

	/****************************************************************************
	 * WebRtcNsx_InitCore(...)
	 *
	 * This function initializes a noise suppression instance
	 *
	 * Input:
	 *      - inst          : Instance that should be initialized
	 *      - fs            : Sampling frequency
	 *
	 * Output:
	 *      - inst          : Initialized instance
	 *
	 * Return value         :  0 - Ok
	 *                        -1 - Error
	 */
	int32_t WebRtcNsx_InitCore(NsxInst_t* inst, uint32_t fs);

	/****************************************************************************
	 * WebRtcNsx_set_policy_core(...)
	 *
	 * This changes the aggressiveness of the noise suppression method.
	 *
	 * Input:
	 *      - inst       : Instance that should be initialized
	 *      - mode       : 0: Mild (6 dB), 1: Medium (10 dB), 2: Aggressive (15 dB)
	 *
	 * Output:
	 *      - inst       : Initialized instance
	 *
	 * Return value      :  0 - Ok
	 *                     -1 - Error
	 */
	int WebRtcNsx_set_policy_core(NsxInst_t* inst, int mode);

	/****************************************************************************
	 * WebRtcNsx_ProcessCore
	 *
	 * Do noise suppression.
	 *
	 * Input:
	 *      - inst          : Instance that should be initialized
	 *      - inFrameLow    : Input speech frame for lower band
	 *      - inFrameHigh   : Input speech frame for higher band
	 *
	 * Output:
	 *      - inst          : Updated instance
	 *      - outFrameLow   : Output speech frame for lower band
	 *      - outFrameHigh  : Output speech frame for higher band
	 *
	 * Return value         :  0 - OK
	 *                        -1 - Error
	 */
	int WebRtcNsx_ProcessCore(NsxInst_t* inst,
		short* inFrameLow,
		short* inFrameHigh,
		short* outFrameLow,
		short* outFrameHigh);
	int WebRtcNsx_ProcessCoreMSbc(NsxInst_t* inst,
		short* inFrameLow,
		short* inFrameHigh,
		short* outFrameLow,
		short* outFrameHigh);

	/****************************************************************************
	 * Some function pointers, for internal functions shared by ARM NEON and
	 * generic C code.
	 */
	 // Noise Estimation.
	typedef void (*NoiseEstimation)(NsxInst_t* inst,
		uint16_t* magn,
		uint32_t* noise,
		int16_t* q_noise);
	extern NoiseEstimation WebRtcNsx_NoiseEstimation;

	// Filter the data in the frequency domain, and create spectrum.
	typedef void (*PrepareSpectrum)(NsxInst_t* inst,
		int16_t* freq_buff);
	extern PrepareSpectrum WebRtcNsx_PrepareSpectrum;

	// For the noise supression process, synthesis, read out fully processed
	// segment, and update synthesis buffer.
	typedef void (*SynthesisUpdate)(NsxInst_t* inst,
		int16_t* out_frame,
		int16_t gain_factor);
	extern SynthesisUpdate WebRtcNsx_SynthesisUpdate;

	// Update analysis buffer for lower band, and window data before FFT.
	typedef void (*AnalysisUpdate)(NsxInst_t* inst,
		int16_t* out,
		int16_t* new_speech);
	extern AnalysisUpdate WebRtcNsx_AnalysisUpdate;

#ifdef __cplusplus
extern "C" {
#endif

/*
 * This function creates an instance to the noise reduction structure
 *
 * Input:
 *      - nsxInst       : Pointer to noise reduction instance that should be
 *                       created
 *
 * Output:
 *      - nsxInst       : Pointer to created noise reduction instance
 *
 * Return value         :  0 - Ok
 *                        -1 - Error
 */
NsxHandle* WebRtcNsx_Create(NsxHandle** nsxInst);


/*
 * This function frees the dynamic memory of a specified Noise Suppression
 * instance.
 *
 * Input:
 *      - nsxInst       : Pointer to NS instance that should be freed
 *
 * Return value         :  0 - Ok
 *                        -1 - Error
 */
int WebRtcNsx_Free(NsxHandle* nsxInst);


/*
 * This function initializes a NS instance
 *
 * Input:
 *      - nsxInst       : Instance that should be initialized
 *      - fs            : sampling frequency
 *
 * Output:
 *      - nsxInst       : Initialized instance
 *
 * Return value         :  0 - Ok
 *                        -1 - Error
 */
int WebRtcNsx_Init(NsxHandle* nsxInst, uint32_t fs);

/*
 * This changes the aggressiveness of the noise suppression method.
 *
 * Input:
 *      - nsxInst       : Instance that should be initialized
 *      - mode          : 0: Mild, 1: Medium , 2: Aggressive
 *
 * Output:
 *      - nsxInst       : Initialized instance
 *
 * Return value         :  0 - Ok
 *                        -1 - Error
 */
int WebRtcNsx_set_policy(NsxHandle* nsxInst, int mode);

/*
 * This functions does noise suppression for the inserted speech frame. The
 * input and output signals should always be 10ms (80 or 160 samples).
 *
 * Input
 *      - nsxInst       : NSx instance. Needs to be initiated before call.
 *      - speechFrame   : Pointer to speech frame buffer for L band
 *      - speechFrameHB : Pointer to speech frame buffer for H band
 *      - fs            : sampling frequency
 *
 * Output:
 *      - nsxInst       : Updated NSx instance
 *      - outFrame      : Pointer to output frame for L band
 *      - outFrameHB    : Pointer to output frame for H band
 *
 * Return value         :  0 - OK
 *                        -1 - Error
 */

struct RealFFT *WebRtcSpl_CreateRealFFT(int order);

 
int WebRtcNsx_Process(NsxHandle* nsxInst,
                      short* speechFrame,
                      short* speechFrameHB,
                      short* outFrame,
                      short* outFrameHB);


void WebRtcSpl_ComplexBitReverse(int16_t* __restrict complex_data, int stages);
int WebRtcSpl_ComplexFFT(int16_t frfi[], int stages, int mode);
int16_t WebRtcSpl_MaxAbsValueW16(const int16_t* vector, size_t length);
int16_t WebRtcSpl_DivW32W16ResW16(int32_t num, int16_t den);
int16_t WebRtcSpl_MaxValueW16(const int16_t* vector, size_t length);
int WebRtcSpl_ComplexIFFT(int16_t frfi[], int stages, int mode);

void WebRtcSpl_FreeRealFFT(struct RealFFT* self);

int WebRtcSpl_RealForwardFFT(struct RealFFT* self,
	const int16_t* real_data_in,
	int16_t* complex_data_out);

int WebRtcSpl_RealInverseFFT(struct RealFFT* self,
	const int16_t* complex_data_in,
	int16_t* real_data_out);

int32_t WebRtcSpl_Energy(int16_t* vector, int vector_length, int* scale_factor);

int16_t WebRtcSpl_GetSizeInBits(uint32_t n);

int WebRtcSpl_GetScalingSquare(int16_t* in_vector, int in_vector_length, int times);

int32_t WebRtcSpl_SqrtFloor(int32_t value);

uint32_t WebRtcSpl_DivU32U16(uint32_t num, uint16_t den);

int32_t WebRtcSpl_DivW32W16(int32_t num, int16_t den);

void WebRtcSpl_MemSetW16(int16_t* ptr, int16_t set_value, int length);

int16_t WebRtcSpl_ZerosArrayW16(int16_t* vector, int16_t length);

static __inline int16_t WebRtcSpl_SatW32ToW16(int32_t value32);

static __inline int16_t WebRtcSpl_AddSatW16(int16_t a, int16_t b);

static __inline int WebRtcSpl_NormW32(int32_t a);

static __inline int WebRtcSpl_NormU32(uint32_t a);

static __inline int WebRtcSpl_NormW16(int16_t a);

static __inline int32_t WebRtc_MulAccumW16(int16_t a, int16_t b, int32_t c);

static __inline int32_t WebRtcSpl_AddSatW32(int32_t l_var1, int32_t l_var2);

static __inline int32_t WebRtcSpl_SubSatW32(int32_t l_var1, int32_t l_var2);

static void UpdateNoiseEstimate(NsxInst_t* inst, int offset);

static void NoiseEstimationC(NsxInst_t* inst,
	uint16_t* magn,
	uint32_t* noise,
	int16_t* q_noise);

static void PrepareSpectrumC(NsxInst_t* inst, int16_t* freq_buf);

static __inline void Denormalize(NsxInst_t* inst, int16_t* in, int factor);

static void SynthesisUpdateC(NsxInst_t* inst,
	int16_t* out_frame,
	int16_t gain_factor);

static __inline void NormalizeRealBuffer(NsxInst_t* inst,
	const int16_t* in,
	int16_t* out);

void WebRtcNsx_CalcParametricNoiseEstimate(NsxInst_t* inst,
	int16_t pink_noise_exp_avg,
	int32_t pink_noise_num_avg,
	int freq_index,
	uint32_t* noise_estimate,
	uint32_t* noise_estimate_avg);



void WebRtcNsx_ComputeSpectralFlatness(NsxInst_t* inst, uint16_t* magn);

void WebRtcNsx_ComputeSpectralDifference(NsxInst_t* inst, uint16_t* magnIn);

void WebRtcNsx_SpeechNoiseProb(NsxInst_t* inst, uint16_t* nonSpeechProbFinal,
	uint32_t* priorLocSnr, uint32_t* postLocSnr);

void WebRtcNsx_DataAnalysis(NsxInst_t* inst, short* speechFrame, uint16_t* magnU16);

void WebRtcNsx_DataSynthesis(NsxInst_t* inst, short* outFrame);


void noise_suppression(int16_t *in_file, int16_t *out_file) ;


int nsProcess_init(void);




