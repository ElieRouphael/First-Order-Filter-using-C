# First-Order Filter using C

A first-order low-pass filter, `tau y' + y = K u`, discretised exactly as

    y[k] = z0 * y[k-1] + K * (1 - z0) * u[k-1],    z0 = exp(-Te / tau)

and implemented in plain C with file input and output: `TD3.c` reads `enregistrement-bruit.txt` (250 samples of a
noisy 1 Hz sine at Te = 0.01 s), filters it, writes `sortie.txt`, and prints a histogram of what was removed.

```bash
gcc -Wall -Wextra -o td3 TD3.c
./td3
```

An interactive walk-through, with the filter running in the browser and the C explained line by line, is chapter 05
of https://elierouphael.github.io/tutorials/kalman-jax/.

## Corrections (October 2026 review)

`TD3_original.c` is the program as first committed; its `sortie.txt` was 250 lines of `0.00`. Compiling and running
it found three problems, fixed in `TD3.c`:

1. `filtrage()` was called on the 51-sample self-test array `u` instead of the recording `bruit`, so it read 199
   cells past the end of `u` (undefined behaviour) and output a near-zero signal.
2. The difference and its histogram were computed over 500 entries of arrays holding 250 values.
3. `tau = 2` s attenuates the 1 Hz recording to 8% of its amplitude, and the self-test (an impulse, labelled a step
   response) printed 0.00 for every sample at two decimals, so it could not reveal either problem. The time constant
   is now 0.02 s, the self-test is a unit step checked in code, and the output is written with four decimals.

`TD3.c` also contains a real-time form of the filter (`filtre_step`, one sample in, one out) and checks that it
matches the batch version. Note that no causal first-order filter makes this recording closer to the clean sine
than it already is (noise sd 0.096): the lag costs more than the smoothing saves at 100 samples per period.
