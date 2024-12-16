/*

    It’s implementation wraps the fundamental elements of a Tetris game, from the loop to the rendering to user interaction. Variables 
set game structure including tile size, board size, and update/render timeouts for deterministic gameplay and layout. setupGlContext – 
Sets the OpenGL context, configures GLFW for window management, and the input callbacks handle the input of users for moving, rotating, 
dropping and changing game state (pause, restart). The main loop enables seamless operation by creating the game state periodically and 
fxing frames at predefined frame rates. Depending on the game state, the loop randomly toggles between the start screen with control, 
playing with piece animations, pausing the overlay, or playing over screen. Modular Classes such as BoardRenderer, PieceRenderer, 
TextRenderer, etc. renders game objects, active and ghost pieces, and text instructions or messages. Shaders and loadable textures make 
the game look great and multithreading helps with the timing for updates and rendering so the game looks sleek and responsive.

*/

#include <string>
#include <vector>
#include <thread>
#include <glm/gtc/matrix_transform.hpp>
#include "render.h"

const GLfloat kTileSize = 32;
const GLint kBoardNumRows = 20;
const GLint kBoardNumCols = 10;
const GLfloat kBoardWidth = kBoardNumCols * kTileSize;
const GLfloat kBoardHeight = kBoardNumRows * kTileSize;
const GLfloat kMargin = 30;
const GLfloat kHudWidth = -25;
const GLfloat kWidth = 3 * kMargin + kBoardWidth + kHudWidth;
const GLfloat kHeight = 2 * kMargin + kBoardHeight;
const GLfloat kBoardX = 2 * kMargin + kHudWidth;
const GLfloat kBoardY = kMargin;
const GLuint kFontSize = 18;

const double kGameTimeStep = 0.005;
const double kFps = 30;
const double kSecondsPerFrame = 1.0 / kFps;

Board board(kBoardNumRows, kBoardNumCols);
Tetris* tetris;

enum GameState { kGameStart, kGameRun, kGamePaused, kGameOver };
GameState gameState = kGameStart;

bool softDrop = false;
bool moveRight = false;
bool moveLeft = false;

/**
 * Sets up an OpenGL context using GLFW and GLEW.
 *
 * @return A pointer to the created GLFWwindow, or nullptr if initialization fails.
 */

GLFWwindow* setupGlContext() {
    // Initialize GLFW
    if (!glfwInit()) {
        return nullptr; // Return nullptr if GLFW initialization fails
    }

    // Configure GLFW window settings
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE); // Disable window resizing
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // Set OpenGL major version to 3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2); // Set OpenGL minor version to 2
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Use core profile
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Enable forward compatibility
    GLFWwindow* window = glfwCreateWindow(kWidth, kHeight, "TETRIS", nullptr, nullptr);

    if (window == nullptr) {
        glfwTerminate(); // Terminate GLFW if window creation fails
    }

    // Make the window's OpenGL context current
    glfwMakeContextCurrent(window);
    // Initialize GLEW
    glewInit();

    return window; // Return the created window
}

/**
 * @brief Handles key presses and releases.
 *
 * @param window The GLFW window associated with the callback.
 * @param key The key that was pressed or released.
 * @param scancode The platform-specific scan code for the key.
 * @param action The action to take for the key press/release.
 * @param mods The modifier keys currently pressed.
 */
void keyCallback(GLFWwindow* /*window*/, int key, int /*scancode*/, int action, int /*mods*/) {
    switch (gameState) {
    case kGameRun:
        if (action == GLFW_PRESS) {
            switch (key) {
            case GLFW_KEY_Z: tetris->rotate(Rotation::kLeft); break;
            case GLFW_KEY_X: tetris->rotate(Rotation::kRight); break;
            case GLFW_KEY_SPACE: tetris->hardDrop(); break;
            case GLFW_KEY_LEFT: moveLeft = true; break;
            case GLFW_KEY_RIGHT: moveRight = true; break;
            case GLFW_KEY_DOWN: softDrop = true; break;
            case GLFW_KEY_ESCAPE: gameState = kGamePaused;
            }
        } else if (action == GLFW_RELEASE) {
            switch (key) {
            case GLFW_KEY_LEFT: moveLeft = false; break;
            case GLFW_KEY_RIGHT: moveRight = false; break;
            case GLFW_KEY_DOWN: softDrop = false;
            }
        }
        break;
    case kGamePaused:
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            gameState = kGameRun;
        } else if (key == GLFW_KEY_ENTER && action == GLFW_PRESS) {
            gameState = kGameStart;
        }
        break;
    case kGameOver:
        if (key == GLFW_KEY_ENTER && action == GLFW_PRESS) {
            gameState = kGameStart;
        }
        break;
    case kGameStart:
        if (key == GLFW_KEY_ENTER && action == GLFW_PRESS) {
            moveRight = false;
            moveLeft = false;
            softDrop = false;
            gameState = kGameRun;
        }
    }
}

/**
 * @brief GLFW window focus callback.
 *
 * If the window loses focus and the game was running, pause the game.
 *
 * @param window The GLFWwindow object.
 * @param focused GLFW_TRUE if the window gained focus, GLFW_FALSE otherwise.
 */
void windowFocusCallback(GLFWwindow* /*window*/, int focused) {
    if (!focused && gameState == kGameRun) {
        gameState = kGamePaused;
    }
}

