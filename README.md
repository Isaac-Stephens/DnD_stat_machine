# DnD Stat Machine

**Repository:** [DnD Stat Machine - GitHub](https://github.com/Isaac-Stephens/DnD_stat_machine)  
**Developer:** Isaac Stephens  
**Language:** C++  
**GUI Library:** Dear ImGui  
**Graphics API:** Vulkan  

## Overview
DnD Stat Machine is a C++ application designed to streamline Dungeons & Dragons character creation and stat generation. The project leverages **Dear ImGui** for a user-friendly graphical interface and **Vulkan** for rendering. It provides an intuitive way to roll dice, assign attributes, and manage character data efficiently.

_This project is still in early development._


## Features
- **Dice Rolling System** – Implements various D&D dice mechanics via `dice.h` and `dice.cpp`.
- **Character Stat Management** – Future integration of race-based stats from JSON files.
- **Graphical User Interface** – Utilizes Dear ImGui for interactive stat allocation and customization.
- **Modular Design** – Structured with a dedicated `imgui/` directory for UI components and `src/` for core logic.

## Project Structure
- **`imgui/`** – Contains the Dear ImGui source files for UI implementation.
- **`src/`** – Houses the main application logic, including:
  - `main.cpp` – Entry point of the application.
  - `dice.h` / `dice.cpp` – Handles D&D dice mechanics.
- **Planned JSON Integration** – Future development will add JSON support for race-based stat bonuses.

## Future Enhancements
- **Race & Class Selection** – Implement a JSON-based system for dynamically loading race and class data.
- **Save & Load Character Data** – Enable saving character builds in a structured format.
- **Expanded UI Features** – Add graphical elements for better visualization of stats and dice rolls.

## Installation & Usage
1. Clone the repository:
   ```sh
   git clone https://github.com/Isaac-Stephens/DnD_stat_machine.git
   ```
2. Navigate to the project directory:
   ```sh
   cd DnD_stat_machine
   ```
3. Build the project (ensure you have a C++ compiler, Vulkan SDK, and dependencies installed):
   ```sh
   make
   ```
4. Run the application:
   ```sh
   ./downndirtyDnD
   ```

## Contributions
Contributions are welcome! Feel free to fork the repository, create a feature branch, and submit a pull request. However this is a small passion project for fun so I work on it when I can.

## License
This project is licensed under the MIT License and the Open Gaming License. See `LEGAL` for more details.

