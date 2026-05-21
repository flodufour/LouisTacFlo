# Ultimate Tic Tac Toe AI

AI implementation for Ultimate Tic Tac Toe featuring advanced search algorithms and modular evaluation systems.

## Project Structure

```text
include/
├── ai/
│   ├── evaluate/
│   │   ├── FeatureEvaluator.h
│   │   └── IEvaluator.h
│   ├── strategy/
│   │   ├── IStrategy.h
│   │   └── MinimaxStrategy.h
│   ├── ArenaHost.h
│   └── GameManager.h
├── core/
│   ├── AIMove.h
│   ├── Cell.h
│   ├── GameState.h
│   ├── SubBoard.h
│   ├── UltimateBoard.h
│   └── WinPatterns.h
├── utils/
│   ├── MoveConverter.h
│   ├── MoveUndo.h
│   └── ZobristHasher.h
└── main.h

src/
├── ai/
│   ├── evaluate/
│   │   └── FeatureEvaluator.cpp
│   ├── strategy/
│   │   └── MinimaxStrategy.cpp
│   ├── ArenaHost.cpp
│   └── GameManager.cpp
├── core/
│   ├── Cell.cpp
│   ├── GameState.cpp
│   ├── SubBoard.cpp
│   └── UltimateBoard.cpp
├── utils/
│   ├── MoveConverter.cpp
│   └── ZobristHasher.cpp
└── main.cpp
```

## Architecture

### **Core Board Architecture**

* **`GameState` & `UltimateBoard`**
    * **Responsibility:** Manages the global 9x9 macro-state.
    * **Tracking:** Keeps real-time track of player turn transitions, valid legal actions, and identifies which specific 3x3 sub-board is currently active for the next move.

* **`SubBoard`**
    * **Responsibility:** Handles isolated 3x3 micro-board logic.
    * **Operations:** Validates local cell allocations, manages piece placements, and executes micro-win/draw detections within its own boundary.

* **Zobrist Hashing**
    * **Responsibility:** Implements a highly optimized incremental 64-bit hashing key system.
    * **Benefit:** Enables lightning-fast game state signatures, allowing instant lookups and caching within the Transposition Table.

## AI System

### Strategy Interface

All decision-making components implement a common interface layout:

```cpp
class IStrategy {
public:
    /**
     * @brief Evaluates the game state and selects the optimal move.
     * @param state The current global game state.
     * @return The chosen AIMove.
     */
    virtual AIMove chooseMove(GameState& state) = 0;

    /**
     * @brief Resets the internal strategy state (e.g., clears transposition tables or history).
     */
    virtual void reset() = 0;

    /**
     * @brief Virtual destructor for safe polymorphic deletion.
     */
    virtual ~IStrategy() = default;
};
```

### Minimax Strategy

The primary competitive engine uses an optimized Minimax search architecture designed to deeply explore variations while minimizing computational waste:

* **Alpha-Beta Pruning**
    * **Mechanism:** Maintains two values throughout the recursive search: `alpha` (the minimum score the maximizing player is assured of) and `beta` (the maximum score the minimizing player is assured of). 
    * **Impact:** As soon as a branch is proven to be worse than a previously examined option, the engine cuts off evaluation for the rest of that subtree, drastically reducing the total number of explored nodes.

* **Iterative Deepening**
    * **Mechanism:** Instead of searching to a fixed target depth immediately, the engine performs a complete search at depth 1, then depth 2, then depth 3, and so on.
    * **Impact:** This approach ensures that the engine always has a fallback "best move" ready if time runs out. Combined with a strict timer check (e.g., stopping if the search exceeds 100ms), it guarantees excellent time management without risking a crash or an incomplete root decision.

* **Transposition Table (TT)**
    * **Mechanism:** A fixed-size array-based hash map consisting of $2^{20}$ ($1,048,576$) entries, indexed via the board's 64-bit Zobrist hash. 
    * **Impact:** In Ultimate Tic-Tac-Toe, different move sequences frequently lead to the exact same board configuration (a transposition). The table caches previously computed exact scores or alpha/beta bounds, allowing the engine to instantly return the result rather than re-searching identical deep subtrees.

