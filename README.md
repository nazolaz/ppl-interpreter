# HOPPL-based C++ Inference Engine

A Probabilistic Programming Language (PPL) inference engine written in C++17. 

This engine was developed following the concepts from the **"Introduction to Probabilistic Programming Languages"** course, taught by Javier Burroni during the second semester of 2026 at the **Universidad de Buenos Aires (UBA)**. The language syntax, structural semantics, and evaluation models are heavily inspired by the Higher-Order Probabilistic Programming Language (HOPPL) as detailed in the textbook *An Introduction to Probabilistic Programming* by Jan-Willem van de Meent, Brooks Paige, David Tolpin, and Frank Wood.

The engine evaluates a Lisp-like syntax based on Clojure. It strictly adheres to the formal grammar of the Higher-Order Probabilistic Programming Language, supporting standard deterministic operations alongside stochastic side-effects.

## Theoretical Principles & Architecture

Following the design decisions outlined in the textbook, this engine abandons hardcoded evaluators in favor of an **interruptible computation model**:

* **Separation of Model and Inference:** A probabilistic program is treated as a mostly deterministic, referentially transparent computation that is inherently inference-agnostic. The evaluator processes standard AST nodes automatically but cedes control to an external "inference controller" when stochastic side-effects occur.
* **The Messaging Interface:** When the evaluator reaches a `sample` or `observe` expression, it pauses execution and yields a message (containing the random variable's address and fully-evaluated arguments). The inference algorithm receives this message, performs the necessary probabilistic math, and replies with a value or a command to resume.

For a deep dive into the C++ class implementations, please refer to [ARCHITECTURE.md](ARCHITECTURE.md).

## Language Syntax & Primitives

The engine supports a Clojure-like syntax. Programs must return a single value or expression.

### Control Flow & Core Forms
* `let`: Evaluates a series of bindings and executes a body.
* `if`: Evaluates a test expression to branch into a `then` or `else` execution.
* `fn`: Defines an anonymous function with parameters and a body.

### Deterministic Operations
* **Arithmetic:** `+`, `-`, `*`, `/`.
* **Comparison:** `<`, `>`, `=`.

### Probabilistic Operations
* `(sample <distribution>)`: Draws a value from the specified distribution.
* `(observe <distribution> <value>)`: Conditions the model by evaluating the log-probability of the observed data under the given distribution.

### Supported Distributions
* `normal`: Parameterized by mean `mu` and standard deviation `sigma`.
* `bernoulli`: Parameterized by a probability `p` between 0 and 1.
* `exponential`: Parameterized by a `rate` greater than 0.
* `uniform`: Parameterized by lower bound `a` and upper bound `b`.
* `poisson`: Parameterized by a rate `lam` greater than 0.
* `beta`: Parameterized by shape parameters `alpha` and `beta`.
* `gamma`: Parameterized by `shape` and `rate`.
* `discrete`: Parameterized by a sequence of probabilities that will be automatically normalized.

## Features

* **Explicit Stack Machine:** Evaluates ASTs without relying on the host language's call stack, enabling complex coroutine-like behaviors (pausing, cloning, and resuming execution).
* **Supported Algorithms:** Likelihood Weighting (LW), Sequential Monte Carlo (SMC), and Single-Site Metropolis-Hastings (MH).

## Build Instructions

### Prerequisites
* A C++17 compatible compiler (GCC, Clang)
* CMake 3.15 or higher
* OpenMP (for multi-threading)
* Python 3.x (only for the validation suite)

### Compilation
The project uses standard CMake out-of-source builds.

```bash
mkdir build
cd build
cmake ..
make
```

### Running the Engine (CLI Usage)
You can run the engine directly from your terminal by the path to the text file containing your HOPPL code.
If you just want to run a file with the default settings, simply type the executable name followed by your file's path:

```bash
./build/ppl_engine path/to/your/custom_model.txt
```

#### Customizing the Execution
You can change how the engine processes your model by adding options (flags) after the filename.

**--algo** $<name>$: Chooses the specific inference algorithm. You can pass lw (Likelihood Weighting), smc (Sequential Monte Carlo), or mh (Metropolis-Hastings). If you omit this, the engine defaults to lw.

**--iter** $<number>$: Sets the number of simulations to run (particles for LW/SMC, or steps for MH). More iterations yield higher accuracy but take longer to compute.

**--seed** $<number>$: Forces the engine to use a specific starting point for its random number generator. This guarantees you get the exact same results every time you run it, which is useful for debugging. If you omit this, the engine generates truly random results.

**--out** $<filename>$: Saves the mathematical results into a file (like data.csv) instead of just printing them to your terminal screen.

#### Examples

Run a model using Sequential Monte Carlo with 20,000 particles:

```bash
./build/ppl_engine my_model.txt --algo smc --iter 20000
```
Run a model using Metropolis-Hastings, lock the random seed to 42 for testing, and export the data to a CSV:

```bash
./build/ppl_engine my_model.txt --algo mh --seed 42 --out results.csv
```

##  Heuristic Validation Suite

To validate the implementation of the C++ inference algorithms, we benchmark the engine against exact analytical results and reference Python outputs (Jupyter Notebooks) provided in the UBA course. 

The models are located in `ppl_programs/` and stress-test different capabilities of the engine:

### 1. The Flip Model (Stochastic Control Flow)
```
(let [z (sample (bernoulli 0.5)) 
      a (if z (sample (normal 0 1)) 0) 
      b (sample (normal 10 1))] 
  (observe (normal (+ a b) 1) 10.0) 
  z)
```
* **File:** `flip_model.txt` | **Algorithm:** Likelihood Weighting (`--algo lw`)
* **Concept:** Tests stochastic branching. A Bernoulli flip determines if a specific branch of the AST executes, altering the sequence of downstream random variables.
* **Course Expected Mean:** `~0.414`

### 2. The 8-Bit Model (Combinatorial Latents)
```
(let [b1 (if (sample (bernoulli 0.5)) 1 0)
      b2 (if (sample (bernoulli 0.5)) 1 0)
      b3 (if (sample (bernoulli 0.5)) 1 0)
      b4 (if (sample (bernoulli 0.5)) 1 0)
      b5 (if (sample (bernoulli 0.5)) 1 0)
      b6 (if (sample (bernoulli 0.5)) 1 0)
      b7 (if (sample (bernoulli 0.5)) 1 0)
      b8 (if (sample (bernoulli 0.5)) 1 0)
      total (+ b1 b2 b3 b4 b5 b6 b7 b8)]
  (observe (normal 7 1) total)
  total)
```
* **File:** `8bit_model.txt` | **Algorithm:** Metropolis-Hastings (`--algo mh`)
* **Concept:** A combinatorial problem of 8 independent bits where the sum is observed to be near 7. It forces the Single-Site MH algorithm to properly propose and accept/reject transitions across 256 possible states.
* **Course Expected Mean:** `~6.000`

### 3. 3-Step Random Walk (Sequential HMM)
```
(let [
  z1 (sample (normal 0 1))
  o1 (observe (normal z1 0.5) 1.2)
  z2 (sample (normal z1 1.0))
  o2 (observe (normal z2 0.5) 1.5)
  z3 (sample (normal z2 1.0))
  o3 (observe (normal z3 0.5) 2.0)
] z3)
```
* **File:** `random_walk_model.txt` | **Algorithm:** Sequential Monte Carlo (`--algo smc`)
* **Concept:** Simulates a short Hidden Markov Model where states are transitioned and observed sequentially. SMC handles this by cloning/forking the surviving particles at each `observe` statement.
* **Course Expected Mean:** `~1.898` 

### 4. Bayesian Linear Regression 
```
(let [
  w (sample (normal 0 3))
  b (sample (normal 0 3))
  
  mu1 (+ (* w -2.0) b)
  o1 (observe (normal mu1 0.3) -1.0)
  
  mu2 (+ (* w 1.5) b)
  o2 (observe (normal mu2 0.3) 2.0)
  
  mu3 (+ (* w 3.0) b)
  o3 (observe (normal mu3 0.3) 1.8)
] w)
```
* **File:** `linreg.txt` | **Algorithm:** Likelihood Weighting (`--algo lw`)
* **Concept:** Continuous parameter estimation. Infers the slope of a line given 3 data points and narrow observation noise.
* **Course Expected Mean (Slope):** `~0.612`

### Running the Tests
To automatically execute these models and verify the outputs against the course baselines, run the validation script from the root directory:

```bash
python validate.py
```