/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

// Performs echo control (suppression) with fft routines in fixed-point.

#ifndef  _AUDIO_PROCESSING_AECM_H_
#define _AUDIO_PROCESSING_AECM_H_

//#define AECM_WITH_ABS_APPROX


#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>


#include <stdint.h>
#include <stddef.h>  // size_t
#include "nr_aec_common.h"



static const int32_t kMaxBitCountsQ9 = (32 << 9);  // 32 matching bits in Q9.

typedef struct {
    // Pointer to bit counts.
    int *far_bit_counts;
    // Binary history variables.
    uint32_t *binary_far_history;
    int history_size;
} BinaryDelayEstimatorFarend;

typedef struct {
    // Pointer to bit counts.
    int32_t *mean_bit_counts;
    // Array only used locally in ProcessBinarySpectrum() but whose size is
    // determined at run-time.
    int32_t *bit_counts;

    // Binary history variables.
    uint32_t *binary_near_history;
    int near_history_size;
    int history_size;

    // Delay estimation variables.
    int32_t minimum_probability;
    int last_delay_probability;

    // Delay memory.
    int last_delay;

    // Robust validation
    int robust_validation_enabled;
    int allowed_offset;
    int last_candidate_delay;
    int compare_delay;
    int candidate_hits;
    int *histogram;
    int last_delay_histogram;

    // For dynamically changing the lookahead when using SoftReset...().
    int lookahead;

    // Far-end binary spectrum history buffer etc.
    BinaryDelayEstimatorFarend *farend;
} BinaryDelayEstimator;

// Releases the memory allocated by
// WebRtc_CreateBinaryDelayEstimatorFarend(...).
// Input:
//    - self              : Pointer to the binary delay estimation far-end
//                          instance which is the return value of
//                          WebRtc_CreateBinaryDelayEstimatorFarend().
//
void WebRtc_FreeBinaryDelayEstimatorFarend(BinaryDelayEstimatorFarend *self);

// Allocates the memory needed by the far-end part of the binary delay
// estimation. The memory needs to be initialized separately through
// WebRtc_InitBinaryDelayEstimatorFarend(...).
//
// Inputs:
//      - history_size    : Size of the far-end binary spectrum history.
//
// Return value:
//      - BinaryDelayEstimatorFarend*
//                        : Created |handle|. If the memory can't be allocated
//                          or if any of the input parameters are invalid NULL
//                          is returned.
//
BinaryDelayEstimatorFarend *WebRtc_CreateBinaryDelayEstimatorFarend(
        int history_size);

// Re-allocates the buffers.
//
// Inputs:
//      - self            : Pointer to the binary estimation far-end instance
//                          which is the return value of
//                          WebRtc_CreateBinaryDelayEstimatorFarend().
//      - history_size    : Size of the far-end binary spectrum history.
//
// Return value:
//      - history_size    : The history size allocated.
int WebRtc_AllocateFarendBufferMemory(BinaryDelayEstimatorFarend *self,
                                      int history_size);

// Initializes the delay estimation far-end instance created with
// WebRtc_CreateBinaryDelayEstimatorFarend(...).
//
// Input:
//    - self              : Pointer to the delay estimation far-end instance.
//
// Output:
//    - self              : Initialized far-end instance.
//
void WebRtc_InitBinaryDelayEstimatorFarend(BinaryDelayEstimatorFarend *self);

// Soft resets the delay estimation far-end instance created with
// WebRtc_CreateBinaryDelayEstimatorFarend(...).
//
// Input:
//    - delay_shift   : The amount of blocks to shift history buffers.
//
void WebRtc_SoftResetBinaryDelayEstimatorFarend(
        BinaryDelayEstimatorFarend *self, int delay_shift);

// Adds the binary far-end spectrum to the internal far-end history buffer. This
// spectrum is used as reference when calculating the delay using
// WebRtc_ProcessBinarySpectrum().
//
// Inputs:
//    - self                  : Pointer to the delay estimation far-end
//                              instance.
//    - binary_far_spectrum   : Far-end binary spectrum.
//
// Output:
//    - self                  : Updated far-end instance.
//
void WebRtc_AddBinaryFarSpectrum(BinaryDelayEstimatorFarend *self,
                                 uint32_t binary_far_spectrum);

// Releases the memory allocated by WebRtc_CreateBinaryDelayEstimator(...).
//
// Note that BinaryDelayEstimator utilizes BinaryDelayEstimatorFarend, but does
// not take ownership of it, hence the BinaryDelayEstimator has to be torn down
// before the far-end.
//
// Input:
//    - self              : Pointer to the binary delay estimation instance
//                          which is the return value of
//                          WebRtc_CreateBinaryDelayEstimator().
//
void WebRtc_FreeBinaryDelayEstimator(BinaryDelayEstimator *self);

// Allocates the memory needed by the binary delay estimation. The memory needs
// to be initialized separately through WebRtc_InitBinaryDelayEstimator(...).
//
// See WebRtc_CreateDelayEstimator(..) in delay_estimator_wrapper.c for detailed
// description.
BinaryDelayEstimator *WebRtc_CreateBinaryDelayEstimator(
        BinaryDelayEstimatorFarend *farend, int max_lookahead);

// Re-allocates |history_size| dependent buffers. The far-end buffers will be
// updated at the same time if needed.
//
// Input:
//      - self            : Pointer to the binary estimation instance which is
//                          the return value of
//                          WebRtc_CreateBinaryDelayEstimator().
//      - history_size    : Size of the history buffers.
//
// Return value:
//      - history_size    : The history size allocated.
int WebRtc_AllocateHistoryBufferMemory(BinaryDelayEstimator *self,
                                       int history_size);

// Initializes the delay estimation instance created with
// WebRtc_CreateBinaryDelayEstimator(...).
//
// Input:
//    - self              : Pointer to the delay estimation instance.
//
// Output:
//    - self              : Initialized instance.
//
void WebRtc_InitBinaryDelayEstimator(BinaryDelayEstimator *self);

