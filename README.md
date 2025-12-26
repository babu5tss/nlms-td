## Noise Cancellation Architecture

```
┌─────────────────────────────────────────────────────┐
│                                                     │
│                             Primary Input Signal (d)│
│                             (Desired Signal + Noise)│
│                                                 |   │
│                                                 |   │
│                                                 │   │
│                                                 ▼   │
│ Reference ------──────>[NLMS]─── Output(y) ─>[-\+]  │
│ Input (x)                ▲                      │   │
│ (Noise Correlated)       │                      │   |
│                       Error feedback            |   |
│                       (for weights update)      |   |
│                          │                      │   |
│                          -───────────────-------|   │
│                                                 │   │
│                                                 ▼   │
│                                     Error Output (e)│
│                                  (Enhanced Signal)  │
│                                                     │
└─────────────────────────────────────────────────────┘
```

**Components:**
- **d(n)**: Primary input (desired signal + noise)
- **x(n)**: Reference input (correlated with noise)
- **y(n)**: Adaptive filter output (filtered reference)
- **e(n)**: Error signal (cleaned output)

## NLMS Algorithm
<br>**Filteration step:** y(n) = W(n)·X(n)
<br>**Weight update:** W(n) = W(n-1) + (μ/|X(n-1)|²)·X(n-1)·e(n-1)
- **μ**: Step size parameter (scalar - controls convergence speed)
- **W(n)**: Filter weights at iteration n (vector of size filter length)
- **X(n)**: Reference delay line (vector of size filter length)
- **|X(n)|²**: Normalized power of reference input (scalar)
- **e(n)**: Error value (scalar)

## Implementation
- **nlms_td_run** need to be called for every sample with the latest sample of x(n) and e(n).
- The function first performs a weight update and use the updated weights to provide y(n).

## Example
### Test vectors
- **x.dat (float32)**: reference signal - 1000 samples of uniform random values in range (-0.5, 0.5) generated with Matlab rand function
- **d.dat (float32)**: desired signal - the above signal filtered with a 2nd order butterworth LPF with a cutoff frequency of 1/4th
### Test
- The test call the NLMS run function for each sample of the test vectors.
- Error is calculated by subtracting the NLMS output from desired signal.
- The error signal and the final filter weights are written into e.dat and x.data respectively.
- These can be imported into Matlab for visualization. 