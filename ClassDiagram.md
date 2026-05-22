```mermaid
classDiagram
    direction topDown

    %% --- INTERFACES & ABSTRACT LAYERS ---
    class IStrategy {
        <<interface>>
        +chooseMove(GameState& state)* AIMove
        +reset()* void
    }

    class IEvaluator {
        <<interface>>
        +evaluate(GameState state) int*
    }

    %% --- AI CORE & STRATEGY ---
    class MinimaxStrategy {
        -IEvaluator* _evaluatorLight
        -IEvaluator* _evaluator
        -int _maxDepth
        -vector~TTEntry~ _transpositionTable
        -size_t TT_SIZE$
        +MinimaxStrategy(IEvaluator* evaluator, int depth)
        +chooseMove(GameState& state) AIMove
        +reset() void
        -orderMovesWithEval(GameState& state, vector~AIMove~& moves, AIMove& ttHint, bool maximizing, int depth) void
        -minimax(GameState& state, int depth, bool maximizing, int alpha, int beta) int
    }
    IStrategy <|-- MinimaxStrategy : implements
    MinimaxStrategy ..> IEvaluator : uses

    class TTFlag {
        <<enum>>
        EXACT
        LOWER_BOUND
        UPPER_BOUND
    }

    class TTEntry {
        <<struct>>
        +uint64_t key
        +int value
        +int depth
        +TTFlag flag
        +AIMove bestMove
        +bool wasMaximizing
    }
    MinimaxStrategy +-- TTEntry
    TTEntry --> TTFlag : uses

    %% --- EVALUATION ENGINE ---
    class FeatureEvaluator {
        -Weights w$
        -int boardWeight[9]$
        +evaluate(GameState state) int
        +evaluate(GameState state, CellState perspective) int
        -extract(GameState state) Features
        -extractTerminal(UltimateBoard b, Features& f, CellState me, CellState opp) void
        -extractMeta(UltimateBoard b, Features& f, CellState me, CellState opp) void
        -extractSubBoards(UltimateBoard b, Features& f, CellState me, CellState opp) void
        -extractForcedMoves(UltimateBoard b, Features& f, CellState me, CellState opp) void
        -evaluateMetaImportance(UltimateBoard b, int boardIndex, CellState me, CellState opp, Features& f) int
        -dot(Features f) int
    }
    IEvaluator <|-- FeatureEvaluator : implements

    class Features {
        <<struct>>
        +int terminalWin
        +int terminalLoss
        +int metaOwned
        +int metaOpponentOwned
        +int metaTwoInRow
        +int metaOneInRow
        +int metaOpponentTwoInRow
        +int metaOpponentOneInRow
        +int metaFork
        +int metaOpponentFork
        +int subCellControl
        +int subCellOpponentControl
        +int subTwoInRow
        +int subOneInRow
        +int subOpponentTwoInRow
        +int subOpponentOneInRow
        +int subFork
        +int subOpponentFork
        +int forcedGood
        +int forcedVeryGood
        +int forcedBad
        +int forcedVeryBad
        +int freeMove
        +int metaImportanceGood
        +int metaImportanceBad
        +int boardPositionBonus
        +int metaNearWin
        +int metaOpponentNearWin
    }

    class Weights {
        <<struct>>
        +int terminalWin
        +int terminalLoss
        +int metaOwned
        +int metaOpponentOwned
        +int metaTwoInRow
        +int metaOneInRow
        +int metaOpponentTwoInRow
        +int metaOpponentOneInRow
        +int metaFork
        +int metaOpponentFork
        +int subCellControl
        +int subCellOpponentControl
        +int subTwoInRow
        +int subOneInRow
        +int subOpponentTwoInRow
        +int subOpponentOneInRow
        +int subFork
        +int subOpponentFork
        +int forcedGood
        +int forcedVeryGood
        +int forcedBad
        +int forcedVeryBad
        +int freeMove
        +int metaImportanceGood
        +int metaImportanceBad
        +int boardPositionBonus
        +int metaNearWin
        +int metaOpponentNearWin
    }
    FeatureEvaluator +-- Features
    FeatureEvaluator +-- Weights

    %% --- HIGH LEVEL ARCHITECTURE ---
    class ArenaHost {
        -GameManager& _manager
        -MoveConverter _converter
        +ArenaHost(GameManager& manager)
        +runSession(int numGames, Level level) void
        -playSingleGame() void
    }
    ArenaHost --> GameManager : orchestrates
    ArenaHost --> MoveConverter : uses

    class GameManager {
        -int s_gameId$
        -int _gameId
        -long long _runTimestamp
        -GameState _state
        -CellState _me
        -CellState _opponent
        -unique_ptr~IEvaluator~ _evaluator
        -unique_ptr~IStrategy~ _minimaxStrategy
        +GameManager(long long runTimestamp)
        +init(CellState mySide) void
        +finalizeGame() void
        +applyMove(AIMove move) void
        +chooseMove() AIMove
        +getState() GameState&
        +getOpponent() CellState
    }
    GameManager *-- GameState : owns
    GameManager --> IStrategy : delegates search
    GameManager --> IEvaluator : owns

    %% --- GAME STATE & LOGIC ---
    class GameState {
        -UltimateBoard _board
        -CellState _myPlayer
        -CellState _opponent
        -CellState _currentPlayer
        -uint64_t _currentHash
        +GameState()
        +setPlayers(CellState me) void
        +switchPlayers() void
        +reset() void
        +applyMove(AIMove move) bool
        +getCurrentPlayer() CellState
        +getBoard() UltimateBoard&
        +getValidMoves() vector~AIMove~
        +isTerminal() bool
        +getMyPlayer() CellState
        +getOpponent() CellState
        +getWinner() CellState
        +applyMoveFast(AIMove move) MoveUndo
        +undoMove(MoveUndo undo) void
        +calculateHash() uint64_t
        +getActiveBoard() int
        +applyNullMove() int
        +undoNullMove(int activeBoard) bool
        +getMovesLeft() int
        +getHash() uint64_t
        +updateHash(AIMove move, CellState player, int oldActive, int newActive) void
    }
    GameState *-- UltimateBoard : owns

    class UltimateBoard {
        -array~SubBoard, 9~ _boards
        -int _activeBoard
        +UltimateBoard()
        +playMove(AIMove aIMove, CellState player) bool
        +isValidMove(AIMove aIMove) bool
        +checkWinner() CellState
        +isFull() bool
        +updateActiveBoard(int lastCellIndex) void
        +getActiveBoard() int
        +getBoard(int index) SubBoard&
        +reset() void
        +isEmpty() bool
        +undoMove(AIMove move, int prevActiveBoard) void
        +getMovesLeftBoard() int
    }
    UltimateBoard *-- SubBoard : contains 9

    class SubBoard {
        -array~Cell, 9~ _cells
        -bool _isFull
        -CellState _winner
        -int _filledCount
        +SubBoard()
        +playMove(int index, CellState player) bool
        +checkWinner() CellState
        +isFull() bool
        +isPlayable() bool
        +getCell(int index) Cell&
        +reset() void
        +isEmpty() bool
        +undoMove(int index) void
        +getMovesLetftSubBoard() int
    }
    SubBoard *-- Cell : contains 9

    class Cell {
        -CellState _state
        +Cell()
        +getState() CellState
        +setState(CellState newState) void
        +isEmpty() bool
    }
    Cell --> CellState : tracks

    class CellState {
        <<enum>>
        EMPTY
        X
        O
    }

    %% --- UTILITIES & HELPERS ---
    class Zobrist {
        +uint64_t cell[9][9][2]$
        +uint64_t activeBoard[10]$
        +uint64_t player[2]$
        +init() void$
        +activeBoardIndex(int b) int$
    }
    GameState --> Zobrist : optimizes hashing

    class AIMove {
        <<struct>>
        +int boardIndex
        +int cellIndex
        +isValid() bool
        +operator==(AIMove other) bool
        +operator!=(AIMove other) bool
    }

    class MoveUndo {
        <<struct>>
        +AIMove move
        +int prevActiveBoard
    }
    MoveUndo --> AIMove : encapsulates
    GameState ..> MoveUndo : returns/consumes

    class MoveConverter {
        +toGameMove(AIMove m) GameMove
        +toAIMove(GameMove p) AIMove
    }
    MoveConverter ..> AIMove : converts

    %% Relationships highlighting interactions with standard types
    GameState ..> AIMove : uses
    UltimateBoard ..> AIMove : uses
    ```