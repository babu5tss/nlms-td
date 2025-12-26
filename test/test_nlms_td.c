#include "nlms_td.h"

#include <stdio.h> //for FILE, fopen, fclose, perror
#include <stdlib.h> //for size_t

int main() {
    /* Reference signal - 1000 uniform random numbers between -0.5 and 0.5 generated with Matlab rand function */
    FILE *x_file = fopen("../test/x.dat", "rb");
    /* Desired signal - the above filtered with 2nd order butterworth LPF with cutoff 1/4th */
    FILE *d_file = fopen("../test/d.dat", "rb");
    /* file to write the error - this should slowly decay to 0 */
    FILE *e_file = fopen("../test/e.dat", "wb");
    /* file to write the weights - this should match the 2nd order butterworth LPF with cutoff 1/4th */
    FILE *w_file = fopen("../test/w.dat", "wb");

    if (d_file == NULL || x_file == NULL) {
        perror("Error opening files");
        return 1;
    }

    printf("Size of float: %zu bytes\n", sizeof(float));

    /* NLMS configuration */
    nlms_td_config_t config = {
        .nFilterSize = 32,
        .nStepSize = 0.1,
        .bInvertSign = false
    };
    printf("NLMS configuration: filter_size=%d, step_size=%f, invert_sign=%s\n",
           config.nFilterSize, config.nStepSize, config.bInvertSign ? "true" : "false");

    /* get memory required */
    size_t mem_size = nlms_td_get_required_size(&config);
    void *mem = malloc(mem_size);
    if (mem == NULL) {
        perror("Memory allocation failed");
        return 1;
    }
    printf("Memory required for NLMS instance: %zu bytes\n", mem_size);

    /* create NLMS instance */
    NLMS_TD_HANDLE hNlmsTd = NULL;
    if (nlms_td_create(&config, mem, mem_size, &hNlmsTd) != NLMS_TD_STATUS_SUCCESS) {
        fprintf(stderr, "NLMS instance creation failed\n");
        free(mem);
        return 1;
    }

    /* process data */
    float d, x, e = 0.0f, y = 0.0f;
    while (fread(&d, 4, 1, d_file) == 1 && fread(&x, 4, 1, x_file) == 1) {
        y = nlms_td_run(hNlmsTd, x, e);
        e = d - y;
        fwrite(&e, sizeof(float), 1, e_file);
    }

    /* write final weights to file */
    float *w = nlms_td_get_filter_ptr(hNlmsTd);
    fwrite(w, sizeof(float), config.nFilterSize, w_file);

    fclose(d_file);
    fclose(x_file);
    fclose(e_file);
    fclose(w_file);
    free(mem);

    return 0;
}