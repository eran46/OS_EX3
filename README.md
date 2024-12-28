
# HW3: Chat Client/Server

## Project Overview
This repository contains a client/server chat application implemented in Linux using the TCP protocol. 
The server accepts multiple client connections, allowing them to broadcast and whisper messages.

## Branches
- **`dev1-server`**: For Dev1 to implement and refine server-related functionality.
- **`dev2-client`**: For Dev2 to implement and refine client-related functionality.
- **`main`**: Used for stable, tested code and final submission.

## Collaboration Guidelines
1. Each developer works in their designated branch (`dev1-server` or `dev2-client`) for development tasks.
2. Use clear and descriptive commit messages for every change.
3. Push changes regularly to avoid conflicts.
4. Before merging to `main`, create a pull request and assign the other developer for review.
5. Resolve conflicts during code integration collaboratively.
6. Conduct pair programming or debugging sessions during the integration phase.

## Development Workflow
1. Clone the repository and create the branches `dev1-server` and `dev2-client`.
2. Switch to your branch using `git checkout` and start implementing tasks as per the schedule.
3. Push changes to your branch and keep it up to date with `main` using `git pull`.
4. After completing development, test thoroughly and initiate a pull request for integration.
5. Collaborate on debugging and testing on the `main` branch.

## Makefile
The `Makefile` is created at the end of development for building the executables `hw3server` and `hw3client`.