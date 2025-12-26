#ifndef NLMS_TD_H_
#define NLMS_TD_H_

#include <stdint.h> /* for integer types */
#include <stdbool.h> /* for bool */
#include <stddef.h> /* for size_t */

/* Number of filter taps (compile-time upper bound) */
#define NLMS_TD_MAX_FILTER_SIZE (1024U)
/* Maximum step size allowed by NLMS */
#define NLMS_TD_MAX_STEP_SIZE (2.0f)

/* Algorithm handle (opaque) */
typedef struct nlms_td_state_t *NLMS_TD_HANDLE;

/* status enum */
typedef enum {
    NLMS_TD_STATUS_SUCCESS = 0,
    NLMS_TD_STATUS_FAILED = 1
} NLMS_TD_STATUS_T;

/* Algorithm configuration data */
typedef struct {
    /* Filter size (number of taps) */
    uint16_t nFilterSize;
    /* Step size */
    float nStepSize;
    /* Invert the sign of the output */
    bool bInvertSign;
} nlms_td_config_t;

#ifdef __cplusplus
extern "C" {
#endif

/* 
* Return required memory (bytes) for a given config.
*/
size_t nlms_td_get_required_size(nlms_td_config_t const * const poConfig);

/*
* Create algorithm instance.
* Inputs:
*  - poConfig: algorithm configuration (must not be NULL)
*  - pMem: pointer to memory provided by caller
*  - memSize: size of memory pointed by pMem
* Output:
*  - phNlmsTd: on success will point to instance handle; on failure set to NULL
*/
NLMS_TD_STATUS_T nlms_td_create(nlms_td_config_t const * const poConfig,
                                void * const pMem,
                                const size_t memSize,
                                NLMS_TD_HANDLE * const phNlmsTd);

/*
* Run one iteration of NLMS algorithm.
 * - updates filter: w += (mu / (eps + ||x||^2)) * x * e
 * - pushes nRef into the input-delay (newest at index 0)
 * - computes output y = dot(filter, inputDelay)
* Inputs:
*  - hNlmsTd: handle returned by nlms_td_create
*  - nRef: reference sample for current iteration
*  - nErr: error sample for current iteration
* Output:
*  - pnOut: output sample (must not be NULL)
*/
float nlms_td_run(const NLMS_TD_HANDLE hNlmsTd, 
                             const float nRef, 
                             const float nErr);

/*
* Get pointer to filter coefficients - this is part of the internal state
* Inputs:
*  - hNlmsTd: algorithm handle
* Output:
*  - pointer to filter coefficients array
*/
float *nlms_td_get_filter_ptr(const NLMS_TD_HANDLE hNlmsTd);

#ifdef __cplusplus
}
#endif

#endif /* NLMS_TD_H_ */