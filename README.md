# Spectre Attack Implementation

## What I implemented

I wrote two main parts: the `readMemoryByte()` function and the `main()` function. The skeleton code already had victim_function, flush, and memaccesstime.

## readMemoryByte()

This is the main attack function. It tries to leak one byte by doing the following:

First I flush array2 from cache completely (all 256 entries). This is important because otherwise we can't tell which one got accessed later.

Then comes the training part - I run about 58 iterations where most of the time I give victim_function a valid index (within bounds), but sometimes (when j%7==0) I give it the malicious out-of-bounds index. The key trick is I also flush array1_size from cache 3 times, which slows down the bounds check. This gives the CPU more time to speculatively execute the out-of-bounds access before it realizes the index is invalid.

The bit manipulation stuff (lines 115-117) is just a branchless way to pick either the training index or malicious index. I did it this way instead of using if statements because I didn't want to mess up the branch predictor even more.

After all that training, I probe array2 to see which entry is cached. I access them in a weird order (i*167+13) because accessing sequentially would trigger the hardware prefetcher and ruin everything. I skip indices 1-16 because those could be from valid array1 accesses. Whichever one is fastest (under 115 cycles) is probably the leaked byte.

I repeat this 100 times and keep a histogram, then the most common result is what we return.

## main()

Pretty straightforward - first I touch all the pages in array2 so we don't get page faults during timing (that would add random delays and mess things up).

Then I calculate where the secret string is relative to array1, and leak it byte by byte. For each byte I call readMemoryByte() and collect results in a histogram.

The branchless max-finding code (lines 171-173) was necessary because I wanted to avoid any if statements that could cause branch mispredictions during the analysis phase.

## Some numbers I picked

- 115 cycles: threshold for cache hit. I tested and cache hits are usually 30-80 cycles, misses are 100+
- 100 trials: seemed like a good balance between accuracy and speed
- j%7 pattern: means roughly 6 training runs for every 1 malicious run
- 95 iteration delay: just for timing, found this worked better than other values
- Triple flush: one flush wasn't reliable enough

## Running it

```
gcc -O0 -o spectre spectre.c
./spectre
```

Important to use -O0 otherwise the compiler optimizes away some of the timing stuff.

## Notes

The success rate depends a lot on what CPU you're running on and what mitigations are enabled. On newer systems with IBRS and other protections it might not work as well but the technique is still sound.
