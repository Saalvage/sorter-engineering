# Programming exercise

Prerequisites:
- UNIX-like OS, or WSL for Windows
- `gcc`, `g++` >= 11
- `CMake` >= 3.19 [^1]
- `python` and the package `matplotlib` (alternatively, you can use `pipenv` [^2] and the provided `Pipfile`)

## Usage

Implement your solution in `src/`, and describe it in `description.md`.
You can also adjust the evaluation code in `framework/` and `eval.py`, if you want.

1. Compile your code using

```console
cmake --preset release
cmake --build --preset release
```

2. Run the experiment via `eval.py run`. The output will be written to `result.txt`.
```console
python ./eval.py run result.txt
# or
cmake --build --preset release --target run
```

3. Create a plot via `eval.py plot`. This will create a PDF file named `plot.pdf`.
```console
python ./eval.py plot results.txt
# or
cmake --build --preset release --target plot
```

4. Describe your solution in `description.md`.

5. Using `cmake`, create `submission.zip`, which will contain your source code, description and plots.
```console
cmake --build --preset release --target submission
```

[^1]: https://cmake.org/download/
[^2]: https://pipenv.pypa.io/en/latest/