// Soft resets the delay estimation instance created with
// WebRtc_CreateBinaryDelayEstimator(...).
//
// Input:
//    - delay_shift   : The amount of blocks to shift history buffers.
//
// Return value:
//    - actual_shifts : The actual number of shifts performed.
//
int WebRtc_SoftResetBinaryDelayEstimator(BinaryDelayEstimator *self,
                                         int delay_shift);

// Estimates and returns the delay between the binary far-end and binary near-
// end spectra. It is assumed the binary far-end spectrum has been added using
// WebRtc_AddBinaryFarSpectrum() prior to this call. The value will be offset by
// the lookahead (i.e. the lookahead should be subtracted from the returned
// value).
//
// Inputs:
//    - self                  : Pointer to the delay estimation instance.
//    - binary_near_spectrum  : Near-end binary spectrum of the current block.
//
// Output:
//    - self                  : Updated instance.
//
// Return value:
//    - delay                 :  >= 0 - Calculated delay value.
//                              -2    - Insufficient data for estimation.
//
int WebRtc_ProcessBinarySpectrum(BinaryDelayEstimator *self,
                                 uint32_t binary_near_spectrum);

// Returns the last calculated delay updated by the function
// WebRtc_ProcessBinarySpectrum(...).
//
// Input:
//    - self                  : Pointer to the delay estimation instance.
//
// Return value:
//    - delay                 :  >= 0 - Last calculated delay value
//                              -2    - Insufficient data for estimation.
//
int WebRtc_binary_last_delay(BinaryDelayEstimator *self);

// Returns the estimation quality of the last calculated delay updated by the
// function WebRtc_ProcessBinarySpectrum(...). The estimation quality is a value
// in the interval [0, 1].  The higher the value, the better the quality.
//
// Return value:
//    - delay_quality         :  >= 0 - Estimation quality of last calculated
//                                      delay value.
float WebRtc_binary_last_delay_quality(BinaryDelayEstimator *self);

// Updates the |mean_value| recursively with a step size of 2^-|factor|. This
// function is used internally in the Binary Delay Estimator as well as the
// Fixed point wrapper.
//
// Inputs:
//    - new_value             : The new value the mean should be updated with.
//    - factor                : The step size, in number of right shifts.
//
// Input/Output:
//    - mean_value            : Pointer to the mean value.
//
void WebRtc_MeanEstimatorFix(int32_t new_value,
                             int factor,
                             int32_t *mean_value);

// Releases the memory allocated by WebRtc_CreateDelayEstimatorFarend(...)
void WebRtc_FreeDelayEstimatorFarend(void *handle);

// Allocates the memory needed by the far-end part of the delay estimation. The
// memory needs to be initialized separately through
// WebRtc_InitDelayEstimatorFarend(...).
//
// Inputs:
//  - spectrum_size     : Size of the spectrum used both in far-end and
//                        near-end. Used to allocate memory for spectrum
//                        specific buffers.
//  - history_size      : The far-end history buffer size. A change in buffer
//                        size can be forced with WebRtc_set_history_size().
//                        Note that the maximum delay which can be estimated is
//                        determined together with WebRtc_set_lookahead().
//
// Return value:
//  - void*             : Created |handle|. If the memory can't be allocated or
//                        if any of the input parameters are invalid NULL is
//                        returned.
void *WebRtc_CreateDelayEstimatorFarend(int spectrum_size, int history_size);

// Initializes the far-end part of the delay estimation instance returned by
// WebRtc_CreateDelayEstimatorFarend(...)
int WebRtc_InitDelayEstimatorFarend(void *handle);

// Soft resets the far-end part of the delay estimation instance returned by
// WebRtc_CreateDelayEstimatorFarend(...).
// Input:
//      - delay_shift   : The amount of blocks to shift history buffers.
void WebRtc_SoftResetDelayEstimatorFarend(void *handle, int delay_shift);

// Adds the far-end spectrum to the far-end history buffer. This spectrum is
// used as reference when calculating the delay using
// WebRtc_ProcessSpectrum().
//
// Inputs:
//    - far_spectrum    : Far-end spectrum.
//    - spectrum_size   : The size of the data arrays (same for both far- and
//                        near-end).
//    - far_q           : The Q-domain of the far-end data.
//
// Output:
//    - handle          : Updated far-end instance.
//
int WebRtc_AddFarSpectrumFix(void *handle,
                             const uint16_t *far_spectrum,
                             int spectrum_size,
                             int far_q);



// Releases the memory allocated by WebRtc_CreateDelayEstimator(...)
void WebRtc_FreeDelayEstimator(void *handle);

// Allocates the memory needed by the delay estimation. The memory needs to be
// initialized separately through WebRtc_InitDelayEstimator(...).
//
// Inputs:
//      - farend_handle : Pointer to the far-end part of the delay estimation
//                        instance created prior to this call using
//                        WebRtc_CreateDelayEstimatorFarend().
//
//                        Note that WebRtc_CreateDelayEstimator does not take
//                        ownership of |farend_handle|, which has to be torn
//                        down properly after this instance.
//
//      - max_lookahead : Maximum amount of non-causal lookahead allowed. The
//                        actual amount of lookahead used can be controlled by
//                        WebRtc_set_lookahead(...). The default |lookahead| is
//                        set to |max_lookahead| at create time. Use
//                        WebRtc_set_lookahead(...) before start if a different
//                        value is desired.
//
//                        Using lookahead can detect cases in which a near-end
//                        signal occurs before the corresponding far-end signal.
//                        It will delay the estimate for the current block by an
//                        equal amount, and the returned values will be offset
//                        by it.
//
//                        A value of zero is the typical no-lookahead case.
//                        This also represents the minimum delay which can be
//                        estimated.
//
//                        Note that the effective range of delay estimates is
//                        [-|lookahead|,... ,|history_size|-|lookahead|)
//                        where |history_size| is set through
//                        WebRtc_set_history_size().
//
// Return value:
//      - void*         : Created |handle|. If the memory can't be allocated or
//                        if any of the input parameters are invalid NULL is
//                        returned.
void *WebRtc_CreateDelayEstimator(void *farend_handle, int max_lookahead);

