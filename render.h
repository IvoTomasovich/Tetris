/*

    The tetris_render.h file is this Header file that specifies an effective OpenGL OpenGL render mechanism for a Tetris game which 
focuses on modularity and reusability to render pieces, board, and text of the game. The file starts with global constants for colors 
(kColorBlack and kColorWhite) to blend the effects in rendering. SpriteRenderer class provides the core rendering 2D textured quads 
that are color bounded and transparent and using a projection matrix to position and scale. On top of that, PieceRenderer does render 
individual Tetris pieces, with tile textures and a SpriteRenderer drawing shapes or starting structures, centering pieces or mixing 
with colors. BoardRenderer leverages these abilities to handle the whole board, render tiles, active and ghost pieces. In addition to these elements, TextRenderer can also render text in a 
bitmap font, and is used to accurately position and center text, along with calculating the text dimension for layout. All these 
classes depend on OpenGL objects such as shaders, VAOs and VBOs to render effectively. This refactors rendering logic away from the 
game so that you can render visually and dynamically (performance wise) for a fluid Tetris experience.

*/

#ifndef TETRIS_RENDER_H
#define TETRIS_RENDER_H

#include <iostream>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include "tetris.h"
#include "util.h"

extern const glm::vec3 kColorBlack;
extern const glm::vec3 kColorWhite;


/**
 * \brief Class for rendering sprites.
 *
 * A sprite is a 2D graphic that is rendered on the screen at a given position
 * and size. The sprite is rendered as a quad with the given texture and color.
 */
class SpriteRenderer {
public:
 /**
     * \brief Constructor for SpriteRenderer.
     * \param projection Projection matrix.
     */
    explicit SpriteRenderer(const glm::mat4& projection);
    /**
     * \brief Renders a sprite with a given texture at a given position.
     *
     * The sprite is rendered as a quad with the given texture and color.
     *
     * The color of the sprite is a linear combination of the color of the
     * texture and the given mixColor. The mixCoeff parameter controls how
     * much of the texture color is used in the linear combination.
     *
     * The alpha value of the rendered pixels is multiplied by alphaMultiplier.
     *
     * \param texture The texture to render.
     * \param x The x-coordinate of the bottom left corner of the sprite.
     * \param y The y-coordinate of the bottom left corner of the sprite.
     * \param width The width of the sprite.
     * \param height The height of the sprite.
     * \param mixCoeff The coefficient used to compute the linear combination of
     *                 the texture color and the mixColor.
     * \param mixColor The color used to compute the linear combination of the
     *                 texture color and the mixColor.
     * \param alphaMultiplier The multiplier used to compute the final alpha
     *                        value of the rendered pixels.
     */
    void render(const Texture& texture, GLfloat x, GLfloat y, GLfloat width, GLfloat height, GLfloat mixCoeff = 0,
                const glm::vec3& mixColor = kColorBlack, GLfloat alphaMultiplier = 1);

private:
    Shader shader_;
    GLuint vao_ = 0;
};

/**
 * \brief Class for rendering Tetris pieces.
 *
 * This class encapsulates the state and behavior necessary for rendering Tetris
 * pieces. It uses a SpriteRenderer to render the sprites of the Tetris pieces.
 */

class PieceRenderer {
public:
 /**
     * \brief Constructor for PieceRenderer.
     * \param tileSize The size of the tiles in pixels.
     * \param textures The textures of the Tetris pieces.
     * \param spriteRenderer The sprite renderer to use.
     */
    PieceRenderer(GLfloat tileSize, const std::vector<Texture>& textures, SpriteRenderer& spriteRenderer)
        : tileSize_(tileSize), textures_(textures), spriteRenderer_(spriteRenderer) {}

