/*
This implementation file contains the code to control how Tetris plays: how the pieces are set up, rotated and stacked, as 
well as collision, line clearing, and scoring on the board. It takes the basics Piece, Board and Tetris classes:

Piece Class: The class represents individual Tetris pieces such as shape, rotation and wall kicks (distortion of rotation to deal 
with collision). Each PieceKind is a different Tetris item (e.g., I, J, L, etc. ), already designed shape and size. the create 
function constructs a piece with its type and rotate method rotates clockwise or counterclockwise by altering shape_ of the piece. 
It also updates the state of the work and wrap-arounds between rotation states. The kicks function gets the kick offsets for a given 
rotation that are necessary to rotate the piece away from walls or other pieces.

Board Type: The board contains the Tetris grid, tiles and currently occupied piece. It records pieces and keeps them in valid 
boundaries. The board accommodates piece moves (moveHorizontal and moveVertical), rotations (wall kicks), hard drops 
(spot placement at lowest position on the board) and line clearing. spawnPiece returns a new piece on the board that is placed 
near the top of the board and checks for game over. findLinesToClear finds empty rows to clear, clearLines removes those and moves 
the rest of the rows below. It also updates the "ghost" piece position so you can see where the active piece would be.

Tetris Class: This class controls play, the board, player inputs, scoring, levels, and state. For our purposes, we stuck with a
very basic one level and no actual score displayed, due to time constraints. It uses bag randomization to randomly assign the piece 
sequence. The restart operation takes the game back to default state (erase the board, and set variables). 
Update: The update method takes in player commands, like low drops, side movements, and rotations, and calculates movement delay and repeat 
times for better gameplay. It even handles locking (securing piece placement) and line-clearing interludes for graphical output. CheckLock and lock allow 
you to check pieces and slap hard drops or deal with game-over.

Game Constants: Some gameplay constants like the soft drop speed, the delay for repetition of movement, the lock-piece time limit, 
the level-up line count for our one level etc. These constants determine how the game is progressing and how hard it is.

It implements everything you'd expect from an XY-tetris game, from the pieces to begin playing, from the rotations to scoring, 
line-clearing, and level development. Wall kicks keep things nice and clean and the bag randomization and ghost piece systems make 
it fun.
*/


/**
 * \file tetris.cpp
 * \brief Contains the implementation of the Piece class.
 */

#include "tetris.h"

/**
 * \brief The number of states a piece can be in.
 */
const int Piece::kNumStates_ = 4;

/**
 * \brief The kicks for I pieces when rotating right.
 *
 * A kick is a translation of the piece that is necessary to ensure that the
 * piece is in a valid position on the board after rotation. The kicks are
 * determined by the shape of the piece.
 */
const std::vector<std::vector<std::pair<int, int>>> Piece::kKicksIRight_ = {
    {{0, 0}, {0, -2}, {0, 1}, {1, -2}, {-2, 1}},
    {{0, 0}, {0, -1}, {0, 2}, {-2, -1}, {1, 2}},
    {{0, 0}, {0, 2}, {0, -1}, {-1, 2}, {2, -1}},
    {{0, 0}, {0, 1}, {0, -2}, {2, 1}, {-1, -2}}};

/**
 * \brief The kicks for I pieces when rotating left.
 *
 * A kick is a translation of the piece that is necessary to ensure that the
 * piece is in a valid position on the board after rotation. The kicks are
 * determined by the shape of the piece.
 */
const std::vector<std::vector<std::pair<int, int>>> Piece::kKicksILeft_ = {{{0, 0}, {0, -1}, {0, 2}, {-2, -1}, {1, 2}},
                                                                           {{0, 0}, {0, 2}, {0, -1}, {-1, 2}, {2, -1}},
                                                                           {{0, 0}, {0, 1}, {0, -2}, {2, 1}, {-1, -2}},
                                                                           {{0, 0}, {0, -2}, {0, 1}, {1, -2}, {-2, 1}}};

/**
 * \brief The kicks for non-I pieces when rotating right.
 *
 * A kick is a translation of the piece that is necessary to ensure that the
 * piece is in a valid position on the board after rotation. The kicks are
 * determined by the shape of the piece.
 */
