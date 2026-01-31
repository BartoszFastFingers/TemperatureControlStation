/* MATLAB GENERATED SOURCE FILE: SMA1_fir.c */
#include "SMA1_fir.h"
// Filter state
uint32_t SMA1_STATE_UINT[SMA1_NUM_TAPS+SMA1_BLOCK_SIZE-1] = {
  #include "SMA1_state_init.csv"
};
float32_t *SMA1_STATE = (float32_t*)SMA1_STATE_UINT;
// Filter coefficients
uint32_t SMA1_COEFFS_UINT[SMA1_NUM_TAPS] = {
  #include "SMA1_coeffs.csv"
};
float32_t *SMA1_COEFFS = (float32_t*)SMA1_COEFFS_UINT;
// Filter instance
arm_fir_instance_f32 SMA1;
