#include <cstdlib>
#include <ctime>
#include <iostream>
#include <limits>
#include <string>

using namespace std;

/* =========================
CONFIG
========================= */
static const int MAX_OPTIONS = 4;
static const int MAX_CLUES_PER_ROOM = 2;
static const int DEFAULT_ATTEMPTS = 3;
static const int HINT_PENALTY = 5;
static const int WRONG_PENALTY = 10;

/* =========================
CLUE / PUZZLE
========================= */
enum ClueType { TEXT_ANSWER, MCQ };
enum ClueDifficulty { EASY_CLUE, HARD_CLUE, ANY_CLUE };
struct Clue {
  ClueType type;
  string problem;
  string solution;
  string hint;

  string options[MAX_OPTIONS];
  char correctOption;

  int attempts;
  int points;
  int timeLimit; // seconds (0 = no limit)
  bool usedHint;

  ClueDifficulty diffTag;
};

static inline string toLowerStr(string s) {
  for (size_t i = 0; i < s.size(); i++) {
    if (s[i] >= 'A' && s[i] <= 'Z')
      s[i] = char(s[i] - 'A' + 'a');
  }
  return s;
}

static inline void flushInputLine() {
  cin.clear();
  cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

static inline bool isChoiceChar(char c) {
  c = (char)toupper((unsigned char)c);
  return (c == 'A' || c == 'B' || c == 'C' || c == 'D');
}

/* =========================
ROOM NODE (LINKED LIST)
========================= */
struct Room {
  int roomID;
  string roomType;   // ENTRANCE / INTERMEDIATE / EXIT / TRAP (optional)
  string difficulty; // EASY / HARD (for intermediate)

  Room *next1; // door 1
  Room *next2; // door 2 (only for EASY)
  Room *prev;  // for back

  Clue clues[MAX_CLUES_PER_ROOM]; // one per door
  int clueCount;                  // 1 or 2

  bool visited;
  bool cleared;
};

/* =========================
HELPERS: Create rooms
========================= */
Room *createRoom(int id, const string &type, const string &diff) {
  Room *r = new Room;
  r->roomID = id;
  r->roomType = type;
  r->difficulty = diff;
  r->next1 = nullptr;
  r->next2 = nullptr;
  r->prev = nullptr;
  r->clueCount = 0;
  r->visited = false;
  r->cleared = false;
  return r;
}

/* =========================
BUILD SAMPLE CLUE BANK
(No STL containers: fixed arrays)
========================= */
static const int CLUE_BANK_SIZE = 53;
static const int FINAL_CLUE_INDEX = CLUE_BANK_SIZE - 1; // = 52
Clue CLUE_BANK[CLUE_BANK_SIZE];
bool USED_CLUES[FINAL_CLUE_INDEX];

void resetUsedClues() {
  for (int i = 0; i < FINAL_CLUE_INDEX; i++)
    USED_CLUES[i] = false;
}

    void initClueBank() {
        CLUE_BANK[0] = { MCQ,
            "What does CPU stand for?",
            "", "It's the main processor of the computer.",
            {"Central Processing Unit","Computer Processing User","Central Program Utility","Core Power Unit"},
            'A', DEFAULT_ATTEMPTS, 10, 20, false,
            EASY_CLUE
        };

        CLUE_BANK[1] = { MCQ,
            "Which planet is known as the Red Planet?",
            "", "It looks reddish from space.",
            {"Mercury","Venus","Earth","Mars"},
            'D', DEFAULT_ATTEMPTS, 10, 20, false,
            EASY_CLUE
        };

        CLUE_BANK[2] = { TEXT_ANSWER,
            "Type the word: stack",
            "stack", "It's a data structure: LIFO.",
            {"","","",""},
            'A', DEFAULT_ATTEMPTS, 10, 20, false,
            ANY_CLUE
        };

        CLUE_BANK[3] = { TEXT_ANSWER,
            "What is the chemical formula of water?",
            "h2o", "Hydrogen + Oxygen.",
            {"","","",""},
            'A', DEFAULT_ATTEMPTS, 10, 20, false,
            EASY_CLUE
        };

        CLUE_BANK[4] = { MCQ,
            "In C++, which symbol ends a statement?",
            "", "End of line in code.",
            {":",";",",","."},
            'B', DEFAULT_ATTEMPTS, 10, 20, false,
            EASY_CLUE
        };

        CLUE_BANK[5] = { MCQ,
            "Which data structure follows LIFO?",
            "", "Last In First Out.",
            {"Queue","Array","Stack","Tree"},
            'C', DEFAULT_ATTEMPTS, 10, 20, false,
            HARD_CLUE
        };

        CLUE_BANK[6] = { TEXT_ANSWER,
            "What keyword allocates memory in C++? (one word)",
            "new", "Used with pointers.",
            {"","","",""},
            'A', DEFAULT_ATTEMPTS, 10, 20, false,
            ANY_CLUE
        };

        CLUE_BANK[7] = { MCQ,
            "Which number is even?",
            "", "Divisible by 2.",
            {"9","14","21","35"},
            'B', DEFAULT_ATTEMPTS, 10, 20, false,
            EASY_CLUE
        };

        CLUE_BANK[8] = { MCQ,
            "What is 7 + 8?",
            "", "Simple addition.",
            {"12","13","15","16"},
            'C', DEFAULT_ATTEMPTS, 10, 20, false,
            EASY_CLUE
        };

        CLUE_BANK[9] = { TEXT_ANSWER,
            "Enter the password: SUT",
            "sut", "It is your university initials.",
            {"","","",""},
            'A', DEFAULT_ATTEMPTS, 10, 20, false,
            EASY_CLUE
        };

        CLUE_BANK[10] = { MCQ,
            "Which protocol is used for routing inside an AS? (Common answer)",
            "", "Think OSPF/RIP/EIGRP.",
            {"HTTP","OSPF","FTP","SMTP"},
            'B', DEFAULT_ATTEMPTS, 10, 20, false,
            HARD_CLUE
        };

        CLUE_BANK[11] = { MCQ,
            "Which operator is used to assign a value to a variable in C++?",
            "", "It stores a value inside a variable.",
            {"==","=","!=","<="},
            'B', DEFAULT_ATTEMPTS, 10, 20, false,
            EASY_CLUE
        };

        CLUE_BANK[12] = { MCQ,
            "Which keyword is used to define a class in C++?",
            "", "It defines user-defined data types.",
            {"struct","class","define","object"},
            'B', DEFAULT_ATTEMPTS, 10, 20, false,
            EASY_CLUE
        };

        CLUE_BANK[13] = { MCQ,
            "Which symbol is used to end a statement in C++?",
            "", "Every statement must end with it.",
            {":",";",".",","},
            'B', DEFAULT_ATTEMPTS, 10, 20, false,
            EASY_CLUE
        };

        CLUE_BANK[14] = { MCQ,
            "Which header is required for input and output in C++?",
            "", "Used with cin and cout.",
            {"<stdio.h>","<iostream>","<conio.h>","<stdlib.h>"},
            'B', DEFAULT_ATTEMPTS, 10, 20, false,
            EASY_CLUE
        };

        CLUE_BANK[15] = { MCQ,
            "Which keyword is used to create an object in C++?",
            "", "Used with classes.",
            {"malloc","new","create","object"},
            'B', DEFAULT_ATTEMPTS, 10, 20, false,
            HARD_CLUE
        };

        CLUE_BANK[16] = { MCQ,
            "Which operator is used to access class members?",
            "", "Used with objects.",
            {".","->","::","*"},
            'A', DEFAULT_ATTEMPTS, 10, 20, false,
            EASY_CLUE
        };

        CLUE_BANK[17] = { MCQ,
            "What is the correct return type of main()?",
            "", "Standard C++ requires it.",
            {"void","int","float","char"},
            'B', DEFAULT_ATTEMPTS, 10, 20, false,
            EASY_CLUE
        };

        CLUE_BANK[18] = { MCQ,
            "Which loop is guaranteed to run at least once?",
            "", "Condition is checked after execution.",
            {"for","while","do-while","foreach"},
            'C', DEFAULT_ATTEMPTS, 10, 20, false,
            EASY_CLUE
        };

        CLUE_BANK[19] = { MCQ,
            "Which operator is used for logical AND?",
            "", "Returns true or false.",
            {"&","&&","|","||"},
            'B', DEFAULT_ATTEMPTS, 10, 20, false,
            EASY_CLUE
        };

        CLUE_BANK[20] = { MCQ,
            "Which keyword is used to inherit a class?",
            "", "Used after class name.",
            {"extends","inherits",":","->"},
            'C', DEFAULT_ATTEMPTS, 10, 20, false,
            HARD_CLUE
        };

        CLUE_BANK[21] = { MCQ,
            "Which data type is used to store true or false?",
            "", "Introduced in C++.",
            {"int","bool","char","float"},
            'B', DEFAULT_ATTEMPTS, 10, 20, false,
            EASY_CLUE
        };

        CLUE_BANK[22] = { MCQ,
            "Which keyword is used to define a constant?",
            "", "Value cannot be changed.",
            {"static","final","const","define"},
            'C', DEFAULT_ATTEMPTS, 10, 20, false,
            EASY_CLUE
        };

        CLUE_BANK[23] = { MCQ,
            "Which access specifier allows access anywhere?",
            "", "Most open level.",
            {"private","protected","public","static"},
            'C', DEFAULT_ATTEMPTS, 10, 20, false,
            EASY_CLUE
        };

        CLUE_BANK[24] = { MCQ,
            "Which keyword is used to allocate memory dynamically?",
            "", "Works with heap.",
            {"alloc","malloc","new","create"},
            'C', DEFAULT_ATTEMPTS, 10, 20, false,
            HARD_CLUE
        };

        CLUE_BANK[25] = { MCQ,
            "Which operator is used to compare equality?",
            "", "Used in conditions.",
            {"=","==","!=","<="},
            'B', DEFAULT_ATTEMPTS, 10, 20, false,
            EASY_CLUE
        };

        CLUE_BANK[26] = { MCQ,
            "Which statement is used to exit a loop?",
            "", "Stops execution immediately.",
            {"stop","end","break","exit"},
            'C', DEFAULT_ATTEMPTS, 10, 20, false,
            EASY_CLUE
        };

        CLUE_BANK[27] = { MCQ,
            "Which keyword is used to return a value from function?",
            "", "Ends function execution.",
            {"send","output","return","break"},
            'C', DEFAULT_ATTEMPTS, 10, 20, false,
            EASY_CLUE
        };

        CLUE_BANK[28] = { MCQ,
            "Which container stores elements in sequence?",
            "", "Part of STL.",
            {"map","set","vector","queue"},
            'C', DEFAULT_ATTEMPTS, 10, 20, false,
            HARD_CLUE
        };

        CLUE_BANK[29] = { MCQ,
            "Which keyword is used to include libraries?",
            "", "Starts with #.",
            {"import","using","#include","#define"},
            'C', DEFAULT_ATTEMPTS, 10, 20, false,
            EASY_CLUE
        };

        CLUE_BANK[30] = { MCQ,
            "Which symbol is used for single-line comments?",
            "", "Ignored by compiler.",
            {"/*","*/","//","#"},
            'C', DEFAULT_ATTEMPTS, 10, 20, false,
            EASY_CLUE
        };

        CLUE_BANK[31] = { MCQ,
            "Which data type stores decimal numbers?",
            "", "Has floating point.",
            {"int","char","float","bool"},
            'C', DEFAULT_ATTEMPTS, 10, 20, false,
            EASY_CLUE
        };

        CLUE_BANK[32] = { MCQ,
            "Which loop is best when number of iterations is known?",
            "", "Has initialization.",
            {"while","do-while","for","loop"},
            'C', DEFAULT_ATTEMPTS, 10, 20, false,
            EASY_CLUE
        };

        CLUE_BANK[33] = { MCQ,
            "Which keyword makes a variable shared across objects?",
            "", "Belongs to class.",
            {"const","global","static","shared"},
            'C', DEFAULT_ATTEMPTS, 10, 20, false,
            HARD_CLUE
        };

        CLUE_BANK[34] = { MCQ,
            "Which operator is used to access pointer members?",
            "", "Used with objects via pointers.",
            {".","::","->","*"},
            'C', DEFAULT_ATTEMPTS, 10, 20, false,
            HARD_CLUE
        };

        CLUE_BANK[35] = { MCQ,
            "Which keyword is used to free dynamic memory?",
            "", "Opposite of new.",
            {"free","delete","remove","clear"},
            'B', DEFAULT_ATTEMPTS, 10, 20, false,
            HARD_CLUE
        };

        CLUE_BANK[36] = { MCQ,
            "Which function is program entry point?",
            "", "Execution starts here.",
            {"start()","run()","main()","init()"},
            'C', DEFAULT_ATTEMPTS, 10, 20, false,
            EASY_CLUE
        };

        CLUE_BANK[37] = { MCQ,
            "Which data type holds a single character?",
            "", "Uses single quotes.",
            {"string","char","text","bool"},
            'B', DEFAULT_ATTEMPTS, 10, 20, false,
            EASY_CLUE
        };

        CLUE_BANK[38] = { MCQ,
            "Which keyword avoids name conflicts?",
            "", "Used with std.",
            {"using","scope","namespace","define"},
            'C', DEFAULT_ATTEMPTS, 10, 20, false,
            HARD_CLUE
        };

        CLUE_BANK[39] = { MCQ,
            "Which operator increases value by one?",
            "", "Increment operator.",
            {"+=","++","--","+="},
            'B', DEFAULT_ATTEMPTS, 10, 20, false,
            EASY_CLUE
        };

        CLUE_BANK[40] = { MCQ,
            "Which STL container stores key-value pairs?",
            "", "Keys are unique.",
            {"vector","list","map","array"},
            'C', DEFAULT_ATTEMPTS, 10, 20, false,
            HARD_CLUE
        };

        CLUE_BANK[41] = { MCQ,
            "Which keyword enables polymorphism?",
            "", "Used with functions.",
            {"static","inline","virtual","override"},
            'C', DEFAULT_ATTEMPTS, 10, 20, false,
            HARD_CLUE
        };

        CLUE_BANK[42] = { MCQ,
            "Which operator is used for OR logic?",
            "", "Returns true if one is true.",
            {"|","||","&","&&"},
            'B', DEFAULT_ATTEMPTS, 10, 20, false,
            EASY_CLUE
        };

        CLUE_BANK[43] = { MCQ,
            "Which function prints output?",
            "", "Uses stream insertion.",
            {"cin","print","cout","output"},
            'C', DEFAULT_ATTEMPTS, 10, 20, false,
            EASY_CLUE
        };

        CLUE_BANK[44] = { MCQ,
            "Which keyword is used to define macros?",
            "", "Preprocessor directive.",
            {"#macro","#define","#include","#ifdef"},
            'B', DEFAULT_ATTEMPTS, 10, 20, false,
            HARD_CLUE
        };

        CLUE_BANK[45] = { MCQ,
            "Which concept allows same function name with different parameters?",
            "", "Compile-time polymorphism.",
            {"Overriding","Inheritance","Overloading","Abstraction"},
            'C', DEFAULT_ATTEMPTS, 10, 20, false,
            HARD_CLUE
        };

        CLUE_BANK[46] = { MCQ,
            "Which keyword hides implementation details?",
            "", "OOP principle.",
            {"Inheritance","Encapsulation","Polymorphism","Abstraction"},
            'D', DEFAULT_ATTEMPTS, 10, 20, false,
            HARD_CLUE
        };

        CLUE_BANK[47] = { MCQ,
            "Which operator is used for address of a variable?",
            "", "Returns memory location.",
            {"*","&","->","%"},
            'B', DEFAULT_ATTEMPTS, 10, 20, false,
            HARD_CLUE
        };

        CLUE_BANK[48] = { MCQ,
            "Which keyword is used to handle exceptions?",
            "", "Used with try.",
            {"catch","throw","error","handle"},
            'A', DEFAULT_ATTEMPTS, 10, 20, false,
            HARD_CLUE
        };

        CLUE_BANK[49] = { MCQ,
            "Which function generates random numbers?",
            "", "Needs <cstdlib>.",
            {"random()","rand()","srand()","generate()"},
            'B', DEFAULT_ATTEMPTS, 10, 20, false,
            EASY_CLUE
        };

        CLUE_BANK[50] = { MCQ,
            "Which keyword makes a function not modify data?",
            "", "Used after function.",
            {"final","const","static","virtual"},
            'B', DEFAULT_ATTEMPTS, 10, 20, false,
            HARD_CLUE
        };

        CLUE_BANK[51] = { MCQ,
            "Which data type is best for large integers?",
            "", "Holds bigger values.",
            {"int","short","long long","float"},
            'C', DEFAULT_ATTEMPTS, 10, 20, false,
            HARD_CLUE
        };

        // Final (index 52)
        CLUE_BANK[52] = { TEXT_ANSWER,
        "Final Gate: who is the best Data Structure doctor?",
        "Dr. Mohamed Ali, Eng. Aya Abdelnabi", "She teaches Data Structures.",
        {"","","",""},
        'A', DEFAULT_ATTEMPTS, 15, 15, false,
        ANY_CLUE
    };
    }
/* Pick a random clue from bank and copy it
- HARD rooms: pick HARD_CLUE أو ANY_CLUE
- غير كده: pick EASY_CLUE أو ANY_CLUE
*/

int pickRandomClueIndexForRoom(const string &roomType,
                               const string &roomDifficulty,
                               bool wantFinal = false) {
  if (wantFinal)
    return FINAL_CLUE_INDEX;

  bool wantHard = (roomType == "INTERMEDIATE" && roomDifficulty == "HARD");

  int valid[FINAL_CLUE_INDEX]; // 0..51
  int cnt = 0;

  for (int i = 0; i < FINAL_CLUE_INDEX; i++) {
    if (USED_CLUES[i])
      continue;

    if (wantHard) {
      if (CLUE_BANK[i].diffTag == HARD_CLUE || CLUE_BANK[i].diffTag == ANY_CLUE)
        valid[cnt++] = i;
    } else {
      if (CLUE_BANK[i].diffTag == EASY_CLUE || CLUE_BANK[i].diffTag == ANY_CLUE)
        valid[cnt++] = i;
    }
  }

  if (cnt == 0) {
    for (int i = 0; i < FINAL_CLUE_INDEX; i++)
      if (!USED_CLUES[i])
        valid[cnt++] = i;
  }

  if (cnt == 0)
    return rand() % FINAL_CLUE_INDEX;

  return valid[rand() % cnt];
}
Clue pickRandomClueForRoom(const string &roomType, const string &roomDifficulty,
                           bool wantFinal = false) {
  if (wantFinal) {
    Clue c = CLUE_BANK[FINAL_CLUE_INDEX];
    c.usedHint = false;
    c.attempts = DEFAULT_ATTEMPTS;
    return c;
  }

  int idx = pickRandomClueIndexForRoom(roomType, roomDifficulty, false);

  USED_CLUES[idx] = true;

  Clue c = CLUE_BANK[idx];
  c.usedHint = false;
  c.attempts = DEFAULT_ATTEMPTS;
  return c;
}

/* =========================
PRINT ROOM INFO
========================= */
void printRoom(const Room *r, int score) {
  cout << "\n========================\n";
  cout << "Score: " << score << "\n";
  cout << "You are in Room ID: " << r->roomID << "\n";
  cout << "Type: " << r->roomType;
  if (r->roomType == "INTERMEDIATE") {
    cout << " (" << r->difficulty << ")";
  }
  cout << "\n\nDoors:\n";

  // EXIT: door 1 is final puzzle (not navigation)
  if (r->roomType == "EXIT") {
    cout << "  1) Final Door (solve to escape)\n";
  } else if (r->clueCount == 1) {
    cout << "  1) Door 1 -> ";
    if (r->next1)
      cout << "Room " << r->next1->roomID << "\n";
    else
      cout << "[NONE]\n";
  } else {
    cout << "  1) Door 1 -> "
         << (r->next1 ? ("Room " + to_string(r->next1->roomID)) : "[NONE]")
         << "\n";
    cout << "  2) Door 2 -> "
         << (r->next2 ? ("Room " + to_string(r->next2->roomID)) : "[NONE]")
         << "\n";
  }
  cout << "\n\n[Abilities]:\n";
  cout << "  0) << RETURN TO PREVIOUS ROOM (Undo Move)\n";
  cout << "  9) Quit Game\n";
  cout << "========================\n";
}

/* =========================
SOLVE A CLUE (with hint)
returns: true if solved
========================= */
bool solveClue(Clue &clue, int &score) {
  cout << "\n--- Puzzle ---\n";
  cout << clue.problem << "\n\n";

  if (clue.type == MCQ) {
    cout << "A) " << clue.options[0] << "\n";
    cout << "B) " << clue.options[1] << "\n";
    cout << "C) " << clue.options[2] << "\n";
    cout << "D) " << clue.options[3] << "\n\n";
  }
  time_t startTime = time(nullptr);
  while (clue.attempts > 0) {
    cout << "(Attempts: " << clue.attempts << ")\n";
    cout << "Your answer";
    if (clue.type == MCQ)
      cout << " (A/B/C/D)";
    cout << " or H for hint: ";

    string input;
    getline(cin, input);
    if (input.size() == 0)
      continue;

    if (clue.timeLimit > 0) {
      int elapsed = (int)(time(nullptr) - startTime);
      if (elapsed > clue.timeLimit) {
        clue.attempts--;
        cout << "Time out! Wrong.\n";
        startTime = time(nullptr);
        continue;
      }
    }

    // Hint
    if (input.size() == 1 && (input[0] == 'H' || input[0] == 'h')) {
      if (!clue.usedHint) {
        clue.usedHint = true;
        score -= HINT_PENALTY;
        cout << "Hint (-" << HINT_PENALTY << "): " << clue.hint << "\n";
      } else {
        cout << "Hint already used.\n";
      }
      startTime = time(nullptr);
      continue;
    }

    bool correct = false;

    if (clue.type == MCQ) {
      char c = (char)toupper((unsigned char)input[0]);
      if (!isChoiceChar(c)) {
        cout << "Invalid choice. Enter A/B/C/D or H.\n";
        continue;
      }
      correct = (c == clue.correctOption);
    } else {
      string ans = toLowerStr(input);
      correct = (ans == toLowerStr(clue.solution));
    }

    if (correct) {
      cout << "Correct!\n";
      score += clue.points;
      return true;
    } else {
      clue.attempts--;
      cout << "Wrong.\n";
    }
  }

  cout << "Door remains LOCKED. (No attempts left)\n";
  cout << "You are trapped inside the game!\n";
  return false;
}

/* =========================
RANDOMIZE EASY ROOM DOORS
(swap next1/next2 + swap their clues)
========================= */
void randomizeEasyDoors(Room *r) {
  if (!r || r->clueCount != 2)
    return;
  if (rand() % 2 == 0) {
    Room *t = r->next1;
    r->next1 = r->next2;
    r->next2 = t;

    Clue tc = r->clues[0];
    r->clues[0] = r->clues[1];
    r->clues[1] = tc;
  }
}
void shuffleRooms(Room **arr, int n) {
  for (int i = n - 1; i > 0; --i) {
    int j = rand() % (i + 1);
    Room *tmp = arr[i];
    arr[i] = arr[j];
    arr[j] = tmp;
  }
}

/* =========================
BUILD MAP (Linked nodes)
========================= */
struct GameMap {
  Room *entrances[4];
  Room *exits[2];

  Room *all[20];
  int count;
};

void addToAll(GameMap &gm, Room *r) { gm.all[gm.count++] = r; }

GameMap buildMap() {
  GameMap gm;
  gm.count = 0;

  Room *EN1 = createRoom(1, "ENTRANCE", "");
  Room *EN2 = createRoom(2, "ENTRANCE", "");
  Room *EN3 = createRoom(3, "ENTRANCE", "");
  Room *EN4 = createRoom(4, "ENTRANCE", "");

  Room *I1 = createRoom(5, "INTERMEDIATE", "HARD");
  Room *I2 = createRoom(6, "INTERMEDIATE", "EASY");
  Room *I3 = createRoom(7, "INTERMEDIATE", "EASY");
  Room *I4 = createRoom(8, "INTERMEDIATE", "EASY");
  Room *I5 = createRoom(9, "INTERMEDIATE", "HARD");
  Room *I6 = createRoom(10, "INTERMEDIATE", "HARD");
  Room *I7 = createRoom(11, "INTERMEDIATE", "EASY");
  Room *I8 = createRoom(12, "INTERMEDIATE", "EASY");

  Room *EX1 = createRoom(99, "EXIT", "");
  Room *EX2 = createRoom(100, "EXIT", "");

  addToAll(gm, EN1);
  addToAll(gm, EN2);
  addToAll(gm, EN3);
  addToAll(gm, EN4);
  addToAll(gm, I1);
  addToAll(gm, I2);
  addToAll(gm, I3);
  addToAll(gm, I4);
  addToAll(gm, I5);
  addToAll(gm, I6);
  addToAll(gm, I7);
  addToAll(gm, I8);
  addToAll(gm, EX1);
  addToAll(gm, EX2);

  // Base connections
  EN1->next1 = I1;
  I1->prev = EN1;
  I1->next1 = I2;
  I2->prev = I1;

  EN4->next1 = I3;
  I3->prev = EN4;
  I3->next1 = I4;
  I4->prev = I3;

  EN2->next1 = I5;
  I5->prev = EN2;
  I5->next1 = I6;
  I6->prev = I5;
  I6->next1 = EX2; // prev will be set dynamically when moving

  EN3->next1 = I7;
  I7->prev = EN3;
  I7->next1 = I8;
  I8->prev = I7;
  I8->next1 = I6; // merge

  // Branching for EASY rooms
  I2->next1 = EX1; // ✅ لا نلمس prev هنا
  I2->next2 = I4;

  I3->next2 = I1;

  I4->next1 = EX1;
  I4->next2 = I2; // optional loop

  I7->next2 = I5;

  I8->next2 = EX2;

  // Set entrances/exits
  gm.entrances[0] = EN1;
  gm.entrances[1] = EN2;
  gm.entrances[2] = EN3;
  gm.entrances[3] = EN4;

  gm.exits[0] = EX1;
  gm.exits[1] = EX2;

  // Assign clues per room
  Room *roomsToAssign[] = {EN1, EN2, EN3, EN4, I1, I2,  I3,
                           I4,  I5,  I6,  I7,  I8, EX1, EX2};
  int total = 14;

  for (int i = 0; i < total; i++) {
    Room *r = roomsToAssign[i];
    if (r->roomType == "INTERMEDIATE" && r->difficulty == "EASY") {
      r->clueCount = 2;

      int idx1 = pickRandomClueIndexForRoom(r->roomType, r->difficulty, false);
      USED_CLUES[idx1] = true;

      int idx2 = pickRandomClueIndexForRoom(r->roomType, r->difficulty, false);
      while (idx2 == idx1) { // زيادة أمان (غالبًا مش هتحصل)
        idx2 = pickRandomClueIndexForRoom(r->roomType, r->difficulty, false);
      }
      USED_CLUES[idx2] = true;

      r->clues[0] = CLUE_BANK[idx1];
      r->clues[1] = CLUE_BANK[idx2];

      r->clues[0].usedHint = false;
      r->clues[0].attempts = DEFAULT_ATTEMPTS;
      r->clues[1].usedHint = false;
      r->clues[1].attempts = DEFAULT_ATTEMPTS;
    } else if (r->roomType == "EXIT") {
      r->clueCount = 1;
      r->clues[0] = pickRandomClueForRoom(r->roomType, r->difficulty, true);
    } else {
      r->clueCount = 1;
      r->clues[0] = pickRandomClueForRoom(r->roomType, r->difficulty, false);
    }
  }

  // Randomize easy doors
  randomizeEasyDoors(I2);
  randomizeEasyDoors(I3);
  randomizeEasyDoors(I4);
  randomizeEasyDoors(I7);
  randomizeEasyDoors(I8);

  return gm;
}

void freeMap(GameMap &gm) {
  for (int i = 0; i < gm.count; i++) {
    delete gm.all[i];
    gm.all[i] = nullptr;
  }
  gm.count = 0;
}

/* =========================
GAME LOOP
========================= */
struct PathNode {
  Room *r;
  PathNode *next;
};

void pushPath(PathNode *&top, Room *r) {
  PathNode *n = new PathNode{r, top};
  top = n;
}

Room *popPath(PathNode *&top) {
  if (!top)
    return nullptr;
  PathNode *n = top;
  Room *r = n->r;
  top = n->next;
  delete n;
  return r;
}

void freePath(PathNode *&top) {
  while (top)
    popPath(top);
}

const char *getTrapMessage(Room *current, Room *nextRoom) {
  if (!current || !nextRoom)
    return nullptr;
  if (current->roomID == 7 && nextRoom->roomID == 5) { // I3 -> I1
    return "\n[TRAP TRIGGERED] OH NO! This door was a trap! You have been sent "
           "back to the beginning of the sector!\n";
  } else if (current->roomID == 8 && nextRoom->roomID == 6) { // I4 -> I2
    return "\n[TRAP TRIGGERED] INFINITE LOOP! You are running in circles!\n";
  } else if (current->roomID == 11 && nextRoom->roomID == 9) { // I7 -> I5
    return "\n[TRAP TRIGGERED] HARD PATH! You fell into a high-difficulty "
           "zone!\n";
  }
  return nullptr;
}

int main() {
  srand((unsigned)time(0));
  initClueBank();
  resetUsedClues();
  GameMap gm = buildMap();

  cout << "==== Escape Room Game (Linked List) ====\n";
  cout << "Choose an entrance:\n";
  cout << "  1) EN1\n  2) EN2\n  3) EN3\n  4) EN4\n";
  cout << "Enter choice (1-4): ";

  int start = 0;
  cin >> start;
  flushInputLine();

  if (start < 1 || start > 4) {
    cout << "Invalid. Exiting.\n";
    freeMap(gm);
    return 0;
  }

  Room *current = gm.entrances[start - 1];
  int score = 100;
  PathNode *history = nullptr;
  pushPath(history, current);

  while (true) {
    current->visited = true;

    printRoom(current, score);
    cout << "Enter choice: ";

    int choice;
    cin >> choice;
    flushInputLine();

    // Quit
    if (choice == 9) {
      score = 0;  
      cout << "\n=== You have been kicked out of the game! ===\n";
      cout << "Quitting... Final Score: " << score << "\n";
      break;
    }

    // Back
    // Back (History Stack)
    if (choice == 0) {
      if (history && history->next) {
        popPath(history);     // شيل الحالية
        current = history->r; // ارجع للي قبلها
      } else {
        cout << "No previous room.\n";
      }
      continue;
    }

    // ✅ EXIT room behavior: door 1 solves final puzzle (NOT navigation)
    if (current->roomType == "EXIT") {
      if (choice == 1) {
        bool win = solveClue(current->clues[0], score);
        if (win) {
          cout << "\nYOU ESCAPED! Final Score: " << score << "\n";
          break;
        } else {
          cout << "Final gate locked. You remain at the exit room.\n";
        }
      } else {
        cout << "Invalid door.\n";
      }
      continue;
    }

    // Determine door & next room
    int doorIndex = -1;
    Room *nextRoom = nullptr;

    if (current->clueCount == 1) {
      if (choice != 1) {
        cout << "Invalid door.\n";
        continue;
      }
      doorIndex = 0;
      nextRoom = current->next1;
    } else {
      if (choice != 1 && choice != 2) {
        cout << "Invalid door.\n";
        continue;
      }
      doorIndex = (choice == 1) ? 0 : 1;
      nextRoom = (choice == 1) ? current->next1 : current->next2;
    }

    if (!nextRoom) {
      cout << "This door leads nowhere.\n";
      continue;
    }

    // Solve door puzzle
    bool ok = solveClue(current->clues[doorIndex], score);
    if (!ok) {
      cout << "\n[FAILED] Door Locked! The room mechanism is RESETTING... the "
              "puzzle has changed or reset!\n";
      cout << "PENALTY: -" << WRONG_PENALTY << " pts\n";
      score -= WRONG_PENALTY;
      current->clues[doorIndex].attempts = DEFAULT_ATTEMPTS; // Restore attempts
      // The clues might change if we re-randomized, but current logic just
      // resets attempts
      continue;
    }

    // Check for traps before moving
    const char *trapMessage = getTrapMessage(current, nextRoom);
    if (trapMessage) {
      cout << trapMessage;
      // Depending on trap type, you might want to modify 'nextRoom' or 'score'
      // For now, just display message and proceed to 'nextRoom' as if it's a
      // normal move If the trap is meant to send you back, you'd modify
      // 'nextRoom' here. The current trap messages imply a move, but with a
      // warning.
    }

    pushPath(history, nextRoom);
    current = nextRoom;
  }

  freeMap(gm);
  freePath(history);
  return 0;
}