const std::vector<std::vector<std::pair<int, int>>> Piece::kKicksOtherRight_ = {
    {{0, 0}, {0, 1}, {-1, -1}, {2, 0}, {2, -1}},
    {{0, 0}, {0, 1}, {1, 1}, {-2, 0}, {-2, 1}},
    {{0, 0}, {0, 1}, {-1, 1}, {2, 0}, {2, 1}},
    {{0, 0}, {0, -1}, {1, -1}, {-2, 0}, {-2, -1}}};

/**
 * \brief The kicks for non-I pieces when rotating left.
 *
 * A kick is a translation of the piece that is necessary to ensure that the
 * piece is in a valid position on the board after rotation. The kicks are
 * determined by the shape of the piece.
 */
const std::vector<std::vector<std::pair<int, int>>> Piece::kicksOtherLeft_ = {
    {{0, 0}, {0, 1}, {-1, 1}, {2, 0}, {2, 1}},
    {{0, 0}, {0, -1}, {1, 1}, {-2, 0}, {-2, 1}},
    {{0, 0}, {0, -1}, {-1, -1}, {2, 0}, {2, -1}},
    {{0, 0}, {0, -1}, {1, -1}, {-2, 0}, {-2, -1}}};

/**
 * \brief Constructs a Tetris piece with the given kind.
 *
 * This constructor sets the size of the piece, the bounding box side, and the
 * initial shape of the piece. It also sets the color of the piece and the
 * number of tiles in the piece.
 *
 * \param kind The kind of piece to construct.
 */
Piece::Piece(PieceKind kind) : kind_(kind), color_(static_cast<TileColor>(kind)), state_(0) {
    TileColor e = kEmpty;
    TileColor c = color_;
    switch (kind) {
    case kNone:
        nRows_ = 0;
        nCols_ = 0;
        bBoxSide_ = 0;
        break;
    case kPieceI:
        nRows_ = 1;
        nCols_ = 4;
        bBoxSide_ = 4;
        initialShape_ = {c, c, c, c};
        shape_ = {e, e, e, e, c, c, c, c, e, e, e, e, e, e, e, e};
        break;
    case kPieceJ:
        nRows_ = 2;
        nCols_ = 3;
        bBoxSide_ = 3;
        initialShape_ = {c, e, e, c, c, c};
        shape_ = {c, e, e, c, c, c, e, e, e};
        break;
    case kPieceL:
        nRows_ = 2;
        nCols_ = 3;
        bBoxSide_ = 3;
        initialShape_ = {e, e, c, c, c, c};
        shape_ = {e, e, c, c, c, c, e, e, e};
        break;
    case kPieceO:
        nRows_ = 2;
        nCols_ = 2;
        bBoxSide_ = 2;
        initialShape_ = {c, c, c, c};
        shape_ = initialShape_;
        break;
    case kPieceS:
        nRows_ = 2;
        nCols_ = 3;
        bBoxSide_ = 3;
        initialShape_ = {e, c, c, c, c, e};
        shape_ = {e, c, c, c, c, e, e, e, e};
        break;
    case kPieceT:
        nRows_ = 2;
        nCols_ = 3;
        bBoxSide_ = 3;
        initialShape_ = {e, c, e, c, c, c};
        shape_ = {e, c, e, c, c, c, e, e, e};
        break;
    case kPieceZ:
        nRows_ = 2;
        nCols_ = 3;
        bBoxSide_ = 3;
        initialShape_ = {c, c, e, e, c, c};
        shape_ = {c, c, e, e, c, c, e, e, e};
        break;
    }

    if (kind == kPieceO || kind == kNone) {
        return;
    }

    if (kind == kPieceI) {
        kicksRight_ = kKicksIRight_;
        kicksLeft_ = kKicksILeft_;
    } else {
        kicksRight_ = kKicksOtherRight_;
        kicksLeft_ = kicksOtherLeft_;
    }
}

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
void Piece::rotate(Rotation rotation) {
    if (kind_ == kPieceO) {
        return;
    }

    std::vector<TileColor> newShape(shape_.size());
    int index = 0;
    switch (rotation) {
    case Rotation::kRight:
        state_ += 1;
        for (int col = bBoxSide_ - 1; col >= 0; --col) {
            for (int row = 0; row < bBoxSide_; ++row) {
                newShape[row * bBoxSide_ + col] = shape_[index];
                ++index;
            }
        }
        break;
    case Rotation::kLeft:
        state_ -= 1;
        for (int col = 0; col < bBoxSide_; ++col) {
            for (int i = bBoxSide_ - 1; i >= 0; --i) {
                newShape[i * bBoxSide_ + col] = shape_[index];
                ++index;
            }
        }
    }
    shape_ = newShape;

    if (state_ == -1) {
        state_ = kNumStates_ - 1;
    } else if (state_ == kNumStates_) {
        state_ = 0;
    }
}

