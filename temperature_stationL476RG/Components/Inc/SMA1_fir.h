/* MATLAB GENERATED  HEADER FILE: SMA1_sma.h */
#ifndef INC_SMA1_H_
#define INC_SMA1_H_

#include "arm_math.h"

#define SMA1_NUM_TAPS  10
#define SMA1_BLOCK_SIZE  1
// Filter state
extern float32_t *SMA1_STATE;
// Filter coefficients
extern float32_t *SMA1_COEFFS;
// Filter instance
extern arm_fir_instance_f32 SMA1;

#endif // INC_SMA1_H_