int main() {
    // Initialize OpenGL context
    GLFWwindow* window = setupGlContext();

    if (window == nullptr) {
        return EXIT_FAILURE; // Exit if window creation fails
    }
    // Load font for text rendering
    auto font = loadFont("resources/font.ttf", kFontSize);
    // Load textures for tiles and ghost pieces
    std::vector<Texture> tileTextures, ghostTextures;
    std::vector<std::string> colors = {"cyan", "blue", "orange", "yellow", "green", "purple", "red"};
    for (int color = kCyan; color <= kRed; ++color) {
        std::string path = "resources/block_" + colors[color] + ".png";
        tileTextures.push_back(loadRgbaTexture(path));
        path = "resources/ghost_" + colors[color] + ".png";
        ghostTextures.push_back(loadRgbaTexture(path));
    }

    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Create projection matrix for rendering
    glm::mat4 projection = glm::ortho(0.0f, kWidth, kHeight, 0.0f, -1.0f, 1.0f);
    // Initialize Tetris game object
    tetris = new Tetris(board, kGameTimeStep, static_cast<unsigned int>(glfwGetTime() * 1e4));
    // Set up input callbacks
    glfwSetKeyCallback(window, keyCallback);
    glfwSetWindowFocusCallback(window, windowFocusCallback);
    // Create text renderer
    TextRenderer textRenderer(projection, font);
    // Compute dimensions for text rendering
    GLfloat letterHeight = textRenderer.computeHeight("A");
    // Create sprite and piece renderers
    SpriteRenderer spriteRenderer(projection);
    PieceRenderer pieceRenderer(kTileSize, tileTextures, spriteRenderer);
    PieceRenderer ghostRenderer(kTileSize, ghostTextures, spriteRenderer);
    BoardRenderer boardRenderer(projection, kTileSize, kBoardX, kBoardY, kBoardNumRows, kBoardNumCols, tileTextures,
                                spriteRenderer, pieceRenderer, ghostRenderer);
    // Timing variables for game loop
    double timeLastGameUpdate = 0;
    double timeLastRender = 0;
    // Main game loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        std::this_thread::sleep_for(std::chrono::duration<double>(timeLastGameUpdate + kGameTimeStep - glfwGetTime()));
        // Update game state
        if (gameState == kGameRun) {
            tetris->update(softDrop, moveRight, moveLeft);
            if (tetris->isGameOver()) {
                gameState = kGameOver;
            }
        }
        timeLastGameUpdate = glfwGetTime();
        // Render frame
        double time = glfwGetTime();
        if (time - timeLastRender >= kSecondsPerFrame) {
            timeLastRender = time;
            glClearColor(0, 0, 0, 1);
            glClear(GL_COLOR_BUFFER_BIT);
            // Determine lines cleared for rendering
            int linesCleared;
            if (gameState == kGameStart) {
                linesCleared = 0;
            } else {
                linesCleared = tetris->linesCleared();
            }
            // Render based on game state
            boardRenderer.renderBackground();
            switch (gameState) {
            case kGameRun:
                boardRenderer.renderTiles(board);
                if (!tetris->isPausedForLinesClear()) {
                    boardRenderer.renderGhost(board.piece(), board.ghostRow(), board.pieceCol());
                    boardRenderer.renderPiece(board.piece(), board.pieceRow(), board.pieceCol(), tetris->lockPercent());
                }
                break;
            case kGamePaused: {
                boardRenderer.renderTiles(board, 0.4);
                boardRenderer.renderPiece(board.piece(), board.pieceRow(), board.pieceCol(), 0, 0.4);

                GLfloat y = kBoardY + 0.38f * kBoardHeight;

                textRenderer.renderCentered("PAUSED", kBoardX, y, kBoardWidth, kColorWhite);

                y = kBoardY + 0.5f * kBoardHeight;
                GLfloat xName = kBoardX + 0.1f * kBoardWidth;


                textRenderer.render("CONTINUE", xName, y, kColorWhite);

                y += 5.5f * letterHeight;
                textRenderer.render("START SCREEN", xName, y, kColorWhite);

                break;
            }
            case kGameStart: {
                GLfloat y = kBoardY + 0.05f * kBoardHeight;

                textRenderer.renderCentered("CONTROLS", kBoardX, y, kBoardWidth, kColorWhite);

                y += 4 * letterHeight;

                GLfloat xName = kBoardX + 0.1f * kBoardWidth;
                GLfloat dyBetweenRows = 3.8f * letterHeight;

                textRenderer.render("Press ENTER to Start", xName, y, kColorWhite);
                y += dyBetweenRows;
                textRenderer.render("Press ESC to Pause", xName, y, kColorWhite);
                y += dyBetweenRows;
                textRenderer.render("Press SPACE to Drop", xName, y, kColorWhite);
                y += dyBetweenRows;
                textRenderer.render("Press LEFT/RIGHT to Move", xName, y, kColorWhite);
                y += dyBetweenRows;
                textRenderer.render("Press UP to Rotate", xName, y, kColorWhite);
                y += dyBetweenRows;
                textRenderer.render("Press DOWN to Soft Drop", xName, y, kColorWhite);
                y += dyBetweenRows;
                textRenderer.render("Press Z/X to Rotate", xName, y, kColorWhite);
                break;
            }
            case kGameOver: {
                boardRenderer.renderTiles(board, 0.4);

                GLfloat y = kBoardY + 0.4f * kBoardHeight;
                GLfloat xName = kBoardX + 0.1f * kBoardWidth;
                textRenderer.render("Game Over :(", xName, y, kColorWhite);
            }
            }
            glfwSwapBuffers(window);
        }
    }

    return EXIT_SUCCESS;
}
