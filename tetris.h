/*
    This is a header file which tells you how a Tetris game would look and work in general, and groups the game into three main categories: 
Piece, Board and Tetris. The Piece class contains all the state and behavior of the Tetris pieces – for example, their type (PieceKind), 
color (TileColor), bounding box, shape and rotation states. It also gives methods to retrieve these properties, rotate the object, and 
access "kick" rotation offsets for collisions with walls or other objects. Board class is the playing surface of Tetris, it controls 
the tile grid and active piece on it. It contains pieces movements (move, rotate, hard drop), removal of lines, and spawning of new 
pieces. It also stores the ghost piece (projection of the active piece’s location) and open lines. Tetris class is the game controller 
which orchestrates gameplay, it handles the Board, state and scoring system. It performs restarts, restores the game state (based on 
player action — move, rotate, drop pieces), checks the level, and enforces rules — soft drops, lock delays, line clearing 
delays. The Tetris class also uses bag system for piece order randomization. There are some constants set in the file 
(e.g., kNumPieces) which controls game settings such as number of types of pieces, level, and movement and lock-in delays. 
These classes are the base classes for implementing the Tetris logic and rules together.

*/

#ifndef TETRIS_TETRIS_H
#define TETRIS_TETRIS_H

#include <algorithm>
#include <cassert>
#include <vector>
#include <random>
#include <iostream>

const int kNumPieces = 7;

enum TileColor { kEmpty = -1, kCyan, kBlue, kOrange, kYellow, kGreen, kPurple, kRed };
enum PieceKind { kNone = -1, kPieceI, kPieceJ, kPieceL, kPieceO, kPieceS, kPieceT, kPieceZ };
enum class Rotation { kRight, kLeft };
enum class Motion { kNone, kRight, kLeft };

/**
 * \class Piece
 *
 * \brief A class that represents a Tetris piece.
 *
 * The piece is represented by its kind, color, bounding box side, and shape.
 * The shape is a vector of TileColor, representing the color of each tile in
 * the piece.
 */
class Piece {
public:
/**
     * \brief Constructor for Piece.
     *
     * The piece is created with the given kind.
     *
     * \param kind The kind of the piece.
     */
    explicit Piece(PieceKind kind);
/**
     * \brief The kind of the piece.
     *
     * \return The kind of the piece.
     */
    PieceKind kind() const { return kind_; }
    /**
     * \brief The color of the piece.
     *
     * \return The color of the piece.
     */
    TileColor color() const { return color_; }
    /**
     * \brief The side of the bounding box of the piece.
     *
     * \return The side of the bounding box of the piece.
     */
    int bBoxSide() const { return bBoxSide_; }
    /**
     * \brief The number of rows of the piece.
     *
     * \return The number of rows of the piece.
     */
    int nRows() const { return nRows_; }
    /**
     * \brief The number of columns of the piece.
     *
     * \return The number of columns of the piece.
     */
    int nCols() const { return nCols_; }
    /**
     * \brief The shape of the piece.
     *
     * The shape is a vector of TileColor, representing the color of each tile in
     * the piece.
     *
     * \return The shape of the piece.
     */
    const std::vector<TileColor>& shape() const { return shape_; }
    /**
     * \brief The initial shape of the piece.
     *
     * The initial shape is a vector of TileColor, representing the color of each
     * tile in the piece.
     *
     * \return The initial shape of the piece.
     */
    const std::vector<TileColor>& initialShape() const { return initialShape_; }
    /**
     * \brief Rotate the piece to the right.
     *
     * The rotation of the piece is implemented by transposing the shape of the
     * piece and reversing the order of the columns. The new shape of the piece is
     * computed from the old shape by iterating over the columns of the old shape
     * from right to left and the rows from top to bottom.
     *
     * The state of the piece is updated by incrementing it by one. If the state is
     * equal to kNumStates_, it is reset to 0.
     */
    void rotate(Rotation rotation);
    /**
     * \brief The kicks of the piece.
     *
     * The kicks are the possible positions of the piece after rotation. The kicks
     * are a vector of pairs of integers, representing the column and row of the
     * piece after rotation.
     *
     * \param rotation The rotation of the piece.
     *
     * \return The kicks of the piece after rotation.
     */
    const std::vector<std::pair<int, int>>& kicks(Rotation rotation) const;

private:
/**
     * \brief The number of states of the piece.
     *
     * The number of states of the piece is 4, representing the 4 possible
     * orientations of the piece.
     */
    static const int kNumStates_;
    /**
     * \brief The kicks of the I piece.
     *
     * The kicks of the I piece are a vector of pairs of integers, representing the
     * column and row of the piece after rotation.
     */
    static const std::vector<std::vector<std::pair<int, int>>> kKicksIRight_, kKicksILeft_;
    /**
     * \brief The kicks of the other pieces.
     *
     * The kicks of the other pieces are a vector of pairs of integers, representing
     * the column and row of the piece after rotation.
     */
    static const std::vector<std::vector<std::pair<int, int>>> kKicksOtherRight_, kicksOtherLeft_;
/**
     * \brief The kind of the piece.
     */
    PieceKind kind_;
    /**
     * \brief The color of the piece.
     */
    TileColor color_;
    /**
     * \brief The number of rows of the piece.
     */
    /**
     * \brief The number of columns of the piece.
     */
    int nRows_, nCols_;
    /**
     * \brief The initial shape of the piece.
     *
     * The initial shape is a vector of TileColor, representing the color of each
     * tile in the piece.
     */
    std::vector<TileColor> initialShape_;
    /**
     * \brief The bounding box side of the piece.
     */
    int bBoxSide_;
    /**
     * \brief The shape of the piece.
     */
    std::vector<TileColor> shape_;
    /**
     * \brief The state of the piece.
     *
     * The state of the piece is an integer representing the number of times the
     * piece has been rotated.
     */
    int state_;
    /**
     * \brief The kicks of the piece.
     */
    std::vector<std::vector<std::pair<int, int>>> kicksRight_;
    /**
     * \brief The kicks of the piece.
     */
    std::vector<std::vector<std::pair<int, int>>> kicksLeft_;
};