* **Move Ordering**
    * **Mechanism:** Before evaluating available moves at any given node, the engine sorts them. It heavily prioritizes the "hint move" fetched from the Transposition Table (the best move from a previous shallower iteration), followed by structural heuristics like center control and corner placement.
    * **Impact:** Searching the best moves first causes the Alpha-Beta bounds to snap shut incredibly early in the loop, maximizing the frequency of branch cut-offs and exponentially increasing search speed.

### Feature Evaluation Pipeline (`FeatureEvaluator`)

The core position evaluation system transforms a raw game state layout into a scalar numerical score through a two-stage evaluation pipeline: **Feature Extraction** and an optimized linear **Dot Product (Dot Vector)** execution.

#### 1. Evaluation Architecture Flow

* **State Parsing**: Extracts terminal conditions first, short-circuiting calculation paths if a definitive match resolution is found.
* **Macro Analysis (`extractMeta`)**: Tallies owned sub-boards, evaluates 9x9 alignment lines, tracks global threats/forks, and calculates tactical alignment approximations (`metaNearWin`).
* **Micro Analysis (`extractSubBoards`)**: Iterates through active sub-grids, scoring raw cell possessions, micro lines, and imminent local board sub-fork threats.
* **Forced Action Profiling (`extractForcedMoves`)**: Evaluates structural properties of a forced active sub-board setup, scoring tactical layout danger relative to the board's meta-game weight.

#### 2. Vector Field Definitions (`Features`)

The evaluation calculation tracks the following parameters to execute a final score generation:

| Feature Variable | Target Context Metric |
| :--- | :--- |
| `terminalWin` / `terminalLoss` | Absolute terminal boundary win or loss confirmation states. |
| `metaOwned` / `metaOpponentOwned` | Static grid alignment value calculated from won sub-boards. |
| `metaTwoInRow` / `metaOneInRow` | Quantified sub-board alignment strings tracking a future 9x9 win pattern. |
| `metaFork` / `metaOpponentFork` | Tracks if a player holds multiple cross-intersecting lines on the macro-grid. |
| `metaNearWin` / `metaOpponentNearWin` | Micro-threat presence scaled directly by the position weight of that sub-board. |
| `subCellControl` / `subCellOpponentControl` | Positional matrix points capturing single raw square ownership on active spaces. |
| `subTwoInRow` / `subOneInRow` | Quantifies localized 3x3 threats or development sequences inside open blocks. |
| `subFork` / `subOpponentFork` | Tracks internal multi-directional win threats within isolated sub-grids. |
| `forcedGood` / `forcedVeryGood` | Strategic evaluation of being forced into an advantageous micro-board layout. |
| `forcedBad` / `forcedVeryBad` | Tactical danger metrics computed when forced onto highly volatile or threatened sub-boards. |
| `metaImportanceGood` / `metaImportanceBad` | Multiplier checking if an active sub-board holds the final cell required to win a macro-line. |
| `freeMove` | Score offset applied when an operation unlocks a total board free-move action state. |

### Communication (ArenaHost)

The `ArenaHost` class serves as the network/protocol translation layer, completely decoupling your AI core algorithmic engine from the underlying tournament platform framework or communication pipes:

* **Session Orchestration & Game Loop Control**
    * **Mechanism:** Acts as the entry driver that executes the overarching state loop across an automated multi-game series (`runSession`). It handles proper setup initializations, monitors step sequences, and gracefully flushes game termination pipelines.
    * **Impact:** Keeps the AI's core minimax tree entirely separated from session management logic. The search architecture doesn't need to know how many games are left in a tournament or what the session bounds are.

* **Dynamic Side Selection & State Tracking**
    * **Mechanism:** Automatically intercepts the engine's initial turn sequences. By observing whether the external engine prompts the AI to move first or provides an opponent's opening coordinates, `ArenaHost` determines the AI's structural assignment (`CellState::X` vs. `CellState::Y`) dynamically.
    * **Impact:** Eliminates hardcoded color configurations or error-prone side parameters, making the engine plug-and-play across diverse competition hosts.

