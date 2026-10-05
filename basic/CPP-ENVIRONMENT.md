# C++ environment and AI workflow

## Read before working

Before building or running a contest solution, read:

1. `basic/AGENTS.md`
2. `basic/original-problem.md`
3. `strats/idea1.md` and `strats/implementation-plan.md`

Follow `basic/AGENTS.md`'s AtCoder AI rule. If you run the solution program, report its result and stop. Do not use that run's observations to change the solution's code, approach, or strategy unless the user gives a new explicit instruction.

## Current machine status

MSYS2 is installed under `C:\msys64`, but the current installation has only the MSYS runtime packages. There is no `g++`, `clang++`, or `cl` compiler available on `PATH` or in the checked MSYS2 `ucrt64`, `mingw64`, and `clang64` compiler directories. Do not claim the solver compiled or ran until a compiler is present.

Installing an MSYS2 compiler requires downloading packages and writing into `C:\msys64`; do not retry that installation when the user lacks permission to do so. If the user or administrator later makes the compiler available, use the UCRT64 toolchain below.

## When the UCRT64 compiler is available

In the MSYS2 UCRT64 shell, install the C++ toolchain if authorized and permitted:

```sh
pacman -Syu
pacman -S --needed mingw-w64-ucrt-x86_64-gcc
```

The compiler should then be at `C:\msys64\ucrt64\bin\g++.exe`. Add `C:\msys64\ucrt64\bin` to the user `PATH` if desired, reopen the terminal, and confirm with `g++ --version`.

From the workspace root, build with:

```powershell
& 'C:\msys64\ucrt64\bin\g++.exe' -std=c++20 -O2 -pipe -Wall -Wextra -Wshadow main.cpp -o main.exe
```

The solution reads one instance from standard input and writes operations to standard output. Running it is subject to the stop-after-run instruction in `basic/AGENTS.md`. Do not treat a successful build as proof of legal output or a good score.

## What the current solver does

`main.cpp` routes each slime individually along a shortest floor path and moves it one adjacent cell per action. It is the simple completion baseline described in `strats/implementation-plan.md`; it does not implement shared group moves or long jumps.
