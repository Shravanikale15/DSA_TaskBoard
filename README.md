# 🗂️ DSA_TaskBoard

A C++ Task Management System built using core data structures like `vector`, `stack`, `enum`, and `priority_queue`.  
Manage your tasks efficiently with support for typed fields, real-time alerts, smart scheduling, and persistent CSV storage — all presented through a structured and user-friendly terminal interface.

---

## 🚀 Features

- **Full CRUD operations** – Add, update, and delete tasks
- **Typed columns** – Supports `int`, `float`, `string`, `bool`, and more
- **Filtering with Undo** – Stack-based undo for filters
- **Sorting** – Sort tasks by any column or attribute
- **Custom columns** – Dynamically add fields to suit your needs
- **Deadline-based alerts** – Categorized warnings for upcoming tasks
- **Priority scheduling** – Automatically arrange tasks based on urgency and importance
- **CSV Import/Export** – Persistent storage of your task data
- **Clean terminal UI** – Uses `setw` for structured, readable output

---

## 🛠️ Tech Stack

- **Language:** C++
- **Core DSA Used:**
  - `vector` – dynamic rows and columns
  - `stack` – filter undo history
  - `priority_queue` – smart task scheduling
  - `enum` – strong typed cell structure
- **File Handling:** CSV-based read/write
- **Terminal UI:** Built using `iomanip` and `windows.h` (for UTF-8 support)

---

## 🖥️ How to Run

1. Clone the repository  
   ```bash
   git clone https://github.com/Shravanikale15/DSA_TaskBoard.git
   cd DSA_TaskBoard
   ```

2. Compile the code using g++ (or any C++ compiler)
   ```bash
   g++ -o ToDoList ToDoList.cpp
   ```

3. Run the executable
   ```bash
   ./ToDoList  # Or `ToDoList.exe` on Windows
   ```

## Trying it out

There's a `Syllabus.csv` in here with some sample tasks — load it through the "Load from CSV" option in the menu so you can see how everything works without typing in tasks by hand first.

## Project files
```
├── ToDoList.cpp   # everything lives in this one file
└── Syllabus.csv     # sample data to load and play with
```