// Initializes the delay estimation instance returned by
// WebRtc_CreateDelayEstimator(...)
int WebRtc_InitDelayEstimator(void *handle);

// Soft resets the delay estimation instance returned by
// WebRtc_CreateDelayEstimator(...)
// Input:
//      - delay_shift   : The amount of blocks to shift history buffers.
//
// Return value:
//      - actual_shifts : The actual number of shifts performed.
int WebRtc_SoftResetDelayEstimator(void *handle, int delay_shift);

// Sets the effective |history_size| used. Valid values from 2. We simply need
// at least two delays to compare to perform an estimate. If |history_size| is
// changed, buffers are reallocated filling in with zeros if necessary.
// Note that changing the |history_size| affects both buffers in far-end and
// near-end. Hence it is important to change all DelayEstimators that use the
// same reference far-end, to the same |history_size| value.
// Inputs:
//  - handle            : Pointer to the delay estimation instance.
//  - history_size      : Effective history size to be used.
// Return value:
//  - new_history_size  : The new history size used. If the memory was not able
//                        to be allocated 0 is returned.
int WebRtc_set_history_size(void *handle, int history_size);

// Returns the history_size currently used.
// Input:
//      - handle        : Pointer to the delay estimation instance.
int WebRtc_history_size(const void *handle);

// Sets the amount of |lookahead| to use. Valid values are [0, max_lookahead]
// where |max_lookahead| was set at create time through
// WebRtc_CreateDelayEstimator(...).
//
// Input:
//      - handle        : Pointer to the delay estimation instance.
//      - lookahead     : The amount of lookahead to be used.
//
// Return value:
//      - new_lookahead : The actual amount of lookahead set, unless |handle| is
//                        a NULL pointer or |lookahead| is invalid, for which an
//                        error is returned.
int WebRtc_set_lookahead(void *handle, int lookahead);

// Returns the amount of lookahead we currently use.
// Input:
//      - handle        : Pointer to the delay estimation instance.
int WebRtc_lookahead(void *handle);

// Sets the |allowed_offset| used in the robust validation scheme.  If the
// delay estimator is used in an echo control component, this parameter is
// related to the filter length.  In principle |allowed_offset| should be set to
// the echo control filter length minus the expected echo duration, i.e., the
// delay offset the echo control can handle without quality regression.  The
// default value, used if not set manually, is zero.  Note that |allowed_offset|
// has to be non-negative.
// Inputs:
//  - handle            : Pointer to the delay estimation instance.
//  - allowed_offset    : The amount of delay offset, measured in partitions,
//                        the echo control filter can handle.
int WebRtc_set_allowed_offset(void *handle, int allowed_offset);

// Returns the |allowed_offset| in number of partitions.
int WebRtc_get_allowed_offset(const void *handle);

// Enables/Disables a robust validation functionality in the delay estimation.
// This is by default set to disabled at create time.  The state is preserved
// over a reset.
// Inputs:
//      - handle        : Pointer to the delay estimation instance.
//      - enable        : Enable (1) or disable (0) this feature.
int WebRtc_enable_robust_validation(void *handle, int enable);

// Returns 1 if robust validation is enabled and 0 if disabled.
int WebRtc_is_robust_validation_enabled(const void *handle);

// Estimates and returns the delay between the far-end and near-end blocks. The
// value will be offset by the lookahead (i.e. the lookahead should be
// subtracted from the returned value).
// Inputs:
//      - handle        : Pointer to the delay estimation instance.
//      - near_spectrum : Pointer to the near-end spectrum data of the current
//                        block.
//      - spectrum_size : The size of the data arrays (same for both far- and
//                        near-end).
//      - near_q        : The Q-domain of the near-end data.
//
// Output:
//      - handle        : Updated instance.
//
// Return value:
//      - delay         :  >= 0 - Calculated delay value.
//                        -1    - Error.
//                        -2    - Insufficient data for estimation.
int WebRtc_DelayEstimatorProcessFix(void *handle,
                                    const uint16_t *near_spectrum,
                                    int spectrum_size,
                                    int near_q);


// Returns the last calculated delay updated by the function
// WebRtc_DelayEstimatorProcess(...).
//
// Input:
//      - handle        : Pointer to the delay estimation instance.
//
// Return value:
//      - delay         : >= 0  - Last calculated delay value.
//                        -1    - Error.
//                        -2    - Insufficient data for estimation.
int WebRtc_last_delay(void *handle);



enum Wrap {
    SAME_WRAP, DIFF_WRAP
};

typedef struct RingBuffer {
    size_t read_pos;
    size_t write_pos;
    size_t element_count;
    size_t element_size;
    enum Wrap rw_wrap;
    char *data;
} RingBuffer;

// Creates and initializes the buffer. Returns null on failure.
RingBuffer *WebRtc_CreateBuffer(size_t element_count, size_t element_size,uint8_t Buffer_number);


void WebRtc_InitBuffer(RingBuffer *handle);

void WebRtc_FreeBuffer(void *handle);

// Reads data from the buffer. Returns the number of elements that were read.
// The |data_ptr| will point to the address where the read data is located.
// If no data can be read, |data_ptr| is set to |NULL|. If all data can be read
// without buffer wrap around then |data_ptr| will point to the location in the
// buffer. Otherwise, the data will be copied to |data| (memory allocation done
// by the user) and |data_ptr| points to the address of |data|. |data_ptr| is
// only guaranteed to be valid until the next call to WebRtc_WriteBuffer().
//
// To force a copying to |data|, pass a null |data_ptr|.
//
// Returns number of elements read.
size_t WebRtc_ReadBuffer(RingBuffer *handle,
                         void **data_ptr,
                         void *data,
                         size_t element_count);