    /**
     * \brief Renders a Tetris piece at a given position.
     *
     * The piece is rendered as a set of sprites. The color of the sprites is a
     * linear combination of the color of the texture and the given mixColor.
     * The mixCoeff parameter controls how much of the texture color is used in
     * the linear combination.
     *
     * The alpha value of the rendered pixels is multiplied by alphaMultiplier.
     *
     * \param piece The piece to render.
     * \param x The x-coordinate of the bottom left corner of the piece.
     * \param y The y-coordinate of the bottom left corner of the piece.
     * \param mixCoeff The coefficient used to compute the linear combination of
     *                 the texture color and the mixColor.
     * \param mixColor The color used to compute the linear combination of the
     *                 texture color and the mixColor.
     * \param alphaMultiplier The multiplier used to compute the final alpha
     *                        value of the rendered pixels.
     * \param startRow The first row of the piece to render.
     */
    void renderShape(const Piece& piece, GLfloat x, GLfloat y, GLfloat mixCoeff = 0,
                     const glm::vec3& mixColor = kColorBlack, GLfloat alphaMultiplier = 1, int startRow = 0) const;
     /**
     * \brief Renders the initial shape of a Tetris piece at a given position.
     *
     * This function renders the initial shape of a Tetris piece at the given
     * position.
     *
     * \param piece The piece to render.
     * \param x The x-coordinate of the bottom left corner of the piece.
     * \param y The y-coordinate of the bottom left corner of the piece.
     */
    void renderInitialShape(const Piece& piece, GLfloat x, GLfloat y) const;

/**
     * \brief Renders the initial shape of a Tetris piece centered at a given
     *        position.
     *
     * This function renders the initial shape of a Tetris piece centered at the
     * given position.
     *
     * \param piece The piece to render.
     * \param x The x-coordinate of the center of the piece.
     * \param y The y-coordinate of the center of the piece.
     * \param width The width of the piece.
     * \param height The height of the piece.
     */
    void renderInitialShapeCentered(const Piece& piece, GLfloat x, GLfloat y, GLfloat width, GLfloat height) const;

private:
    GLfloat tileSize_;
    std::vector<Texture> textures_;
    SpriteRenderer& spriteRenderer_;
};


/**
 * \class BoardRenderer
 *
 * \brief A class that renders a Tetris board.
 *
 * This class encapsulates the state and behavior necessary to render a Tetris
 * board. It includes the projection matrix, the tile size, the position of the
 * board, and the number of rows and columns of the board. It also contains the
 * necessary textures and shaders for rendering the board and the pieces.
 */
class BoardRenderer {
public:
/**
     * \brief Constructor for BoardRenderer.
     *
     * This constructor initializes the BoardRenderer object with the given
     * parameters.
     *
     * \param projection Projection matrix.
     * \param tileSize The size of each tile in the board.
     * \param x The x-coordinate of the bottom left corner of the board.
     * \param y The y-coordinate of the bottom left corner of the board.
     * \param nRows The number of rows in the board.
     * \param nCols The number of columns in the board.
     * \param tileTextures The textures used for rendering the tiles in the board.
     * \param spriteRenderer The sprite renderer used to render the tiles.
     * \param pieceRenderer The piece renderer used to render the pieces.
     * \param ghostRenderer The ghost renderer used to render the ghost pieces.
     */
    BoardRenderer(const glm::mat4& projection, GLfloat tileSize, GLfloat x, GLfloat y, int nRows, int nCols,
                  const std::vector<Texture>& tileTextures, SpriteRenderer& spriteRenderer,
                  PieceRenderer& pieceRenderer, PieceRenderer& ghostRenderer);
/**
     * \brief Renders the background of the board.
     *
     * This function renders the background of the board as a large square.
     */
    void renderBackground() const;
    /**
     * \brief Renders the tiles of the board.
     *
     * This function renders the tiles of the board as sprites. The
     * sprite renderer is used to render the tiles, and the tile textures are
     * stored in the tileTextures_ map. The tiles are rendered in the order
     * they appear in the board, from top to bottom and left to right.
     *
     * \param board The board to render.
     * \param alphaMultiplier The alpha multiplier to apply to the tiles.
     */
    void renderTiles(const Board& board, GLfloat alphaMultiplier = 1) const;
    /**
     * \brief Renders a piece on the board.
     *
     * This function renders a piece on the board. The piece is rendered as a
     * set of tiles with the given color and a linear combination of the color
     * of the texture and mixColor. The mixCoeff parameter controls how much
     * of the texture color is used in the linear combination.
     *
     * The alpha value of the rendered pixels is multiplied by alphaMultiplier.
     *
     * The startRow parameter is used to render only a subset of the tiles of
     * the piece.
     *
     * \param piece The piece to render.
     * \param row The row of the piece in the board.
     * \param col The column of the piece in the board.
     * \param lockPercent The percentage of the piece that is locked.
     * \param alphaMultiplier The multiplier used to compute the final alpha
     *                        value of the rendered pixels.
     */
    void renderPiece(const Piece& piece, int row, int col, double lockPercent, double alphaMultiplier = 1) const;
    /**
     * \brief Renders a ghost piece on the board.
     *
     * This function renders a ghost piece on the board. The ghost piece is
     * rendered as a set of tiles with the given color and a linear combination
     * of the color of the texture and mixColor. The mixCoeff parameter controls
     * how much of the texture color is used in the linear combination.
     *
     * The alpha value of the rendered pixels is multiplied by alphaMultiplier.
     *
     * The startRow parameter is used to render only a subset of the tiles of
     * the ghost piece.
     *
     * \param piece The ghost piece to render.
     * \param ghostRow The row of the ghost piece in the board.
     * \param col The column of the ghost piece in the board.
     * \param alphaMultiplier The multiplier used to compute the final alpha
     *                        value of the rendered pixels.
     */
    void renderGhost(const Piece& piece, int ghostRow, int col) const;

private:
/**
     * \brief The size of each tile in the board.
     */
    GLfloat tileSize_;
    /**
     * \brief The x-coordinate of the bottom left corner of the board.
     */
    GLfloat x_, y_;
    /**
     * \brief The number of rows in the board.
     */
    /**
     * \brief The number of columns in the board.
     */
    int nRows_, nCols_;
    /**
     * \brief The textures used for rendering the tiles in the board.
     */
    const std::vector<Texture> tileTextures_;
    /**
     * \brief The piece renderer used to render the pieces.
     */
    /**
     * \brief The ghost renderer used to render the ghost pieces.
     */
    PieceRenderer &pieceRenderer_, ghostRenderer_;
    /**
     * \brief The sprite renderer used to render the tiles.
     */
    SpriteRenderer& spriteRenderer_;
/**
     * \brief The shader used to render the background of the board.
     */
    Shader backgroundShader_;
    /**
     * \brief The vertices of the background of the board.
     */
    std::vector<GLfloat> verticesBackground_;
    /**
     * \brief The vertex array object used to render the background of the board.
     */
    GLuint vaoBackground_ = 0;
};