const std::vector<std::pair<int, int>>& Piece::kicks(Rotation rotation) const {
    switch (rotation) {
    case Rotation::kRight: return kicksRight_[state_];
    case Rotation::kLeft: return kicksLeft_[state_];
    }
    throw std::runtime_error("This line is unreachable!");
}

const int Board::kRowsAbove_ = 2;
/**
 * \brief Constructor for Board.
 *
 * This constructor initializes the Board object with the given
 * parameters. The tiles_ vector is initialized with the given number
 * of rows and columns, and the piece_ is initialized to kNone.
 *
 * \param nRows The number of rows in the board.
 * \param nCols The number of columns in the board.
 */
Board::Board(int nRows, int nCols)
    : nRows(nRows), nCols(nCols), tiles_((nRows + kRowsAbove_) * nCols, kEmpty), piece_(kNone) {}

void Board::clear() { std::fill(tiles_.begin(), tiles_.end(), kEmpty); }

/**
 * \brief Freezes the current piece on the board.
 *
 * This function is called when the current piece reaches the bottom of the
 * board. It copies the shape of the piece to the tiles_ vector, and updates
 * the state of the board by finding the lines to clear.
 *
 * \return true if the current piece has reached the bottom of the board, false
 *         otherwise.
 */
bool Board::frozePiece() {
    auto shape = piece_.shape();
    bool belowSkyline = false;
    int index = 0;
    // Iterate over the rows of the piece
    for (int row = row_; row < row_ + piece_.bBoxSide(); ++row) {
        // Iterate over the columns of the piece
        for (int col = col_; col < col_ + piece_.bBoxSide(); ++col) {
            if (shape[index] != kEmpty) {
                if (row >= 0) {
                    belowSkyline = true;
                }
                // Copy the tile of the piece to the tiles_ vector
                setTile(row, col, shape[index]);
            }
            ++index;
        }
    }
    // Find the lines to clear after freezing the piece
    findLinesToClear();
    // Reset the current piece to kNone
    piece_ = Piece(kNone);
    return belowSkyline;
}


/**
 * \brief Spawns a new piece on the board.
 *
 * This function spawns a new piece at the top of the board. The piece is
 * centered horizontally and moved down until it is not possible to move it
 * down any further.
 *
 * \param kind The kind of the piece to spawn.
 * \return true if the piece was spawned successfully, false otherwise.
 */
bool Board::spawnPiece(PieceKind kind) {
    piece_ = Piece(kind);
    row_ = -2;
    col_ = (nCols - piece_.bBoxSide()) / 2;

    if (!isPositionPossible(row_, col_, piece_)) {
        return false;
    }
    // Move the piece down until it is not possible to move it down any further
    int maxMoveDown = kind == kPieceI ? 1 : 2;
    for (int moveDown = 0; moveDown < maxMoveDown; ++moveDown) {
        if (!isPositionPossible(row_ + 1, col_, piece_)) {
            break;
        }
        ++row_;
    }
    updateGhostRow();
    return true;
}

/**
 * \brief Moves the current piece horizontally.
 *
 * This function moves the current piece horizontally by dCol columns.
 * If the move is possible, the piece is moved and the ghost row is updated.
 * Otherwise, the function returns false.
 *
 * \param dCol The number of columns to move the piece.
 * \return true if the move was successful, false otherwise.
 */
bool Board::moveHorizontal(int dCol) {
        // Check if the move is possible
    if (isPositionPossible(row_, col_ + dCol, piece_)) {
            // Move the piece horizontally
        col_ += dCol;
        // Update the ghost row
        updateGhostRow();
        return true;
    }
    // The move is not possible, so return false
    return false;
}

/**
 * \brief Moves the current piece vertically.
 *
 * This function moves the current piece vertically by dRow rows. If the move
 * is possible, the piece is moved and the ghost row is updated. Otherwise,
 * the function returns false.
 *
 * \param dRow The number of rows to move the piece.
 * \return true if the move was successful, false otherwise.
 */