// Writes |data| to buffer and returns the number of elements written.
size_t WebRtc_WriteBuffer(RingBuffer *handle, const void *data,
                          size_t element_count);

// Moves the buffer read position and returns the number of elements moved.
// Positive |element_count| moves the read position towards the write position,
// that is, flushing the buffer. Negative |element_count| moves the read
// position away from the the write position, that is, stuffing the buffer.
// Returns number of elements moved.
int WebRtc_MoveReadPtr(RingBuffer *handle, int element_count);

// Returns number of available elements to read.
size_t WebRtc_available_read(const RingBuffer *handle);

// Returns number of available elements for write.
size_t WebRtc_available_write(const RingBuffer *handle);

#ifdef __cplusplus
}
#endif
#ifndef MODULES_AUDIO_PROCESSING_AECM_AECM_DEFINES_H_
#define MODULES_AUDIO_PROCESSING_AECM_AECM_DEFINES_H_

#define AEC_MAX_LOOKAHEAD (1)

#define AECM_DYNAMIC_Q                 /* Turn on/off dynamic Q-domain. */

/* Algorithm parameters */
#define FRAME_LEN       64             /* Total frame length, 8 ms. */

#define PART_LEN        64             /* Length of partition. */
#define PART_LEN_SHIFT  7              /* Length of (PART_LEN * 2) in base 2. */

#define PART_LEN1       (PART_LEN + 1)  /* Unique fft coefficients. */
#define PART_LEN2       (PART_LEN << 1) /* Length of partition * 2. */
#define PART_LEN4       (PART_LEN << 2) /* Length of partition * 4. */
#define FAR_BUF_LEN     PART_LEN4       /* Length of buffers. */
#define MAX_DELAY       20

/* Counter parameters */
#define CONV_LEN        512          /* Convergence length used at startup. */
#define CONV_LEN2       (CONV_LEN << 1) /* Used at startup. */

/* Energy parameters */
#define MAX_BUF_LEN     64           /* History length of energy signals. */
#define FAR_ENERGY_MIN  1025         /* Lowest Far energy level: At least 2 */
/* in energy. */
#define FAR_ENERGY_DIFF 929          /* Allowed difference between max */
/* and min. */
#define ENERGY_DEV_OFFSET       0    /* The energy error offset in Q8. */
#define ENERGY_DEV_TOL  400          /* The energy estimation tolerance (Q8). */
#define FAR_ENERGY_VAD_REGION   230  /* Far VAD tolerance region. */

/* Stepsize parameters */
#define MU_MIN          10          /* Min stepsize 2^-MU_MIN (far end energy */
/* dependent). */
#define MU_MAX          1           /* Max stepsize 2^-MU_MAX (far end energy */
/* dependent). */
#define MU_DIFF         9           /* MU_MIN - MU_MAX */

/* Channel parameters */
#define MIN_MSE_COUNT   20 /* Min number of consecutive blocks with enough */
/* far end energy to compare channel estimates. */
#define MIN_MSE_DIFF    29 /* The ratio between adapted and stored channel to */
/* accept a new storage (0.8 in Q-MSE_RESOLUTION). */
#define MSE_RESOLUTION  5           /* MSE parameter resolution. */
#define RESOLUTION_CHANNEL16    12  /* W16 Channel in Q-RESOLUTION_CHANNEL16. */
#define RESOLUTION_CHANNEL32    28  /* W32 Channel in Q-RESOLUTION_CHANNEL. */
#define CHANNEL_VAD     16          /* Minimum energy in frequency band */
/* to update channel. */

/* Suppression gain parameters: SUPGAIN parameters in Q-(RESOLUTION_SUPGAIN). */
#define RESOLUTION_SUPGAIN      8     /* Channel in Q-(RESOLUTION_SUPGAIN). */
#define SUPGAIN_DEFAULT (1 << RESOLUTION_SUPGAIN)  /* Default. */
#define SUPGAIN_ERROR_PARAM_A   3072  /* Estimation error parameter */
/* (Maximum gain) (8 in Q8). */
#define SUPGAIN_ERROR_PARAM_B   1536  /* Estimation error parameter */
/* (Gain before going down). */
#define SUPGAIN_ERROR_PARAM_D   SUPGAIN_DEFAULT /* Estimation error parameter */
/* (Should be the same as Default) (1 in Q8). */
#define SUPGAIN_EPC_DT  200     /* SUPGAIN_ERROR_PARAM_C * ENERGY_DEV_TOL */

/* Defines for "check delay estimation" */
#define CORR_WIDTH      31      /* Number of samples to correlate over. */
#define CORR_MAX        16      /* Maximum correlation offset. */
#define CORR_MAX_BUF    63
#define CORR_DEV        4
#define CORR_MAX_LEVEL  20
#define CORR_MAX_LOW    4
#define CORR_BUF_LEN    ((CORR_MAX << 1) + 1)
/* Note that CORR_WIDTH + 2*CORR_MAX <= MAX_BUF_LEN. */

#define ONE_Q14         (1 << 14)

/* NLP defines */
#define NLP_COMP_LOW    3277    /* 0.2 in Q14 */
#define NLP_COMP_HIGH   ONE_Q14 /* 1 in Q14 */

#endif

#ifdef _MSC_VER  // visual c++
#define ALIGN8_BEG __declspec(align(8))
#define ALIGN8_END
#else  // gcc or icc
#define ALIGN8_BEG
#define ALIGN8_END __attribute__((aligned(8)))
#endif

// Macros specific for the fixed point implementation

// TODO(kma/bjorn): For the next two macros, investigate how to correct the code
// for inputs of a = WEBRTC_SPL_WORD16_MIN or WEBRTC_SPL_WORD32_MIN.



typedef struct {
    int16_t real;
    int16_t imag;
} ComplexInt16;