/**
 * \class Board
 *
 * \brief Represents the Tetris game board.
 *
 * The Board class encapsulates the state and behavior of a Tetris game board,
 * including the tiles, current piece, and logic for piece movement and line clearing.
 */
class Board {
public:
/**
     * \brief Number of rows and columns in the board.
     */
    const int nRows, nCols;
    /**
     * \brief Constructor for Board.
     *
     * \param nRows Number of rows in the board.
     * \param nCols Number of columns in the board.
     */
    Board(int nRows, int nCols);
    /**
     * \brief Clears the board.
     */
    void clear();
    /**
     * \brief Returns the color of the tile at the specified position.
     *
     * \param row The row of the tile.
     * \param col The column of the tile.
     * \return The color of the tile.
     */
    TileColor tileAt(int row, int col) const { return tiles_[(row + kRowsAbove_) * nCols + col]; };
    /**
     * \brief Freezes the current piece on the board.
     *
     * \return True if the piece has reached the bottom, false otherwise.
     */
    bool frozePiece();
    /**
     * \brief Spawns a new piece of the specified kind.
     *
     * \param kind The kind of piece to spawn.
     * \return True if the piece was successfully spawned, false if not possible.
     */
    bool spawnPiece(PieceKind kind);
    /**
     * \brief Moves the current piece horizontally.
     *
     * \param dCol The horizontal displacement.
     * \return True if the move was successful, false otherwise.
     */
    bool moveHorizontal(int dCol);
    /**
     * \brief Moves the current piece vertically.
     *
     * \param dRow The vertical displacement.
     * \return True if the move was successful, false otherwise.
     */
    bool moveVertical(int dRow);
    /**
     * \brief Rotates the current piece.
     *
     * \param rotation The rotation direction.
     * \return True if the rotation was successful, false otherwise.
     */
    bool rotate(Rotation rotation);
    /**
     * \brief Performs a hard drop of the current piece.
     *
     * \return The number of rows the piece dropped.
     */
    int hardDrop();
    /**
     * \brief Checks if the current piece is on the ground.
     *
     * \return True if the piece is on the ground, false otherwise.
     */
    bool isOnGround() const;
    /**
     * \brief Returns the number of lines to clear.
     *
     * \return The number of lines to clear.
     */
    int numLinesToClear() const { return linesToClear_.size(); };
    /**
     * \brief Clears the lines marked for clearing.
     */
    void clearLines();
    /**
     * \brief Returns the lines marked for clearing.
     *
     * \return A vector of line indices to clear.
     */
    const std::vector<int>& linesToClear() const { return linesToClear_; }
    /**
     * \brief Returns the current piece.
     *
     * \return The current piece.
     */
    Piece piece() const { return piece_; }
    /**
     * \brief Returns the current row of the piece.
     *
     * \return The row of the piece.
     */
    int pieceRow() const { return row_; }
    /**
     * \brief Returns the current column of the piece.
     *
     * \return The column of the piece.
     */
    int pieceCol() const { return col_; }
    /**
     * \brief Returns the current row of the ghost piece.
     *
     * \return The row of the ghost piece.
     */
    int ghostRow() const { return ghostRow_; }

private:
/**
     * \brief Number of rows above the visible area.
     */
    static const int kRowsAbove_;
    /**
     * \brief The tiles of the board.
     */
    std::vector<TileColor> tiles_;
    /**
     * \brief The current piece on the board.
     */
    Piece piece_;
    /**
     * \brief The current row of the piece.
     */
    int row_ = 0;
    /**
     * \brief The current column of the piece.
     */
    int col_ = 0;
    /**
     * \brief The current row of the ghost piece.
     */
    int ghostRow_ = 0;
    /**
     * \brief The tiles after clearing lines.
     */
    std::vector<TileColor> tilesAfterClear_;
    /**
     * \brief The lines marked for clearing.
     */
    std::vector<int> linesToClear_;
    /**
     * \brief Sets the tile color at the specified position.
     *
     * \param row The row of the tile.
     * \param col The column of the tile.
     * \param color The color to set.
     */
    void setTile(int row, int col, TileColor color);
    /**
     * \brief Checks if a tile is filled.
     *
     * \param row The row of the tile.
     * \param col The column of the tile.
     * \return True if the tile is filled, false otherwise.
     */
    bool isTileFilled(int row, int col) const;
    /**
     * \brief Checks if the specified piece position is possible.
     *
     * \param row The row of the position.
     * \param col The column of the position.
     * \param piece The piece to check.
     * \return True if the position is possible, false otherwise.
     */
    bool isPositionPossible(int row, int col, const Piece& piece) const;
    /**
     * \brief Updates the ghost row for the current piece.
     */
    void updateGhostRow();
    /**
     * \brief Finds the lines to clear.
     */
    void findLinesToClear();
};