/**
 * \class TextRenderer
 * 
 * \brief A class responsible for rendering text using OpenGL.
 * 
 * This class utilizes a shader, vertex array object, and vertex buffer object
 * to render text glyphs on the screen. It supports rendering text at specific
 * positions and also centering text within a specified width.
 */
class TextRenderer {
public:
/**
     * \brief Constructor for TextRenderer.
     * 
     * Initializes the text renderer with a projection matrix and a set of font glyphs.
     * 
     * \param projection The projection matrix for rendering.
     * \param font The font glyphs used for rendering text.
     */
    TextRenderer(const glm::mat4& projection, const std::vector<Glyph>& font);
    /**
     * \brief Renders a string of text at a given position.
     * 
     * \param text The string of text to render.
     * \param x The x-coordinate of the top left corner of the text.
     * \param y The y-coordinate of the top left corner of the text.
     * \param color The color of the text.
     */
    void render(const std::string& text, GLfloat x, GLfloat y, glm::vec3 color) const;
    /**
     * \brief Renders a string of text centered within a specified width.
     * 
     * \param text The string of text to render.
     * \param x The x-coordinate of the center line of the text.
     * \param y The y-coordinate of the top of the text.
     * \param width The total width within which to center the text.
     * \param color The color of the text.
     */
    void renderCentered(const std::string& text, GLfloat x, GLfloat y, GLfloat width, const glm::vec3& color) const;
/**
     * \brief Computes the width of the given text string.
     * 
     * \param text The string of text to measure.
     * \return The width of the text in pixels.
     */
    GLint computeWidth(const std::string& text) const;
    /**
     * \brief Computes the height of the given text string.
     * 
     * \param text The string of text to measure.
     * \return The height of the text in pixels.
     */
    GLint computeHeight(const std::string& text) const;

private:
    std::vector<Glyph> font_; ///< The font glyphs used for rendering.
    Shader shader_; ///< The shader used for rendering text.
    GLuint vao_ = 0; ///< The vertex array object for rendering text.
    GLuint vbo_ = 0; ///< The vertex buffer object for rendering text.
};

#endif  // TETRIS_RENDER_H