typedef struct {
    int farBufWritePos;
    int farBufReadPos;
    int knownDelay;
    int lastKnownDelay;
    int firstVAD;  // Parameter to control poorly initialized channels

    RingBuffer *farFrameBuf;
    RingBuffer *nearNoisyFrameBuf;
    RingBuffer *outFrameBuf;

    int16_t farBuf[FAR_BUF_LEN];

    int16_t mult;
    uint32_t seed;

    // Delay estimation variables
    void *delay_estimator_farend;
    void *delay_estimator;
    uint16_t currentDelay;
    // Far end history variables
    // TODO(bjornv): Replace |far_history| with ring_buffer.
    uint16_t far_history[PART_LEN1 * MAX_DELAY];
    int far_history_pos;
    int far_q_domains[MAX_DELAY];

    int16_t nlpFlag;
    int16_t fixedDelay;

    uint32_t totCount;

    int16_t dfaCleanQDomain;
    int16_t dfaCleanQDomainOld;
    int16_t dfaNoisyQDomain;
    int16_t dfaNoisyQDomainOld;

    int16_t nearLogEnergy[MAX_BUF_LEN];
    int16_t farLogEnergy;
    int16_t echoAdaptLogEnergy[MAX_BUF_LEN];
    int16_t echoStoredLogEnergy[MAX_BUF_LEN];

    // The extra 16 or 32 bytes in the following buffers are for alignment based
    // Neon code.
    // It's designed this way since the current GCC compiler can't align a
    // buffer in 16 or 32 byte boundaries properly.
    int16_t channelStored_buf[PART_LEN1];
    int16_t channelAdapt16_buf[PART_LEN1 ];
    int32_t channelAdapt32_buf[PART_LEN1 ];
    int16_t xBuf_buf[PART_LEN2 ];  // farend
    int16_t dBufNoisy_buf[PART_LEN2 ];  // nearend
    int16_t outBuf_buf[PART_LEN];

    // Pointers to the above buffers
    int16_t *channelStored;
    int16_t *channelAdapt16;
    int32_t *channelAdapt32;
    int16_t *xBuf;
    int16_t *dBufClean;
    int16_t *dBufNoisy;
    int16_t *outBuf;

    int32_t echoFilt[PART_LEN1];
    int16_t nearFilt[PART_LEN1];
   // int32_t noiseEst[PART_LEN1];
  //  int noiseEstTooLowCtr[PART_LEN1];
  //  int noiseEstTooHighCtr[PART_LEN1];
  //  int16_t noiseEstCtr;
    int16_t cngMode;

    int32_t mseAdaptOld;
    int32_t mseStoredOld;
    int32_t mseThreshold;

    int16_t farEnergyMin;
    int16_t farEnergyMax;
    int16_t farEnergyMaxMin;
    int16_t farEnergyVAD;
    int16_t farEnergyMSE;
    int currentVADValue;
    int16_t vadUpdateCount;

    int16_t startupState;
    int16_t mseChannelCount;
    int16_t supGain;
    int16_t supGainOld;

    int16_t supGainErrParamA;
    int16_t supGainErrParamD;
    int16_t supGainErrParamDiffAB;
    int16_t supGainErrParamDiffBD;

    struct RealFFT real_fft;


} AecmCore;



typedef struct {
    int sampFreq;
    int scSampFreq;
    short bufSizeStart;
    int knownDelay;

    // Stores the last frame added to the farend buffer
    short farendOld[2][FRAME_LEN];
    short initFlag; // indicates if AEC has been initialized

    // Variables used for averaging far end buffer size
    short counter;
    short sum;
    short firstVal;
    short checkBufSizeCtr;

    // Variables used for delay shifts
    short msInSndCardBuf;
    short filtDelay;
    int timeForDelayChange;
    int ECstartup;
    int checkBuffSize;
    int delayChange;
    short lastDelayDiff;

    int16_t echoMode;


    // Structures
    RingBuffer *farendBuf;

    AecmCore *aecmCore;
} AecMobile;

typedef union {
    int32_t float_;
    int32_t int32_;
} SpectrumType;

typedef struct {
    // Pointers to mean values of spectrum.
    SpectrumType *mean_far_spectrum;
    // |mean_far_spectrum| initialization indicator.
    int far_spectrum_initialized;

    int spectrum_size;

    // Far-end part of binary spectrum based delay estimation.
    BinaryDelayEstimatorFarend *binary_farend;
} DelayEstimatorFarend;

typedef struct {
    // Pointers to mean values of spectrum.
    SpectrumType *mean_near_spectrum;
    // |mean_near_spectrum| initialization indicator.
    int near_spectrum_initialized;

    int spectrum_size;

    // Binary spectrum based delay estimator
    BinaryDelayEstimator *binary_handle;
} DelayEstimator;


typedef struct {
AecMobile gAecMobile;
AecmCore gAecmCore;
RingBuffer gRingBuffer0;
RingBuffer gRingBuffer1;
RingBuffer gRingBuffer2;
RingBuffer gRingBuffer3;
DelayEstimatorFarend gDelayEstimatorFarend;
BinaryDelayEstimatorFarend gBinaryDelayEstimatorFarend;
DelayEstimator gDelayEstimator;
BinaryDelayEstimator gBinaryDelayEstimator;
SpectrumType gSpectrumType0[520];
SpectrumType gSpectrumType1[520];
char elementaddr0[256];
char elementaddr1[256];
char elementaddr2[256];
char elementaddr3[3840];
AecMobile  * gAecMobilePtr;
int16_t gechoMode;
int16_t msInSndCardBuf;

int binary_far_history[MAX_DELAY*4];
int far_bit_counts[MAX_DELAY*4];
int mean_bit_counts[(MAX_DELAY + 1)*4];
int bit_counts[MAX_DELAY*4];
int histogram[(MAX_DELAY + 1)*4];
uint32_t binary_near_history[AEC_MAX_LOOKAHEAD];
}AECVariable;