class Tetris {
public:
/**
     * \brief Constructor.
     *
     * \param board The game board.
     * \param timeStep The time step of the game.
     * \param randomSeed The random seed.
     */
    Tetris(Board& board, double timeStep, unsigned int randomSeed);
    /**
     * \brief Restarts the game at the specified level.
     *
     * \param level The level to restart at.
     */
    void restart(int level);
    /**
     * \brief Checks if the game is over.
     * \return True if the game is over, false otherwise.
     */
    bool isGameOver() const { return gameOver_; }
    /**
     * \brief Updates the game state.
     *
     * \param softDrop Whether to soft drop the pieces.
     * \param moveRight Whether to move the piece right.
     * \param moveLeft Whether to move the piece left.
     */
    void update(bool softDrop, bool moveRight, bool moveLeft);
    /**
     * \brief Rotates the current piece.
     *
     * \param rotation The rotation to apply.
     */
    void rotate(Rotation rotation);
    /**
     * \brief Hard drops the current piece.
     */
    void hardDrop();
    /**
     * \brief Gets the current lock percentage.
     * \return The lock percentage as a value between 0 and 1.
     */
    double lockPercent() const { return lockingTimer_ / kLockDownTimeLimit_; }
    /**
     * \brief Checks if the game is paused for lines clear.
     * \return True if the game is paused, false otherwise.
     */
    bool isPausedForLinesClear() const { return pausedForLinesClear_; }
    /**
     * \brief Gets the lines clear pause percentage.
     * \return The pause percentage as a value between 0 and 1.
     */
    double linesClearPausePercent() const { return linesClearTimer_ / kPauseAfterLineClear_; }
    /**
     * \brief Gets the current level.
     * \return The level.
     */
    int level() const { return level_; }
    /**
     * \brief Gets the number of lines cleared.
     * \return The number of lines cleared.
     */
    int linesCleared() const { return linesCleared_; }
    /**
     * \brief Gets the next piece.
     * \return The next piece.
     */
    Piece nextPiece() const { return Piece(bag_[nextPiece_]); }

private:
    static const int kMaxLevel_;
    static const double kMoveDelay_;
    static const double kMoveRepeatDelay_;
    static const double kSoftDropSpeedFactor_;
    static const double kLockDownTimeLimit_;
    static const int kLockDownMovesLimit_;
    static const double kPauseAfterLineClear_;

    Board& board_;

    bool gameOver_ = false;

    double timeStep_;

    std::default_random_engine rng_;
    std::vector<PieceKind> bag_;
    int nextPiece_;

    int level_;
    int linesCleared_;
    int score_;

    double secondsPerLine_;
    double moveDownTimer_;

    Motion motion_;
    bool moveLeftPrev_, moveRightPrev_;
    double moveRepeatDelayTimer_;
    double moveRepeatTimer_;

    bool isOnGround_;
    double lockingTimer_;
    int nMovesWhileLocking_;

    bool pausedForLinesClear_;
    double linesClearTimer_;

    void moveHorizontal(int dCol);
    void checkLock();
    void lock();
    void spawnPiece();
};

#endif  // TETRIS_TETRIS_H