bool Board::moveVertical(int dRow) {
    // Check if the move is possible
    if (isPositionPossible(row_ + dRow, col_, piece_)) {
        // Move the piece vertically
        row_ += dRow;
        // Update the ghost row
        return true;
    }
    // The move is not possible, so return false
    return false;
}

/**
 * \brief Rotate the current piece by the given rotation.
 *
 * This function rotates the current piece by the given rotation. The
 * rotation is applied to the piece by calling the rotate() member function
 * of the Piece class. The function returns true if the rotation was
 * successful, false otherwise.
 *
 * \param rotation The rotation to apply.
 * \return true if the rotation was successful, false otherwise.
 */
bool Board::rotate(Rotation rotation) {
    if (piece_.kind() == kPieceO || piece_.kind() == kNone) {
        return false;
    }

    Piece testPiece(piece_);
    testPiece.rotate(rotation);
    // Apply the kick table to check for possible positions
    for (const auto kick : piece_.kicks(rotation)) {
        int dRow = kick.first;
        int dCol = kick.second;
        if (isPositionPossible(row_ + dRow, col_ + dCol, testPiece)) {
            piece_ = testPiece;
            row_ += dRow;
            col_ += dCol;
            updateGhostRow();
            return true;
        }
    }

    return false;
}


/**
 * \brief Drops the current piece until it reaches the bottom of the board.
 *
 * This function simulates a hard drop of the current piece by moving it
 * down until it reaches the bottom of the board. The number of rows that
 * the piece moves down is returned.
 *
 * \return The number of rows that the piece moved down.
 */
int Board::hardDrop() {
    int rowsPassed = ghostRow_ - row_;
    row_ = ghostRow_;
    return rowsPassed;
}

bool Board::isOnGround() const { return !isPositionPossible(row_ + 1, col_, piece_); }

/**
 * \brief Clears the lines that are marked for clearing.
 *
 * This function is called when the user clears the lines that are marked
 * for clearing. It clears the lines marked for clearing and updates the
 * tiles_ vector.
 */
void Board::clearLines() {
    if (linesToClear_.empty()) {
        // If there are no lines marked for clearing, do nothing.
        return;
    }
    // Clear the lines marked for clearing.
    linesToClear_.clear();
    // Update the tiles_ vector by copying the tilesAfterClear_ vector.
    tiles_ = tilesAfterClear_;
}

void Board::setTile(int row, int col, TileColor color) { tiles_[(row + kRowsAbove_) * nCols + col] = color; }

/**
 * \brief Checks if the given tile is filled.
 *
 * This function checks if the given tile is filled. If the tile is out of
 * bounds, it is considered filled. Otherwise, the function checks if the
 * tile is filled by checking the value of the tile at the given row and
 * column in the tiles_ vector.
 *
 * \param row The row of the tile to check.
 * \param col The column of the tile to check.
 * \return true if the tile is filled, false otherwise.
 */
bool Board::isTileFilled(int row, int col) const {
    if (col < 0 || col >= nCols || row < -kRowsAbove_ || row >= nRows) {
        // If the tile is out of bounds, consider it filled.
        return true;
    }
    // Check if the tile is filled by checking the value of the tile in the
    // tiles_ vector.
    return tileAt(row, col) != kEmpty;
}

/**
 * \brief Checks if the given piece is possible at the given row and column.
 *
 * This function checks if the given piece is possible at the given row and
 * column. It does this by checking if any of the tiles of the piece are out
 * of bounds or if they are filled.
 *
 * \param row The row of the piece.
 * \param col The column of the piece.
 * \param piece The piece to check.
 * \return true if the piece is possible, false otherwise.
 */
bool Board::isPositionPossible(int row, int col, const Piece& piece) const {
    if (piece.kind() == kNone) {
        // If the piece is kNone, it is not possible.
        return false;
    }
    // Get the shape of the piece.
    auto shape = piece.shape();
    // Iterate over the tiles of the piece.
    int index = 0;
    for (int pieceRow = 0; pieceRow < piece.bBoxSide(); ++pieceRow) {
        for (int pieceCol = 0; pieceCol < piece.bBoxSide(); ++pieceCol) {
           // If the tile of the piece is not empty and it is filled, return false.
            if (shape[index] != kEmpty && isTileFilled(row + pieceRow, col + pieceCol)) {
                return false;
            }

            // Increment the index.
            ++index;
        }
    }
    // If we have checked all the tiles of the piece and none of them are
    // filled, return true.
    return true;
}

