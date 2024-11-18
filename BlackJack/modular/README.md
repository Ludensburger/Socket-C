# Blackjack Game

This project is a modular implementation of a Blackjack game written in C. The code is split into separate files to improve maintainability and organization, addressing the challenges of having everything in only two files. A `bash.sh` script is also included to streamline the build process, making it simple to compile and execute the game.

## Project Structure

The project’s modular structure consists of multiple files, each representing distinct functionality or aspects of the game. This organization makes the code more readable and manageable, allowing for easier updates and expansions.

### Key Features

- **Modularized Code**: The game’s logic is split into multiple files based on functionality, making it straightforward to understand and maintain.
- **Efficient Build Process**: The `bash.sh` script compiles all necessary files and links them for execution, simplifying the build process.
- **Playable Blackjack**: The core functionality is a simple, text-based Blackjack game, with player and dealer interactions handled within the terminal.

## How to Build and Run

To compile and run the game, you can use the included `bash.sh` script:

1. **Open a Terminal (or PowerShell)**:

   - Navigate to the directory containing your game files.

2. **Run the Script**:
   - In Linux/Mac, use:
     ```bash
     ./bash.sh
     ```
   - In Windows PowerShell, run:
     ```powershell
     ./bash.sh
     ```

This command compiles the game, creating an executable that you can play in the terminal.

## Game Overview

Blackjack, also known as 21, is a popular card game where players aim to achieve a hand value as close to 21 as possible without going over. This implementation follows the basic rules:

- **Player vs. Dealer**: The player competes against the dealer, aiming to get closer to 21 than the dealer without busting.
- **Basic Commands**: Hit, Stand, and other simple commands are used to play.
- **Scoring and Conditions**: The game checks for win, loss, or tie conditions based on Blackjack rules.

### Implementation Details

- **Modularity**: Each file in the project handles a specific aspect of the game, such as card logic, game rules, and user interaction, making it easy to locate and modify code segments.
- **Bash Script**: The `bash.sh` script compiles all modules and links them, so only one command is needed to build and run the game.