////////////////////////////////////////////////////////////////////////////////
// WebRtcAecm_CreateCore()
//
// Allocates the memory needed by the AECM. The memory needs to be
// initialized separately using the WebRtcAecm_InitCore() function.
// Returns a pointer to the instance and a nullptr at failure.
AecmCore *WebRtcAecm_CreateCore(void);

////////////////////////////////////////////////////////////////////////////////
// WebRtcAecm_InitCore(...)
//
// This function initializes the AECM instant created with
// WebRtcAecm_CreateCore()
// Input:
//      - aecm          : Pointer to the AECM instance
//      - samplingFreq  : Sampling Frequency
//
// Output:
//      - aecm          : Initialized instance
//
// Return value         :  0 - Ok
//                        -1 - Error
//
int WebRtcAecm_InitCore(AecmCore *aecm, int samplingFreq);

////////////////////////////////////////////////////////////////////////////////
// WebRtcAecm_FreeCore(...)
//
// This function releases the memory allocated by WebRtcAecm_CreateCore()
// Input:
//      - aecm          : Pointer to the AECM instance
//
void WebRtcAecm_FreeCore(AecmCore *aecm);

int WebRtcAecm_Control(AecmCore *aecm, int delay, int nlpFlag);

////////////////////////////////////////////////////////////////////////////////
// WebRtcAecm_InitEchoPathCore(...)
//
// This function resets the echo channel adaptation with the specified channel.
// Input:
//      - aecm          : Pointer to the AECM instance
//      - echo_path     : Pointer to the data that should initialize the echo
//                        path
//
// Output:
//      - aecm          : Initialized instance
//
void WebRtcAecm_InitEchoPathCore(AecmCore *aecm, const int16_t *echo_path);

////////////////////////////////////////////////////////////////////////////////
// WebRtcAecm_ProcessFrame(...)
//
// This function processes frames and sends blocks to
// WebRtcAecm_ProcessBlock(...)
//
// Inputs:
//      - aecm          : Pointer to the AECM instance
//      - farend        : In buffer containing one frame of echo signal
//      - nearendNoisy  : In buffer containing one frame of nearend+echo signal
//                        without NS
//      - nearendClean  : In buffer containing one frame of nearend+echo signal
//                        with NS
//
// Output:
//      - out           : Out buffer, one frame of nearend signal          :
//
//
int WebRtcAecm_ProcessFrame(AecmCore *aecm,
                            const int16_t *farend,
                            const int16_t *nearendNoisy,
                            int16_t *out);

////////////////////////////////////////////////////////////////////////////////
// WebRtcAecm_ProcessBlock(...)
//
// This function is called for every block within one frame
// This function is called by WebRtcAecm_ProcessFrame(...)
//
// Inputs:
//      - aecm          : Pointer to the AECM instance
//      - farend        : In buffer containing one block of echo signal
//      - nearendNoisy  : In buffer containing one frame of nearend+echo signal
//                        without NS
//      - nearendClean  : In buffer containing one frame of nearend+echo signal
//                        with NS
//
// Output:
//      - out           : Out buffer, one block of nearend signal          :
//
//
int WebRtcAecm_ProcessBlock(AecmCore *aecm,
                            const int16_t *farend,
                            const int16_t *nearendNoisy,
                            int16_t *out);

////////////////////////////////////////////////////////////////////////////////
// WebRtcAecm_BufferFarFrame()
//
// Inserts a frame of data into farend buffer.
//
// Inputs:
//      - aecm          : Pointer to the AECM instance
//      - farend        : In buffer containing one frame of farend signal
//      - farLen        : Length of frame
//
void WebRtcAecm_BufferFarFrame(AecmCore *aecm,
                               const int16_t *farend,
                               int farLen);

////////////////////////////////////////////////////////////////////////////////
// WebRtcAecm_FetchFarFrame()
//
// Read the farend buffer to account for known delay
//
// Inputs:
//      - aecm          : Pointer to the AECM instance
//      - farend        : In buffer containing one frame of farend signal
//      - farLen        : Length of frame
//      - knownDelay    : known delay
//
void WebRtcAecm_FetchFarFrame(AecmCore *aecm,
                              int16_t *farend,
                              int farLen,
                              int knownDelay);

// All the functions below are intended to be private

////////////////////////////////////////////////////////////////////////////////
// WebRtcAecm_UpdateFarHistory()
//
// Moves the pointer to the next entry and inserts |far_spectrum| and
// corresponding Q-domain in its buffer.
//
// Inputs:
//      - self          : Pointer to the delay estimation instance
//      - far_spectrum  : Pointer to the far end spectrum
//      - far_q         : Q-domain of far end spectrum
//
void WebRtcAecm_UpdateFarHistory(AecmCore *self,
                                 uint16_t *far_spectrum,
                                 int far_q);

////////////////////////////////////////////////////////////////////////////////
// WebRtcAecm_AlignedFarend()
//
// Returns a pointer to the far end spectrum aligned to current near end
// spectrum. The function WebRtc_DelayEstimatorProcessFix(...) should have been
// called before AlignedFarend(...). Otherwise, you get the pointer to the
// previous frame. The memory is only valid until the next call of
// WebRtc_DelayEstimatorProcessFix(...).
//
// Inputs:
//      - self              : Pointer to the AECM instance.
//      - delay             : Current delay estimate.
//
// Output:
//      - far_q             : The Q-domain of the aligned far end spectrum
//
// Return value:
//      - far_spectrum      : Pointer to the aligned far end spectrum
//                            NULL - Error
//
const uint16_t *WebRtcAecm_AlignedFarend(AecmCore *self, int *far_q, int delay);

///////////////////////////////////////////////////////////////////////////////
// WebRtcAecm_CalcSuppressionGain()
//
// This function calculates the suppression gain that is used in the
// Wiener filter.
//
// Inputs:
//      - aecm              : Pointer to the AECM instance.
//
// Return value:
//      - supGain           : Suppression gain with which to scale the noise
//                            level (Q14).
//
int16_t WebRtcAecm_CalcSuppressionGain(AecmCore *aecm);