/**
 * \brief Updates the ghost row position for the current piece.
 *
 * This function calculates the lowest possible row position (ghost row) 
 * for the current piece without any collisions. The ghost row is used 
 * to display a translucent outline of where the piece will land.
 */
void Board::updateGhostRow() {
    // Initialize ghostRow_ to the current row of the piece
    ghostRow_ = row_;
    // Increment ghostRow_ while the piece can be moved down without collision
    while (isPositionPossible(ghostRow_ + 1, col_, piece_)) {
        ++ghostRow_;
    }
}

/**
 * \brief Finds the lines to clear in the board.
 *
 * This function finds the lines in the board that need to be cleared and
 * stores them in the linesToClear_ vector. It also updates the tilesAfterClear_
 * vector with the tiles that will be remaining after clearing the lines.
 */
void Board::findLinesToClear() {
    linesToClear_.clear();
    tilesAfterClear_ = tiles_;

    int linesCleared = 0;
    int index = tiles_.size() - 1;
    for (int row = nRows - 1; row >= -kRowsAbove_; --row) {
        bool fullRow = true;
        for (int col = 0; col < nCols; ++col) {
            // Check if the tile at the given row and column is filled.
            if (!isTileFilled(row, col)) {
                fullRow = false;
                break;
            }
        }
        if (fullRow) {
            // Add the row to the lines to clear.
            linesToClear_.push_back(row);
            linesCleared++;
            // Shift the index down by the number of columns.
            index -= nCols;
        } else if (linesCleared > 0) {
            // Shift the index up by the number of lines cleared.
            int indexShift = linesCleared * nCols;
            // Copy the tiles from the original board to the tiles after clear.
            for (int col = 0; col < nCols; ++col) {
                tilesAfterClear_[index + indexShift] = tiles_[index];
                --index;
            }
        } else {
            // Shift the index down by the number of columns.
            index -= nCols;
        }
    }
    // Fill the tiles above the cleared lines with empty tiles.
    std::fill(tilesAfterClear_.begin(), tilesAfterClear_.begin() + linesCleared * nCols, kEmpty);
}


const double Tetris::kMoveDelay_ = 0.05;
const double Tetris::kMoveRepeatDelay_ = 0.15;
const double Tetris::kSoftDropSpeedFactor_ = 20;
const double Tetris::kLockDownTimeLimit_ = 0.4;
const int Tetris::kLockDownMovesLimit_ = 15;
const double Tetris::kPauseAfterLineClear_ = 0.3;

/**
 * \brief Constructor for Tetris.
 *
 * This constructor initializes the Tetris object with the given board,
 * time step, and random seed.
 *
 * \param board The board to play on.
 * \param timeStep The time step to use for the game.
 * \param randomSeed The random seed to use for the game.
 */
Tetris::Tetris(Board& board, double timeStep, unsigned int randomSeed)
    : board_(board), timeStep_(timeStep), rng_(randomSeed), bag_(2 * kNumPieces), nextPiece_(0){
// Initialize the bag with the seven Tetris pieces.
    // The pieces are duplicated in the bag to ensure that the probability of
    // drawing a piece is uniform.
    bag_[0] = kPieceI;
    bag_[1] = kPieceJ;
    bag_[2] = kPieceL;
    bag_[3] = kPieceO;
    bag_[4] = kPieceS;
    bag_[5] = kPieceT;
    bag_[6] = kPieceZ;
    std::copy(bag_.begin(), bag_.begin() + kNumPieces, bag_.begin() + kNumPieces);
    // Initialize the game state.
    restart(1);
}

/**
 * \brief Resets the game state and restarts the game.
 *
 * This function resets the game state, clears the board, and restarts the game
 * with the given level, only one level is supported
 *
 * \param level The level to start the game with.
 */
static double secondsPerLineForLevel(int level) {
    return std::pow(0.8 - (level - 1) * 0.007, level - 1);
}


