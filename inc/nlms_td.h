#ifndef NLMS_TD_H_
#define NLMS_TD_H_

#include <stdio.h>

/* Number of filter taps */
#define NLMS_TD_MAX_FILTER_SIZE (1024U)

/* Algorithm handle */
typedef void* NLMS_TD_HANDLE;

/* status enum */
typedef enum {
    NLMS_TD_STATUS_SUCCESS = 0,
    NLMS_TD_STATUS_FAILED = 1
}NLMS_TD_STATUS_T;

/* Algorithm configuration data */
typedef struct {
    /* Filter size */
    float nFilterSize;
    /* Step size */
    float nStepSize;
}nlms_td_config_t;

/* Algorithm state data */
typedef struct {
    /* Input delay line */
    float anInputDelayLine[NLMS_TD_MAX_FILTER_SIZE];
    /* Reference delay line */
    float anReferenceDelayLine[NLMS_TD_MAX_FILTER_SIZE];
    /* Filter coefficients */
    float anFilter[NLMS_TD_MAX_FILTER_SIZE];
    /* algorithm parameters */
    nlms_td_config_t oParams;
}nlms_td_state_t;

/*
* INPUTS
* poConfig - algorithm configuration parameters
* INOUT
* pMem - memory provided for the algorithm
* OUTPUTS
* phNlmsTd - pointer to algorithm handle - if successful,
*            algorithm handle is created here, else it is NULL
*/
NLMS_TD_STATUS_T nlms_td_create(nlms_td_config_t const * const poConfig, void * const pMem, NLMS_TD_HANDLE * const phNlmsTd);

/*
* INPUTS
* nIn - input sample for the current iteration
* nRef - reference sample for the current iteration
* nErr - error sample for the current iteration
* OUTPUTS
* pnOut - output sample - 
*/
NLMS_TD_STATUS_T nlms_td_run(float nIn, float nRef, float nErr, float *pnOut);

#endif /* NLMS_TD_H_ */