# Watchdog - Process Reviver

[![License](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![GitHub last commit](https://img.shields.io/github/last-commit/username/watchdog.svg)](https://github.com/username/watchdog/commits/master)

Watchdog is a C program that attaches to a process and revives it if it fails for any reason. It monitors the process and sends signals to keep it alive. If the process stops responding or crashes, the watchdog automatically restarts it to ensure continuous operation.

## Table of Contents

- [Watchdog - Process Reviver](#watchdog---process-reviver)
  - [Introduction](#introduction)
  - [Features](#features)
  - [Getting Started](#getting-started)
    - [Prerequisites](#prerequisites)
    - [Installation](#installation)
  - [Usage](#usage)
  - [Contributing](#contributing)
  - [License](#license)
  - [Contact](#contact)

## Introduction

Watchdog is a utility designed to monitor a critical process and ensure its uninterrupted operation. It acts as a safety mechanism by periodically sending signals to the target process and checking its responsiveness. If the process fails to respond or terminates unexpectedly, the watchdog automatically restarts it to maintain system stability.

## Features

- Monitors a specified process and revives it if it stops responding or crashes.
- Configurable interval and threshold for sending signals and reviving the process.
- User-friendly command-line interface to set up the watchdog parameters.
- Implemented using POSIX threads, signals, and semaphores for efficient and reliable operation.
- Handles graceful shutdown of the watchdog process.

## Getting Started

### Prerequisites

To build and run the watchdog program, you need the following:

- C compiler (e.g., GCC)
- POSIX-compatible operating system (e.g., Linux, macOS)

### Installation

1. Clone the repository:

$ git clone https://github.com/username/watchdog.git
$ cd watchdog
Compile the watchdog library and binaries:
$ make library
$ make
Execute the watchdog and user program:
$ ./watchdog.out [options] -- ./user_program.out [user_program_args]
options: Specify the interval and threshold for monitoring the user program by the watchdog.
user_program_args: The arguments for the user program.
Example usage:

$ ./watchdog.out -i 5 -t 3 -- ./user_program.out arg1 arg2

### Watchdog and User Process Interaction
The watchdog program and the user process have an interactive relationship. The watchdog continuously monitors the user process by sending periodic signals to check its responsiveness. If the user process fails to respond or terminates unexpectedly, the watchdog automatically revives it to maintain system stability.

Moreover, the user process also monitors the watchdog to ensure that it is functioning correctly. In the event of a watchdog failure, the user process can take appropriate actions, such as restarting the watchdog or alerting the system administrator.

Using signals for communication between the watchdog and the user process allows for efficient and lightweight monitoring, ensuring that the system remains robust and resilient to failures.