void Tetris::restart(int level) {
    // Clear the board.
    board_.clear();
    // Reset the game state.
    gameOver_ = false;
    level_ = level;
    secondsPerLine_ = secondsPerLineForLevel(level);
    linesCleared_ = 0;
    score_ = 0;
    motion_ = Motion::kNone;
    moveLeftPrev_ = false;
    moveRightPrev_ = false;
    // Reset the move timers.
    moveDownTimer_ = 0;
    moveRepeatTimer_ = 0;
    moveRepeatDelayTimer_ = 0;
    // Reset the locking timer.
    isOnGround_ = false;
    lockingTimer_ = 0;
    // Reset the paused for line clear flag and timer.
    pausedForLinesClear_ = false;
    linesClearTimer_ = 0;
    // Shuffle the bag with the seven Tetris pieces.
    std::shuffle(bag_.begin(), bag_.begin() + kNumPieces, rng_);
    std::shuffle(bag_.begin() + kNumPieces, bag_.end(), rng_);
    // Spawn a new piece.
    spawnPiece();
}

/**
 * @brief Updates the game state in response to user input.
 *
 * This function should be called once per frame. It processes user input,
 * updates the game state, and checks for line clears and the game over
 * condition.
 *
 * @param softDrop Whether the user is pressing the down arrow key. If true,
 *                 the piece will move down faster.
 * @param moveRight Whether the user is pressing the right arrow key. If true,
 *                  the piece will move right.
 * @param moveLeft Whether the user is pressing the left arrow key. If true,
 *                 the piece will move left.
 */
void Tetris::update(bool softDrop, bool moveRight, bool moveLeft) {
    // Check if the game is paused after a line clear.
    if (pausedForLinesClear_) {
        linesClearTimer_ += timeStep_;
        // If the pause is over, clear the lines, spawn a new piece, and unpause.
        if (linesClearTimer_ < kPauseAfterLineClear_) {
            return;
        }

        board_.clearLines();
        spawnPiece();
        pausedForLinesClear_ = false;
    }
    // Increment the move down timer.
    moveDownTimer_ += timeStep_;
    // Increment the move repeat timer and delay timer.
    moveRepeatTimer_ += timeStep_;
    moveRepeatDelayTimer_ += timeStep_;
    // If the piece is on the ground, increment the locking timer.
    if (isOnGround_) {
        lockingTimer_ += timeStep_;
    } else {
        lockingTimer_ = 0;
    }
    // Get the current state of the left and right arrow keys.
    bool moveLeftInput = moveLeft;
    bool moveRightInput = moveRight;
    // If both keys are pressed, prioritize the one that was pressed most
    // recently.
    if (moveLeft && moveRight) {
        if (!moveRightPrev_) {
            moveLeft = false;
        } else if (!moveLeftPrev_) {
            moveRight = false;
        } else if (motion_ == Motion::kLeft) {
            moveRight = false;
        } else {
            moveLeft = false;
        }
    }
    // Process the move right key press.
    if (moveRight) {
// If the piece is not moving right, move it right and reset the move
        // repeat timer and delay timer.
        if (motion_ != Motion::kRight) {
            moveRepeatDelayTimer_ = 0;
            moveRepeatTimer_ = 0;
            moveHorizontal(1);
// If the piece is moving right, check if the move repeat timer and
        // delay timer have expired. If so, move the piece right and reset
        // the timers.
        } else if (moveRepeatDelayTimer_ >= kMoveRepeatDelay_ && moveRepeatTimer_ >= kMoveDelay_) {
            moveRepeatTimer_ = 0;
            moveHorizontal(1);
        }
        // Set the motion to right.
        motion_ = Motion::kRight;
    // Process the move left key press.
    } else if (moveLeft) {
// If the piece is not moving left, move it left and reset the move
        // repeat timer and delay timer.
        if (motion_ != Motion::kLeft) {
            moveRepeatDelayTimer_ = 0;
            moveRepeatTimer_ = 0;
            moveHorizontal(-1);
     // If the piece is moving left, check if the move repeat timer and
        // delay timer have expired. If so, move the piece left and reset
        // the timers.
        } else if (moveRepeatDelayTimer_ >= kMoveRepeatDelay_ && moveRepeatTimer_ >= kMoveDelay_) {
            moveRepeatTimer_ = 0;
            moveHorizontal(-1);
        }
        // Set the motion to left.
        motion_ = Motion::kLeft;
    } else {
        // Set the motion to none.
        motion_ = Motion::kNone;
    }
    // Store the current state of the left and right arrow keys.
    moveLeftPrev_ = moveLeftInput;
    moveRightPrev_ = moveRightInput;
// Calculate the move down speed factor based on whether the user is
    // pressing the down arrow key.
    double speedFactor_ = softDrop ? kSoftDropSpeedFactor_ : 1;
// If the move down timer has expired, move the piece down and reset the
    // timer.
    if (moveDownTimer_ >= secondsPerLine_ / speedFactor_) {
        if (board_.moveVertical(1) && softDrop) {
        }
        moveDownTimer_ = 0;
    }
    // Check if the piece is locked.

    checkLock();
}

