/* High Pass Filter based on RBJ Cookbook linked above */
/* Analog Transfer Function for this filter: H(s) = s^2 / (s^2 + s/Q + 1) */

/* These floating point values are used by the filter code below */
float Fs = 44100;      /* sample rate in samples per second */
float Pi = 3.141592;   /* the value of Pi */

/* These floating point values implement the specific filter type */
float f0 = 100;                /* cut-off (or center) frequency in Hz */
float Q = 1.5;                 /* filter Q */
float w0 = 2 * Pi * f0 / Fs;
float alpha = sin(w0) / (2 * Q);
float a0 = 1 + alpha;
float a1 = -2 * cos(w0);
float a2 = 1 - alpha;
float b0 = (1 - cos(w0)) / 2;
float b1 = (1 - cos(w0));
float b2 = (1 - cos(w0)) / 2;



float sampleR[5] = {0};
float sampleL[5] = {0};

uint32_t filter_sample(uint32_t mySample) {

    float filtered_sampleR = ( b0 / a0 * sampleR[2]) +
        (b1 / a0 * sampleR[3]) +
        (b2 / a0 * sampleR[4]) -
        (a1 / a0 * sampleR[1]) -
        (a2 / a0 * sampleR[0]);

    // I = I + 1;      /* increment the counter I by adding 1 */
    // }               /* this is the end of the code loop */
    int16_t return_sampleR = filtered_sampleR*INT16_MAX;
    int16_t return_sampleL = sampleL[4]*INT16_MAX;
    uint32_t return_sample = ((uint16_t)(return_sampleR) << 16) | (uint16_t)(return_sampleL);
    sampleR[4] = sampleR[3];
    sampleR[3] = sampleR[2];
    sampleR[2] = sampleR[1];
    sampleR[1] = sampleR[0];
    sampleR[0] = ((int16_t)(mySample >> 16))/(INT16_MAX*1.f);
    sampleL[4] = sampleL[3];
    sampleL[3] = sampleL[2];
    sampleL[2] = sampleL[1];
    sampleL[1] = sampleL[0];
    sampleL[0] = ((int16_t)(mySample & 0b00000000000000001111111111111111))/(INT16_MAX*1.f);

    return return_sample;
}