///////////////////////////////////////////////////////////////////////////////
// WebRtcAecm_CalcEnergies()
//
// This function calculates the log of energies for nearend, farend and
// estimated echoes. There is also an update of energy decision levels,
// i.e. internal VAD.
//
// Inputs:
//      - aecm              : Pointer to the AECM instance.
//      - far_spectrum      : Pointer to farend spectrum.
//      - far_q             : Q-domain of farend spectrum.
//      - nearEner          : Near end energy for current block in
//                            Q(aecm->dfaQDomain).
//
// Output:
//     - echoEst            : Estimated echo in Q(xfa_q+RESOLUTION_CHANNEL16).
//
void WebRtcAecm_CalcEnergies(AecmCore *aecm,
                             const uint16_t *far_spectrum,
                             int16_t far_q,
                             uint32_t nearEner,
                             int32_t *echoEst);

///////////////////////////////////////////////////////////////////////////////
// WebRtcAecm_CalcStepSize()
//
// This function calculates the step size used in channel estimation
//
// Inputs:
//      - aecm              : Pointer to the AECM instance.
//
// Return value:
//      - mu                : Stepsize in log2(), i.e. number of shifts.
//
int16_t WebRtcAecm_CalcStepSize(AecmCore *aecm);

///////////////////////////////////////////////////////////////////////////////
// WebRtcAecm_UpdateChannel(...)
//
// This function performs channel estimation.
// NLMS and decision on channel storage.
//
// Inputs:
//      - aecm              : Pointer to the AECM instance.
//      - far_spectrum      : Absolute value of the farend signal in Q(far_q)
//      - far_q             : Q-domain of the farend signal
//      - dfa               : Absolute value of the nearend signal
//                            (Q[aecm->dfaQDomain])
//      - mu                : NLMS step size.
// Input/Output:
//      - echoEst           : Estimated echo in Q(far_q+RESOLUTION_CHANNEL16).
//
void WebRtcAecm_UpdateChannel(AecmCore *aecm,
                              const uint16_t *far_spectrum,
                              int16_t far_q,
                              const uint16_t *dfa,
                              int16_t mu,
                              int32_t *echoEst);

extern const int16_t WebRtcAecm_kCosTable[];
extern const int16_t WebRtcAecm_kSinTable[];

///////////////////////////////////////////////////////////////////////////////
// Some function pointers, for internal functions shared by ARM NEON and
// generic C code.
//
typedef void (*CalcLinearEnergies)(AecmCore *aecm,
                                   const uint16_t *far_spectrum,
                                   int32_t *echoEst,
                                   uint32_t *far_energy,
                                   uint32_t *echo_energy_adapt,
                                   uint32_t *echo_energy_stored);

extern CalcLinearEnergies WebRtcAecm_CalcLinearEnergies;

typedef void (*StoreAdaptiveChannel)(AecmCore *aecm,
                                     const uint16_t *far_spectrum,
                                     int32_t *echo_est);

extern StoreAdaptiveChannel WebRtcAecm_StoreAdaptiveChannel;

typedef void (*ResetAdaptiveChannel)(AecmCore *aecm);

extern ResetAdaptiveChannel WebRtcAecm_ResetAdaptiveChannel;

// For the above function pointers, functions for generic platforms are declared
// and defined as static in file aecm_core.c, while those for ARM Neon platforms
// are declared below and defined in file aecm_core_neon.c.
#if defined(WEBRTC_HAS_NEON)
void WebRtcAecm_CalcLinearEnergiesNeon(AecmCore* aecm,
                                       const uint16_t* far_spectrum,
                                       int32_t* echo_est,
                                       uint32_t* far_energy,
                                       uint32_t* echo_energy_adapt,
                                       uint32_t* echo_energy_stored);

void WebRtcAecm_StoreAdaptiveChannelNeon(AecmCore* aecm,
                                         const uint16_t* far_spectrum,
                                         int32_t* echo_est);

void WebRtcAecm_ResetAdaptiveChannelNeon(AecmCore* aecm);
#endif

#if defined(MIPS32_LE)
void WebRtcAecm_CalcLinearEnergies_mips(AecmCore* aecm,
                                        const uint16_t* far_spectrum,
                                        int32_t* echo_est,
                                        uint32_t* far_energy,
                                        uint32_t* echo_energy_adapt,
                                        uint32_t* echo_energy_stored);
#if defined(MIPS_DSP_R1_LE)
void WebRtcAecm_StoreAdaptiveChannel_mips(AecmCore* aecm,
                                          const uint16_t* far_spectrum,
                                          int32_t* echo_est);

void WebRtcAecm_ResetAdaptiveChannel_mips(AecmCore* aecm);
#endif
#endif

enum {
    AecmFalse = 0,
    AecmTrue
};

// Errors
#define AECM_UNSPECIFIED_ERROR           12000
#define AECM_UNSUPPORTED_FUNCTION_ERROR  12001
#define AECM_UNINITIALIZED_ERROR         12002
#define AECM_NULL_POINTER_ERROR          12003
#define AECM_BAD_PARAMETER_ERROR         12004

// Warnings
#define AECM_BAD_PARAMETER_WARNING       12100

typedef struct {
    int16_t cngMode;            // AECM_FALSE, AECM_TRUE (default)
    int16_t echoMode;           // 0, 1, 2, 3 (default), 4
} AecmConfig;

// For ComplexFFT(), the maximum fft order is 10;
// for OpenMax FFT in ARM, it is 12;
// WebRTC APM uses orders of only 7 and 8.
//
   
/*
 * Allocates the memory needed by the AECM. The memory needs to be
 * initialized separately using the WebRtcAecm_Init() function.
 * Returns a pointer to the instance and a nullptr at failure.
 */
void *WebRtcAecm_Create(void);

