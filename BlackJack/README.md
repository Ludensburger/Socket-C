# Blackjack

This directory contains two versions of the Blackjack game: **Monolithic** and **Modular**. Both versions are equipped with a `build.sh` script to simplify the build and run process.

## Directory Overview

- **monolithic/**: Contains a single, tightly-coupled implementation of the Blackjack game.
  - Includes both the **server** and **client** components.
- **modular/**: A modularized implementation of the Blackjack game.
  - Code is split into smaller, reusable modules for better structure and maintainability.

## Build and Run

Both versions include a `build.sh` script, which provides a convenient way to compile and run the program.

### Steps to Build and Run

1. **Navigate to the respective directory:**

   ```sh
   cd monolithic
   # OR
   cd modular
   ```

2. **Run the build script:**

   ```sh
   ./build.sh
   ```

3. **Follow any on-screen instructions to start the game.**

## Requirements

Ensure you have the necessary tools installed (e.g., compilers, interpreters) before running the script.
