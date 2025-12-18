# Escape Room Game (Linked List Based)

## 1. Introduction

This project is a console-based **Escape Room Game** implemented as a requirement for the *Data Structures and Algorithms* course. The main objective of the game is to allow the player to navigate through a maze of rooms, solve puzzles (clues), and finally escape through one of the exit rooms.

The core focus of this project is the correct and efficient use of **Linked Lists** to represent the entire game map, room connections, and player navigation history.

---

## 2. Project Objectives

* Apply **Linked List data structures** in a practical project.
* Design a dynamic escape room maze with multiple paths.
* Implement puzzles with attempts, time limits, and hints.
* Use randomization to ensure a different experience every run.
* Demonstrate clean code structure and proper memory management.

---

## 3. Data Structures Used

### 3.1 Room Node (Linked List)

Each room in the game is represented as a node in a linked list structure.

```cpp
struct Room {
    int roomID;
    string roomType;      // ENTRANCE / INTERMEDIATE / EXIT
    string difficulty;    // EASY / HARD (INTERMEDIATE only)

    Room* next1;          // First door
    Room* next2;          // Second door (EASY only)
    Room* prev;           // Previous room (for back navigation)

    Clue clues[2];        // One clue per door
    int clueCount;

    bool visited;
    bool cleared;
};
```

All room connections are done **only using pointers**, without using arrays or STL containers for navigation.

---

### 3.2 Clue Structure

Each door requires solving a clue. Clues are stored in a fixed-size array (Clue Bank).

```cpp
struct Clue {
    ClueType type;        // MCQ or TEXT_ANSWER
    string problem;
    string solution;
    string hint;

    string options[4];    // For MCQ
    char correctOption;

    int attempts;
    int points;
    int timeLimit;
    bool usedHint;

    ClueDifficulty diffTag; // EASY / HARD / ANY
};
```

---

### 3.3 History Stack (Linked List)

To support the **Back** feature, a stack implemented using a linked list is used.

```cpp
struct PathNode {
    Room* r;
    PathNode* next;
};
```

This allows the player to return to previously visited rooms safely.

---

## 4. Game Map Design

### 4.1 Room Types

* **Entrance Rooms (4)**
  Each entrance has one door and one clue.

* **Intermediate Rooms**

  * EASY: Two doors, each with a separate clue.
  * HARD: One door with one clue.

* **Exit Rooms (2)**
  Final rooms that contain the last puzzle required to escape.

---

### 4.2 Map Construction

* Rooms are dynamically created using `new`.
* Connections between rooms are done using `next1` and `next2` pointers.
* Some paths merge together to form a complex maze.
* EASY room doors are randomized by swapping pointers and clues.

---

## 5. Randomization Logic

Randomization is applied in multiple aspects:

* Random selection of clues from the clue bank.
* Difficulty-based clue assignment (EASY / HARD).
* Random swapping of EASY room doors.

This ensures that each game run provides a unique experience.

---

## 6. Clue Solving Mechanics

Each clue includes:

* **Limited Attempts**: The player has a fixed number of trials.
* **Time Limit**: If time expires, the attempt is considered wrong.
* **Hint System**: The player may request one hint per clue.

### Hint Penalty

* Using a hint reduces the total score by **5 points**.

### Scoring Rules

* Correct answer → points added.
* Wrong answer → attempts reduced.
* Hint usage → score penalty.

---

## 7. Gameplay Flow

1. Player chooses one of the four entrance rooms.
2. Player solves the entrance clue.
3. Player navigates through intermediate rooms:

   * EASY → choose between two doors.
   * HARD → single door.
4. Player may go back using the history stack.
5. Player reaches one of the exit rooms.
6. Final puzzle is solved.
7. Game ends and final score is displayed.

---

## 8. Memory Management

* All rooms are allocated dynamically using `new`.
* Before program termination, all allocated memory is released using `delete`.
* History stack is also freed to prevent memory leaks.

---

## 9. How to Run the Project (Execution Steps)

Follow these steps to compile and run the Escape Room Game:

### Step 1: Prepare the Environment

* Install a C++ compiler (such as **g++** or **Visual Studio**).
* Make sure the compiler supports **C++11 or later**.

### Step 2: Open the Project

* Open the `.cpp` source file containing the project code.
* Ensure all code is inside a single source file (as required by the project).

### Step 3: Compile the Program

If you are using a terminal with g++:

```bash
g++ EscapeRoom.cpp -o EscapeRoom
```

If you are using **Visual Studio**:

* Create a new *Console Application* project.
* Paste the source code into the main `.cpp` file.
* Build the project.

### Step 4: Run the Program

From the terminal:

```bash
./EscapeRoom
```

Or press **Run** inside your IDE.

### Step 5: Playing the Game

1. Choose one of the four entrance rooms (1–4).
2. Solve the displayed clue to open doors.
3. Navigate through intermediate rooms:

   * EASY rooms → choose between two doors.
   * HARD rooms → only one door.
4. Use `H` to request a hint (score penalty applies).
5. Use `0` to go back to the previous room.
6. Reach an exit room and solve the final puzzle to escape.

---

## 10. User Experience

* Clear console instructions.
* Validated user input.
* Informative messages for correct and wrong answers.
* Score displayed continuously.

---

## 10. Conclusion

This project successfully demonstrates the practical use of **Linked Lists** in building a dynamic game environment. The Escape Room Game combines data structures, algorithmic thinking, and game logic into a complete and interactive console application.

The project meets all the course requirements and includes additional features such as time-limited clues, a hint system, score penalties, and navigation history, making it both educational and engaging.
