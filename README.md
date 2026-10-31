# First-Order Filter using C

A first-order low-pass filter, `tau y' + y = K u`, discretised exactly as

    y[k] = z0 * y[k-1] + K * (1 - z0) * u[k-1],    z0 = exp(-Te / tau)

and implemented in plain C with file input and output: `TD3.c` reads `enregistrement-bruit.txt` (250 samples of a
noisy 1 Hz sine at Te = 0.01 s), filters it, writes `sortie.txt`, and prints a histogram of what was removed.

```bash
gcc -Wall -Wextra -o td3 TD3.c -lm
./td3
```

An interactive walk-through, with the filter running in the browser and the C explained line by line, is chapter 05
of https://elierouphael.github.io/tutorials/kalman-jax/.

## Corrections (October 2026 review)

`TD3_original.c` is the program as first committed; its `sortie.txt` was 250 lines of `0.00`. Compiling and running
it found three problems, fixed in `TD3.c`:

1. `filtrage()` was called on the 51-sample self-test array `u` instead of the recording `bruit`, so it read 198
   cells past the end of `u` (undefined behaviour) and output a near-zero signal.
2. The difference and its histogram were computed over 500 entries of arrays holding 250 values.
3. `tau = 2` s attenuates the 1 Hz recording to 8% of its amplitude, and the self-test (an impulse, labelled a step
   response) printed 0.00 for every sample at two decimals, so it could not reveal the problem. The time constant
   is now 0.02 s, which passes the sine almost intact but removes no noise (the output is 0.120 RMS from the clean
   sine, against 0.096 for the recording; see below), the self-test is a unit step checked in code, and the output
   is written with four decimals.

`TD3.c` also contains a real-time form of the filter (`filtre_step`, one sample in, one out) and checks that it
matches the batch version. Note that this form feeds on the previous sample, `u[k-1]`, so it forecasts the signal
one sample ahead: on this recording (noise sd 0.096) even its best time constant, about 0.008 s, brings the output
only 1.5% closer to the clean sine than the recording itself. The same recursion fed the current sample,
`y[k] = z0 * y[k-1] + K * (1 - z0) * u[k]`, gets 28% closer, at tau = 0.012 s.
