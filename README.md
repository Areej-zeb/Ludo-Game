# Multithreaded Ludo Game – Operating Systems Project

## Overview

This project is a multithreaded implementation of the classic board game **Ludo** using **C++**. The game supports up to four players and is designed to demonstrate core operating system concepts including:

- Threads
- Mutexes
- Semaphores
- Conditional Variables

Each player in the game is represented by a separate thread, ensuring concurrent gameplay while maintaining synchronization and consistency using OS-level primitives.

---

## Features

- Console-based Ludo game with color-coded board
- Supports 2 to 4 players
- Dynamic board rendering
- Thread-based player turns
- Dice rolling managed via semaphores
- Mutex-protected board updates to prevent race conditions
- Token capturing and safe zones
- Game-over detection and automatic winner declaration
- Inactivity handling (players are kicked after 20 turns without a six)
- Hit rate and position stats after the game ends

---