* **State Synchronization & Data Conversion**
    * **Mechanism:** Maps incoming generic spatial coordinate records (`GameMove` global rows and columns) into optimized localized index structures (`AIMove` macro/micro indices) via the `MoveConverter` utility. It then safely feeds these structural shifts straight down to update the internal `GameManager` representation.
    * **Impact:** Shields your highly specialized board state lookup optimizations from external array layouts. If the tournament protocol modifications change their frame data formats, you only need to adjust the `ArenaHost` boundary translation mapping without touching a single heuristic parameter inside your evaluation or search trees.

## Technical Optimizations

To compete effectively under strict time controls, the engine implements low-level architectural optimizations focused on cache efficiency, memory bandwidth conservation, and instruction throughput:

* **Memory Management (Zero Runtime Allocation)**
    * **Mechanism:** The Transposition Table (`_transpositionTable`) is initialized and pre-allocated as a contiguous block in memory (`std::vector<TTEntry>`) during class instantiation. Its size is locked to a power of two ($2^{20} = 1,048,576$ entries).
    * **Impact:** Eliminates expensive heap allocations (`malloc`/`new`) during active search queries. By leveraging a power-of-two size constraint, index mapping avoids costly modulo operations (`hash % size`) and instead executes via an ultra-fast bitwise AND mask (`hash & (TT_SIZE - 1)`), reducing structural lookup latency down to CPU cycles.

* **Fast Reversible Tree Mutators (`applyMoveFast` / `undoMove`)**
    * **Mechanism:** Instead of utilizing traditional "copy-on-move" minimax states—where every evaluated branch duplicates the entire board structure—the search modifies a single, global `GameState` reference inline. The system packages minimal structural delta parameters into a lightweight, stack-allocated `MoveUndo` structure.
    * **Impact:** Reduces memory traffic and allocation footprints down to near-zero. Reverting an exploratory branch is a simple matter of restoring a few primitive variables (the previous active board index and bitwise cell flips), allowing the engine to traverse millions of states while keeping data resident within the CPU's L1/L2 cache lines.

* **Branch Minimization & Cache Line Optimization**
    * **Mechanism:** Arrays are flattened where possible and packed tightly (such as tracking active boards with simple flat arrays or bit fields). Branch paths are designed sequentially, allowing compilers to perform loop unrolling and generate branchless instructions.
    * **Impact:** Maximizes **Nodes-Per-Second (NPS)** metrics. Minimizing memory indirection (following pointers across disparate memory addresses) protects the CPU from cache misses, while flattening code logic prevents hardware pipeline stalls caused by branch mispredictions during deep alpha-beta lookahead trees.

## Game Flow

1. **Main**: Initializes Zobrist keys and instantiates GameManager and ArenaHost.
2. **ArenaHost**: Receives a move from the engine.
3. **GameManager**: Updates the internal GameState.
4. **Strategy**: Minimax explores the search space using the Evaluator.
5. **Decision**: Returns the best move via ArenaHost to the engine.

## Academic Setup (Code::Blocks & Allegro 5)

### 1. Required Allegro 5.2 DLLs
Download Allegro 5.2 and place these **5 mandatory DLLs** directly into the directory containing your Code::Blocks project file (`.cbp`):

* `allegro-5.2.dll`
* `allegro_font-5.2.dll`
* `allegro_ttf-5.2.dll`
* `allegro_image-5.2.dll`
* `allegro_primitives-5.2.dll`

### 2. Code::Blocks Workspace Configuration
In **Project** -> **Build options...** (Select the root target on the left panel):

* **Workspace Files:** Import all code files from the `src/` directory into your project workspace.
* **Search Directories:** * **Compiler tab:** Add Allegro's `include/` path.
  * **Linker tab:** Add Allegro's `lib/` path.
* **Linker Settings:** Add the library files (`.a` or `.lib`) corresponding to the 5 components listed above.