/**
 * \brief Moves the current piece horizontally.
 *
 * This function moves the current piece horizontally by the given number of
 * columns. If the move is possible, the piece is moved and the ghost row is
 * updated. Otherwise, the function returns false.
 *
 * \param dCol The number of columns to move the piece.
 */
void Tetris::moveHorizontal(int dCol) {
    if (board_.moveHorizontal(dCol) && isOnGround_) {
        // Reset the locking timer and increment the number of moves while
        // locking.
        lockingTimer_ = 0;
        nMovesWhileLocking_ += 1;
    }
}
/**
 * \brief Rotates the current piece by the given rotation.
 *
 * This function rotates the current piece by the given rotation. If the move
 * is possible, the piece is rotated and the ghost row is updated. Otherwise,
 * the function returns false.
 *
 * \param rotation The rotation of the piece.
 */
void Tetris::rotate(Rotation rotation) {
    // Rotate the piece.
    // If the piece is on the ground, reset the locking timer and increment
    // the number of moves while locking.
    if (board_.rotate(rotation) && isOnGround_) {
        lockingTimer_ = 0;
        nMovesWhileLocking_ += 1;
    }
    // Check if the piece is locked.
    checkLock();
}

/**
 * \brief Performs a hard drop of the current piece.
 *
 * This function instantly drops the current piece to the bottom of the
 * board, locking it in place. If there is no piece currently active,
 * the function exits without performing any action.
 */
void Tetris::hardDrop() {
        // Check if there is an active piece

    if (board_.piece().kind() == kNone) {
        return; // Exit if no active piece
    }
    score_ += 2 * level_ * board_.hardDrop();
    lock();
}


void Tetris::checkLock() {
    // Check if the piece is not on the ground
    if (!board_.isOnGround()) {
        isOnGround_ = false; // Update ground state
        return; // Exit if not on the ground
    }

    isOnGround_ = true; // Update ground state to on ground
    // Check if the locking criteria are met
    if (lockingTimer_ >= kLockDownTimeLimit_ || nMovesWhileLocking_ >= kLockDownMovesLimit_) {
        lock(); // Lock the piece in place
    }
}
/**
 * \brief Locks the current piece in place.
 *
 * This function locks the current piece in place, updating the ground state
 * and resetting the locking timer. It also checks if the game is over and
 * updates the game state accordingly.
 */
void Tetris::lock() {
    lockingTimer_ = 0;
    isOnGround_ = false;
    // Check if the piece was successfully locked
    if (!board_.frozePiece()) {
        gameOver_ = true;
        return;
    }
    // Check if there are any lines to clear
    // Spawn a new piece if there are no lines to clear
    if (board_.numLinesToClear() == 0) {
        spawnPiece();
        return;
    }
    pausedForLinesClear_ = true;
    linesClearTimer_ = 0;
}

/**
 * \brief Spawns a new Tetris piece.
 *
 * This function spawns a new Tetris piece and updates the game state
 * accordingly. It also checks if the game is over.
 */
void Tetris::spawnPiece() {
    // Check if the game is over
    gameOver_ = !board_.spawnPiece(bag_[nextPiece_]);
    // Update the bag index
    ++nextPiece_;
    if (nextPiece_ == kNumPieces) {
        // Shuffle the bag when all pieces have been spawned
        std::copy(bag_.begin() + kNumPieces, bag_.end(), bag_.begin());
        std::shuffle(bag_.begin() + kNumPieces, bag_.end(), rng_);
        nextPiece_ = 0;
    }
    // Reset the number of moves while locking
    nMovesWhileLocking_ = 0;
}
