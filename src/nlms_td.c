/*
 * src/nlms_td.c
 *
 * Implementation of the NLMS time-domain API declared in inc/nlms_td.h
 *
 * Notes:
 * - The implementation uses a single contiguous memory block when caller
 *   provides `pMem` or when the library allocates internally.
 * - The state is opaque (only visible as `NLMS_TD_HANDLE`).
 */
#include "nlms_td.h"

#include <stdint.h> /* for integer types */
#include <stdbool.h> /* for bool */
#include <stddef.h> /* for size_t */
#include <assert.h> /* for assert */
#include <string.h> /* for memset */
#include <math.h>  /* for fabsf */

/* Internal state structure (opaque to users). It contains pointers into
 * a contiguous block for delay lines and filter coefficients. */
typedef struct {
    nlms_td_config_t params;
    /* Reference signal delay line */
    float *pReferenceDelayLine;
    /* Filter coefficients */
    float *pFilter;
} nlms_td_state_internal_t;

static void setup_pointers(nlms_td_state_internal_t *s, void *memBlock);
static void update_filter(nlms_td_state_internal_t *s, float e);

/* Compute required size for given config */
size_t nlms_td_get_required_size(nlms_td_config_t const * const poConfig)
{
    assert(poConfig != NULL);

    assert (poConfig->nFilterSize != 0 && poConfig->nFilterSize <= NLMS_TD_MAX_FILTER_SIZE);

    /* add size of the state structure */
    size_t n = sizeof(nlms_td_state_internal_t);
    /* add size of reference delay line */
    n += (size_t)poConfig->nFilterSize * sizeof(float);
    /* add size of filter coefficients */
    n += (size_t)poConfig->nFilterSize * sizeof(float);

    /* align to pointer size for safety */
    size_t align = sizeof(void*);
    if (n % align)
    {
        n += (align - (n % align));
    }

    return n;
}

/* Create instance */
NLMS_TD_STATUS_T nlms_td_create(nlms_td_config_t const * const poConfig,
                                void * const pMem,
                                const size_t memSize,
                                NLMS_TD_HANDLE * const phNlmsTd)
{
    assert(phNlmsTd != NULL && pMem != NULL);

    assert(poConfig->nStepSize > 0.0f && poConfig->nStepSize <= NLMS_TD_MAX_STEP_SIZE);

    size_t required = nlms_td_get_required_size(poConfig);

    if (memSize < required) {
        return NLMS_TD_STATUS_FAILED;
    }

    /* initialize handle to NULL */
    *phNlmsTd = NULL;

    /* Place the internal structure at the start of raw memory */
    nlms_td_state_internal_t *s = (nlms_td_state_internal_t *)pMem;

    /* zero only the struct first (arrays will be zeroed separately) */
    memset(s, 0, sizeof(nlms_td_state_internal_t));

    s->params = *poConfig;

    setup_pointers(s, pMem);

    /* Initialize arrays to zero (input delay, reference delay, filter) */
    size_t bytes = s->params.nFilterSize * sizeof(float);
    memset(s->pReferenceDelayLine, 0, bytes);
    memset(s->pFilter, 0, bytes);

    *phNlmsTd = (NLMS_TD_HANDLE)s;

    return NLMS_TD_STATUS_SUCCESS;
}

/* Run one iteration:
 * - updates filter: w += (mu / (eps + ||x||^2)) * x * e
 * - pushes nRef into the input-delay (newest at index 0)
 * - computes output y = dot(filter, inputDelay)
 */
float nlms_td_run(const NLMS_TD_HANDLE hNlmsTd, 
                             const float nRef, 
                             const float nErr)
{
    assert (hNlmsTd != NULL);

    nlms_td_state_internal_t *s = (nlms_td_state_internal_t *)hNlmsTd;
    uint16_t N = s->params.nFilterSize;
    float *x = s->pReferenceDelayLine;
    float *w = s->pFilter;

    /* update the filter */
    update_filter(s, nErr);

    /* shift delay line to the right by one position */
    memmove(&x[1], &x[0], (N - 1) * sizeof(float));
    /* insert new sample at position 0 */
    x[0] = nRef;

    /* compute output y = dot(w, x) */
    float y = 0.0f;
    for (uint16_t i = 0; i < N; i++) {
        y += w[i] * x[i];
    }

    /* invert the sign of the output if requested */
    if (s->params.bInvertSign) {
        y = -y;
    }

    return y;
}

float *nlms_td_get_filter_ptr(const NLMS_TD_HANDLE hNlmsTd)
{
    assert (hNlmsTd != NULL);

    nlms_td_state_internal_t *s = (nlms_td_state_internal_t *)hNlmsTd;
    return s->pFilter;
}

/* Helper to initialize internal pointers into a contiguous block */
static void setup_pointers(nlms_td_state_internal_t *s, void *memBlock)
{
    unsigned char *base = (unsigned char *)memBlock + sizeof(nlms_td_state_internal_t);
    /* ensure base is aligned to float; the earlier alignment step makes this safe */
    s->pReferenceDelayLine = (float *)base;
    s->pFilter = s->pReferenceDelayLine + s->params.nFilterSize;
}


static void update_filter(nlms_td_state_internal_t *s, float e)
{
    float *w = s->pFilter;
    float *x = s->pReferenceDelayLine;
    uint16_t N = s->params.nFilterSize;

    /* compute squared norm of x (for normalization) */
    float norm2 = 0.0f;
    for (uint16_t i = 0; i < N; i++) 
    {
        float xi = x[i];
        norm2 += xi * xi;
    }

    /* normalization term: avoid divide-by-zero */
    const float epsilon = 1e-8f;
    float denom = norm2 + epsilon;

    /* step size (mu) */
    float mu = s->params.nStepSize;

    /* coefficient update = (mu / denom) * e */
    float coef = (mu / denom) * e;

    for (uint16_t i = 0; i < N; i++) {
        w[i] += coef * x[i];
    }
}