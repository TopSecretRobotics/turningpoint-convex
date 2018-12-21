# Turning Point ConVEX project

[![Build Status](https://travis-ci.org/TopSecretRobotics/turningpoint-convex.svg?branch=master)](https://travis-ci.org/TopSecretRobotics/turningpoint-convex)

This project expects [PROS](http://pros.cs.purdue.edu/) to be installed and available in the `PATH`.

The project may be built by running:

```bash
make
```

And sent to the Cortex by running:

```bash
make flash
```

#### Code Formatting

If you have [`clang-format`](https://clang.llvm.org/docs/ClangFormat.html) installed, you can run the following for format all of the C code for consistency:

```bash
make format
```

#### Shell

On macOS and Linux, you can connect to the device's terminal by running:

```bash
make shell
```

To exit, press `CTRL+A, CTRL+\` and type `y`.

On Windows, you will need to download [PuTTY](http://www.chiark.greenend.org.uk/~sgtatham/putty/download.html) and run `make lsusb` to see which device to connect to at speed 115200.

#### Docker

Usage with docker:

```bash
docker build -t "turningpoint-convex" -f Dockerfile .
docker run --rm -v `pwd`:/build/project "turningpoint-convex" make
```

Usage with docker-compose:

```bash
# Start containers
docker-compose up -d
docker attach convex_project_1
# Stop containers
docker-compose stop
```

#### Documentation

* [ChibiOS/RT 2.6.x](http://chibios.sourceforge.net/html/)
* [ConVEX API](https://jpearman.github.io/convex/doxygen/html/)
