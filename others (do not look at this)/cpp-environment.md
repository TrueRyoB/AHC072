# C++ environment for this contest workspace

## AI instructions

Before changing or running a contest solution, read `basic/AGENTS.md`, `basic/original-problem.md`, and the current strategy notes. Follow `basic/AGENTS.md` exactly: after running the solution program, report its output, logs, scores, or other observations and stop. Do not change the solution code, approach, or strategy in response to that run until the user gives a new explicit instruction. Compilation is a build step; executing the resulting solver on an input is a solution run.

Keep toolchain setup separate from contest-solution changes. Never describe a solver as validated unless it has been run against an appropriate judge or verifier.

## Windows setup (MSYS2 UCRT64)

This workspace currently has no `g++`, `clang++`, or `cl` on `PATH`. Install MSYS2, then open its **UCRT64** terminal and install the compiler and debugger:

```sh
pacman -Syu
pacman -S --needed mingw-w64-ucrt-x86_64-toolchain
```

Add `C:\msys64\ucrt64\bin` to the user `PATH`, reopen the terminal, and check:

```powershell
g++ --version
gdb --version
```

## Build and run

From the workspace root:

```powershell
g++ -std=c++20 -O2 -pipe -Wall -Wextra -Wshadow main.cpp -o main.exe
Get-Content path\to\input.txt | .\main.exe
```

For an interactive session, run `.\main.exe` and paste the input. The program reads one instance from standard input and prints one operation per line. Save the build output and execution output separately when recording observations. Do not interpret a successful compile as proof that output is legal or scores well.

## VS Code

Select the UCRT64 `g++.exe` as the C/C++ extension compiler. Keep its compiler path consistent with `C:\msys64\ucrt64\bin\g++.exe`. The current `.vscode/c_cpp_properties.json` only has generic include paths and does not select a compiler.