/*
 * This function releases the memory allocated by WebRtcAecm_Create()
 *
 * Inputs                       Description
 * -------------------------------------------------------------------
 * void*    aecmInst            Pointer to the AECM instance
 */
void WebRtcAecm_Free(void *aecmInst);

/*
 * Initializes an AECM instance.
 *
 * Inputs                       Description
 * -------------------------------------------------------------------
 * void*          aecmInst      Pointer to the AECM instance
 * int32_t        sampFreq      Sampling frequency of data
 *
 * Outputs                      Description
 * -------------------------------------------------------------------
 * int32_t        return        0: OK
 *                              1200-12004,12100: error/warning
 */
int32_t WebRtcAecm_Init(void *aecmInst, int32_t sampFreq);

/*
 * Inserts an 80 or 160 sample block of data into the farend buffer.
 *
 * Inputs                       Description
 * -------------------------------------------------------------------
 * void*          aecmInst      Pointer to the AECM instance
 * int16_t*       farend        In buffer containing one frame of
 *                              farend signal
 * int16_t        nrOfSamples   Number of samples in farend buffer
 *
 * Outputs                      Description
 * -------------------------------------------------------------------
 * int32_t        return        0: OK
 *                              1200-12004,12100: error/warning
 */
int32_t WebRtcAecm_BufferFarend(void *aecmInst,
                                const int16_t *farend,
                                size_t nrOfSamples);

/*
 * Reports any errors that would arise when buffering a farend buffer.
 *
 * Inputs                       Description
 * -------------------------------------------------------------------
 * void*          aecmInst      Pointer to the AECM instance
 * int16_t*       farend        In buffer containing one frame of
 *                              farend signal
 * int16_t        nrOfSamples   Number of samples in farend buffer
 *
 * Outputs                      Description
 * -------------------------------------------------------------------
 * int32_t        return        0: OK
 *                              1200-12004,12100: error/warning
 */
int32_t WebRtcAecm_GetBufferFarendError(void *aecmInst,
                                        const int16_t *farend,
                                        size_t nrOfSamples);

/*
 * Runs the AECM on an 80 or 160 sample blocks of data.
 *
 * Inputs                        Description
 * -------------------------------------------------------------------
 * void*          aecmInst       Pointer to the AECM instance
 * int16_t*       nearendNoisy   In buffer containing one frame of
 *                               reference nearend+echo signal. If
 *                               noise reduction is active, provide
 *                               the noisy signal here.
 * int16_t*       nearendClean   In buffer containing one frame of
 *                               nearend+echo signal. If noise
 *                               reduction is active, provide the
 *                               clean signal here. Otherwise pass a
 *                               NULL pointer.
 * int16_t        nrOfSamples    Number of samples in nearend buffer
 * int16_t        msInSndCardBuf Delay estimate for sound card and
 *                               system buffers
 *
 * Outputs                       Description
 * -------------------------------------------------------------------
 * int16_t*       out            Out buffer, one frame of processed nearend
 * int32_t        return         0: OK
 *                               1200-12004,12100: error/warning
 */
int32_t WebRtcAecm_Process(void *aecmInst,
                           const int16_t *nearendNoisy,
                           int16_t *out,
                           size_t nrOfSamples,
                           int16_t msInSndCardBuf);

/*
 * This function enables the user to set certain parameters on-the-fly
 *
 * Inputs                       Description
 * -------------------------------------------------------------------
 * void*          aecmInst      Pointer to the AECM instance
 * AecmConfig     config        Config instance that contains all
 *                              properties to be set
 *
 * Outputs                      Description
 * -------------------------------------------------------------------
 * int32_t        return        0: OK
 *                              1200-12004,12100: error/warning
 */
int32_t WebRtcAecm_set_config(void *aecmInst, AecmConfig config);

/*
 * This function enables the user to set the echo path on-the-fly.
 *
 * Inputs                       Description
 * -------------------------------------------------------------------
 * void*        aecmInst        Pointer to the AECM instance
 * void*        echo_path       Pointer to the echo path to be set
 * size_t       size_bytes      Size in bytes of the echo path
 *
 * Outputs                      Description
 * -------------------------------------------------------------------
 * int32_t      return          0: OK
 *                              1200-12004,12100: error/warning
 */
int32_t WebRtcAecm_InitEchoPath(void *aecmInst,
                                const void *echo_path,
                                size_t size_bytes);

/*
 * This function enables the user to get the currently used echo path
 * on-the-fly
 *
 * Inputs                       Description
 * -------------------------------------------------------------------
 * void*        aecmInst        Pointer to the AECM instance
 * void*        echo_path       Pointer to echo path
 * size_t       size_bytes      Size in bytes of the echo path
 *
 * Outputs                      Description
 * -------------------------------------------------------------------
 * int32_t      return          0: OK
 *                              1200-12004,12100: error/warning
 */
int32_t WebRtcAecm_GetEchoPath(void *aecmInst,
                               void *echo_path,
                               size_t size_bytes);

/*
 * This function enables the user to get the echo path size in bytes
 *
 * Outputs                      Description
 * -------------------------------------------------------------------
 * size_t       return          Size in bytes
 */
size_t WebRtcAecm_echo_path_size_bytes(void);


#ifdef __cplusplus
}
#endif

void AECM(int16_t *near_file, int16_t *far_file, int16_t *out_file,uint8_t calltype);
AecMobile  *  AECProcess_init( uint32_t sampleRate, size_t samplesCount,int16_t nMode);

int aecProcess(int16_t *far_frame, int16_t *near_frame, uint32_t sampleRate, size_t samplesCount, int16_t nMode,
               int16_t msInSndCardBuf);

static int TimeToFrequencyDomain(AecmCore* aecm,const int16_t* time_signal,ComplexInt16* freq_signal,uint16_t* freq_signal_abs,uint32_t* freq_signal_sum_abs);


extern AECVariable* gpAECStructVariable;

#endif





























