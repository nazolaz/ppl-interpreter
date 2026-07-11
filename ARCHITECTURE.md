# Engine Architecture

This document details the internal architecture of the C++ HOPPL inference engine. The design strictly separates the probabilistic model's evaluation from the mathematical inference algorithms, using an interruptible coroutine-like model.

## 1. The Execution Model (`Machine.h` & `Machine.cpp`)

The core of the evaluator is a Stack Machine that does absolutely no probabilistic inference itself. Its only job is to evaluate deterministic expressions and pause when it hits a probabilistic effect.

* **The Continuation Stacks:** Execution state is maintained using a Control Stack (`C`) and a Value Stack (`V`). Together, these stacks represent the current continuation. This allows the machine `M` to be paused (by returning from the `resume` function) and cloned (by copying the stacks during a `fork`), which is essential for algorithms like SMC.
* **Lexical Scoping & Closures:** To support first-class functions and recursion, the machine implements closures. A closure pairs a function's body with the environment (`env`) where it was defined. During the `callk` instruction, the machine extends the closure's captured environment with the new arguments, ensuring proper lexical scope over caller scope.
* **Execution Flow:** The evaluator runs via `resume(m)` until it hits a probabilistic effect or finishes, at which point it yields control back to the inference controller.

## 2. The Messaging Interface (`Message.h`)

Because the evaluator does not calculate probabilities, it communicates with the inference controllers via message passing. 

When the machine encounters a `sample` or `observe` instruction, it records the current execution address, packages the distribution, and yields a message. We use `std::variant` to represent these messages safely in C++:

* `SampleMsg(address, distribution, machine)`
* `ObserveMsg(address, distribution, observed_value, machine)`
* `DoneMsg(value, machine)`

The inference controller receives this message, performs the necessary math (like drawing from a prior or accumulating log-weights), and then answers the machine via `send(m, value)`, which pushes the result onto the machine's value stack and calls `resume(m)` again.

## 3. The Inference Controllers

The algorithms act as "controllers" over the message stream emitted by the `Machine`. They all share the exact same underlying runtime.

### Likelihood Weighting (`LikelihoodWeighting.h/cpp`)
A straightforward controller. When it receives a `SampleMsg`, it draws a value from the distribution using the RNG and sends it back. When it receives an `ObserveMsg`, it calculates the log-probability of the observed value under the distribution and accumulates it into the particle's total weight.

### Sequential Monte Carlo (`SequentialMonteCarlo.h/cpp`)
SMC maintains a population of multiple `Machine` instances (particles). It advances all machines to their next breakpoint (usually an `observe` message). Once all machines are paused at the same synchronization point, SMC scores their weights, resamples the population to prune low-weight executions, and uses the `fork()` mechanism to clone the surviving machines before resuming them.

### Single-Site Metropolis-Hastings (`SSMetropolisHastings.h/cpp`)
Instead of keeping many machines, SSMH keeps an address-keyed trace of a single machine's execution. 
* In each step, it picks one random address from the previous trace to resample.
* It re-runs the machine, reusing values for all other addresses from the old trace if they are encountered again.
* When the run completes, it calculates the dimension-corrected acceptance ratio (`mh_log_alpha`), which cancels out prior proposals and accounts for reused sites, observations, and trace length changes. 
* It either accepts the new trace or reverts to the old one based on this ratio.

## 4. Random Number Generation (`AnyRNG.h`)

To ensure statistical independence across particles and enable reproducible testing, random number generation is strictly controlled:

* **Move-Only Semantics:** The `AnyRNG` class prevents copying, ensuring that multithreaded environments (like OpenMP loops) don't accidentally share RNG states, which would destroy the mathematical validity of the algorithms.
* **Deterministic vs. Stochastic:** Algorithms can be initialized with an `std::optional<uint32_t> seed`. If no seed is provided, `AnyRNG` uses `std::random_device` for OS-level entropy. If a seed is provided, it uses a template constructor to initialize deterministic chains (e.g., `seed + i` for particles), guaranteeing reproducible benchmarks.

## 5. The CLI Layer (`PPLRunner.h/cpp`)

The `PPLRunner` class encapsulates all I/O and argument parsing, keeping the algorithms pure. 

* **Routing:** It parses command-line flags (`--algo`, `--iter`, `--seed`, `--out`) and instantiates the correct inference controller.
* **Execution:** It invokes the `.run()` methods, passing along the iterations and the optional seed.
* **Exporting:** It calculates the expected values (handling both weighted and unweighted outputs) and automatically routes raw trace data into an `/outputs/` directory structure via the `<filesystem>` library for downstream analysis.