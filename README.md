# Project's Title
Spectify: Transforming Regular Vulnerabilities into Spectre Vulnerabilities

# Project Description
This repository shows multiple Spectre V1 proof-of-concepts. Spectre V1 is a subset of the Spectre vulnerability type, which specifically involves branch prediction (as opposed to address speculation or exception prediction). 

Very briefly put: Spectre vulnerabilities rely on wrong (branch) predictions during speculative execution. When a wrong branch is taken speculatively, variables are loaded into cache. This wrong branch could be an if-statement checking the password, due to the data accessed within it being sensitive. When the CPU later verifies that the branch was incorrectly taken, a machine clear occurs which removes all speculative execution progress. However, the loaded variables still remain in cache. Using a side channel like Flush+Reload, the value of these variables can sometimes be leaked. 

This paper shows 5 different classic architectural (read: regular) vulnerability times, with classic mitigations in comments. These can be found in the [vulnerability]_arch.c files. Furthermore, a speculative counterpart (Spectre V1) for these architectural vulnerabilities is presented ([vulnerability].c). Explanations for how they work can be found in the paper (paper.pdf).

The specific proof-of-concepts presented are:
- Buffer overread (bounds-check-bypass)
- Buffer overflow
- Use-after-free
- Uninitialied read
- Race condition (mutex-lock-bypass)

## What the application does
Each proof-of-concept is executed many times with different parameters.
The process starts in the ’Main’ phase (1). This phase repeats the
overall experiment multiple times. Not every leakage attempt is
successful, for example, due to an OS rescheduling which makes a
measurement take longer than it otherwise would have. Hence, the
experiment to leak the secret string is repeated multiple times for
robustness. The proof-of-concepts leak one byte at a time. Hence,
an individual ’Experiment’ (2) repetition involves iterating through
each character of the secret string. For each character, the ’Prepare’ (3) phase is entered. Before a leakage attempt is made, the
proof-of-concept is executed multiple times to ’Train’ (4) the branch
predictor to take a favorable branch for the leakage attempt. After
training, the ’Proof-of-concept’ (5) vulnerability executes with the
right parameters to leak a byte.

## Which technologies were used
- C language
- Python3
- GCC compiler
- Make

## Challenges
Creating Spectre vulnerabilities requires both software and hardware knowledge. Furthermore, since these vulnerabilities are undefined behaviour, things like a compiler optimization can prevent your code to work as you intended. As a result, sometimes you have to look at the generated assembly code to know what is actually going on under the hood. 

Furthermore, it is not always easy to deduce which variables are on the same cache or memory at a certain point in time. This is especially difficult because variables sometimes reside on the same cache line (hence, when you flush one variable from cache, the other variable on that cache line will also be flushed). 

## Possible future work
Since this Bachelor's thesis has been completed, I will no longer be actively working on this project. I will accept submissions of new proof-of-concepts, given that they follow the existing structure. 

# How to Install and Run the Project




## Prerequisites
If you ended up here, I expect you already have these installed. Nevertheless, make you sure you have _gcc_, _python3_, and _make_ installed:
```
sudo apt-get install gcc
sudo apt-get install python3
sudo apt-get install make
```

## Usage

Every proof-of-concept directory contains a _makefile_, [vulnerability]_arch.c and [vulnerability].c. To run, execute the following command:

```
make && ./bcb
```

Alternatively, you can make use of the provided script. This script will repeat every proof-of-concept experiment multiple times and parse the results. From the root directory, execute the following command:

```
python3 scripts/run_all.py
```

The script will save the output in the _data_ directory. Examples can be found in the directory _example_data_



# Troubleshooting
Unfortunately, the code will not run on all CPU architectures (this was beyond the scope of this project). The proof-of-concepts were specifically designed for and tested on an Intel i7-6700HQ chip. The proof-of-concepts were also succesfully tested on an Intel i5-7500 and an Intel i5-850. Lastly, the code was tested on an AMD Ryzen 7 5800h, on which the proof-of-concepts did not work at all. 

If you're lucky, you only have to change a few constants to make it work after all. More specifically, you can try increasing the value of the constant value _CACHE_HIT_. You can also increase the number of repetitions through constant _REPETITIONS_. 

## No luck on your machine?
I also tested the proof-of-concepts on a DigitalOcean droplet (regular Intel - 1 vCPU). Without any troubleshooting, this ran reasonably well. Results can be found in the directory _example_data_, with which you can compare your own results. 

# Acknowledgements and sources
During this Bachelor thesis project, I had help from:
- Prof. Cristiano Giuffrida - research direction advice
- PhD. student Mathé Hertogh - technical help
- VUSec Dep. at Vrije Universiteit Amsterdam - weekly meetings

Sources can be found in the paper (paper.pdf)

# Contact
Any questions? Feel free to reach out. I'd love to help elaborate on any questions you might have. I'd also love to hear about and help with your Spectre-related projects. You can send me an e-mail at:

Academic: c.c.j.wagenaar@student.vu.nl<br>
Business: ccj.wagenaar@gmail.com


