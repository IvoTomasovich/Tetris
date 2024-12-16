/*
   This implementation file exposes the rendering code for Tetris game, which drew tiles, pieces, the board and text with OpenGL shaders, 
textures and geometric primitives. A few shaders are declared for different rendering functions like colored primitives, textured tiles 
and glyphs for text. Each shader class controls the translation of vertices and coloration of fragments as well as color mixing, 
transparency of alpha and dynamic texturing. SpriteRenderer is the base layer for textured quads and it uses shaders for flexibility. 
It initializes vertex data to set the quad geometry and a render method to draw textures in defined positions with scaling, color 
blending, and transparency. To supplement this, the PieceRenderer class can render Tetris pieces using tile textures and show current 
and initial shapes (for alignment and blending). BoardRenderer class to render the entire Tetris board. It loads the grid and background 
geometry, coloured them in shaders, and sets board tiles and active pieces on them. It even allows drawing ghost pieces 
(to indicate where the currently active piece will land). RenderBackground will draw the background 
and lines of the board and renderTiles, renderPiece and renderGhost are for rendering game objects with transitions and visual clues. 
TextRenderer : Text renders with bitmap fonts in the form of Glyph objects. It automatically computes vertex information for each 
character based on position, texture and size and so is very accurate to place. Render: Render method creates text with a particular 
position and renderCentered aligns text with a specific width. This class even has utility methods to compute text dimensions, which 
are useful for layout modifications. This implementation implements OpenGL such as VAOs, VBOs, shaders and textures for a smooth and 
realistic rendering. It breaks down rendering logic into component modules that are tuned for a particular purpose, so that you have a 
scalable and easy-to-maintain rendering engine that supports dynamic animations, interaction and customization.

*/


#include <cmath>
#include "render.h"

const char* kColoredPrimitiveVertexShader = R"glsl(
# version 330 core

layout (location = 0) in vec2 position;
uniform mat4 projection;

void main() {
    gl_Position = projection * vec4(position, 0, 1);
}

)glsl";

const char* kColoredPrimitiveFragmentShader = R"glsl(
# version 330 core

uniform vec3 inColor;
out vec4 color;

void main() {
    color = vec4(inColor, 1);
}

)glsl";

const char* kTileVertexShader = R"glsl(
# version 330 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 texCoord;

out vec2 texCoordFragment;

uniform vec2 shift;
uniform vec2 scale = vec2(1, 1);
uniform mat4 projection;

void main() {
    gl_Position = projection * vec4(scale * position + shift, 0, 1);
    texCoordFragment = texCoord;
}
)glsl";

const char* kTileFragmentShader = R"glsl(

# version 330 core

in vec2 texCoordFragment;
out vec4 color;

uniform sampler2D sampler;
uniform vec3 mixColor;
uniform float mixCoeff = 0;
uniform float alphaMultiplier = 1;

void main() {
    color = mix(texture(sampler, texCoordFragment), vec4(mixColor, 1), mixCoeff);
    color.a *= alphaMultiplier;
}

)glsl";

const char* kGlyphVertexShader = R"glsl(

#version 330 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 texCoord;

out vec2 texCoordFragment;

uniform mat4 projection;

void main() {
    gl_Position = projection * vec4(position, 0, 1);
    texCoordFragment = texCoord;
}

)glsl";

const char* kGlyphFragmentShader = R"glsl(

#version 330 core

in vec2 texCoordFragment;
out vec4 color;

uniform vec3 textColor;
uniform sampler2D glyph;

void main() {
    float alpha = texture(glyph, texCoordFragment).r;
    color = vec4(textColor, alpha);
}

)glsl";

const glm::vec3 kColorBlack(0, 0, 0);
const glm::vec3 kColorWhite(1, 1, 1);


/**
 * \brief Constructor for SpriteRenderer.
 * \param projection Projection matrix.
 *
 * Loads the sprite rendering shader, creates a vertex buffer object, and
 * configures the vertex attributes.
 */
SpriteRenderer::SpriteRenderer(const glm::mat4& projection) : shader_(kTileVertexShader, kTileFragmentShader) {
    // The vertices array is a list of 2D coordinates and their corresponding
    // texture coordinates.
    GLfloat vertices[] = {0, 0, 0, 1, // bottom left
    0, 1, 0, 0, // top left
    1, 0, 1, 1,  // top right
    1, 1, 1, 0}; // bottom right

    GLuint vbo;
    glGenBuffers(1, &vbo);

    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*) 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*) (2 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    shader_.use();
    shader_.setMat4("projection", projection);
}

/**
 * \brief Renders a sprite with a given texture at a given position.
 *
 * The sprite is rendered as a quad with the bottom left corner at (x, y) and
 * size (width, height).
 *
 * The sprite is rendered with a color that is a linear combination of the
 * color of the texture and the given mixColor. The mixCoeff parameter controls
 * how much of the texture color is used in the linear combination.
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
 * \param alphaMultiplier The multiplier used to compute the final alpha value
 *                        of the rendered pixels.
 */
void SpriteRenderer::render(const Texture& texture, GLfloat x, GLfloat y, GLfloat width, GLfloat height,
                            GLfloat mixCoeff, const glm::vec3& mixColor, GLfloat alphaMultiplier) {
    texture.bind();
    shader_.use();
    shader_.setVec2("shift", glm::vec2(x, y));
    shader_.setVec2("scale", glm::vec2(width, height));
    shader_.setFloat("mixCoeff", mixCoeff);
    shader_.setVec3("mixColor", mixColor);
    shader_.setFloat("alphaMultiplier", alphaMultiplier);
    glBindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

/**
 * \brief Renders a piece on the board.
 *
 * This function renders a piece at a given position on the board. The
 * piece is rendered as a set of tiles with the given color and a linear
 * combination of the color of the texture and mixColor. The mixCoeff
 * parameter controls how much of the texture color is used in the
 * linear combination.
 *
 * The alpha value of the rendered pixels is multiplied by alphaMultiplier.
 *
 * The startRow parameter is used to render only a subset of the tiles of
 * the piece.
 *
 * \param piece The piece to render.
 * \param x The x-coordinate of the bottom left corner of the piece.
 * \param y The y-coordinate of the bottom left corner of the piece.
 * \param mixCoeff The coefficient used to compute the linear combination of
 *                 the texture color and the mixColor.
 * \param mixColor The color used to compute the linear combination of the
 *                 texture color and the mixColor.
 * \param alphaMultiplier The multiplier used to compute the final alpha value
 *                        of the rendered pixels.
 * \param startRow The first row of the piece to render.
 */
void PieceRenderer::renderShape(const Piece& piece, GLfloat x, GLfloat y, GLfloat mixCoeff, const glm::vec3& mixColor,
                                GLfloat alphaMultiplier, int startRow) const {
    if (piece.kind() == kNone) {
        return;
    }

    Texture texture = textures_.at(piece.color());

    int index = startRow * piece.bBoxSide();
    auto shape = piece.shape();
    for (int row = startRow; row < piece.bBoxSide(); ++row) {
        for (int col = 0; col < piece.bBoxSide(); ++col) {
            if (shape[index] != kEmpty) {
                spriteRenderer_.render(texture, x + col * tileSize_, y + row * tileSize_, tileSize_, tileSize_,
                                       mixCoeff, mixColor, alphaMultiplier);
            }

            ++index;
        }
    }
}

/**
 * Renders the initial shape of a piece.
 *
 * This function renders the initial shape of a piece at the specified
 * position.
 *
 * \param piece The piece to render.
 * \param x The x-coordinate of the bottom left corner of the piece.
 * \param y The y-coordinate of the bottom left corner of the piece.
 */
void PieceRenderer::renderInitialShape(const Piece& piece, GLfloat x, GLfloat y) const {
    if (piece.kind() == kNone) {
        return;
    }

    Texture texture = textures_.at(piece.color());

    int index = 0;
    auto shape = piece.initialShape();
    for (int row = 0; row < piece.nRows(); ++row) {
        for (int col = 0; col < piece.nCols(); ++col) {
            if (shape[index] != kEmpty) {
                // Render the tile at the specified position with the given
                // texture.
                spriteRenderer_.render(texture, x + col * tileSize_, y + row * tileSize_, tileSize_, tileSize_);
            }

            ++index;
        }
    }
}

/**
 * Renders the initial shape of a piece centered within a given area.
 *
 * This function calculates the necessary shifts to center the initial shape
 * of a piece within a specified width and height, then renders it.
 *
 * \param piece The piece to render.
 * \param x The x-coordinate of the bottom left corner of the area to center in.
 * \param y The y-coordinate of the bottom left corner of the area to center in.
 * \param width The width of the area to center the piece in.
 * \param height The height of the area to center the piece in.
 */
void PieceRenderer::renderInitialShapeCentered(const Piece& piece, GLfloat x, GLfloat y, GLfloat width,
                                               GLfloat height) const {
    // Calculate the width and height of the piece in pixels                                                
    GLfloat pieceWidth = tileSize_ * piece.nCols();
    GLfloat pieceHeight = tileSize_ * piece.nRows();
    // Calculate the horizontal and vertical shift required to center the piece
    GLfloat xShift = 0.5f * (width - pieceWidth);
    GLfloat yShift = 0.5f * (height - pieceHeight);
    // Render the piece at the centered position
    renderInitialShape(piece, x + xShift, y + yShift);
}

const glm::vec3 kBackgroundColor(0.05, 0.05, 0.05);
const glm::vec3 kGridColor(0.4, 0.4, 0.4);

/**
 * \brief A class that renders a Tetris board.
 *
 * This class encapsulates the state and behavior necessary to render a Tetris
 * board. It includes the projection matrix, the tile size, the position of the
 * board, and the number of rows and columns of the board. It also contains the
 * necessary textures and shaders for rendering the board and the pieces.
 */
BoardRenderer::BoardRenderer(const glm::mat4& projection, GLfloat tileSize, GLfloat x, GLfloat y, int nRows, int nCols,
                             const std::vector<Texture>& tileTextures, SpriteRenderer& spriteRenderer,
                             PieceRenderer& pieceRenderer, PieceRenderer& ghostRenderer)
    : tileSize_(tileSize)
    , x_(x)
    , y_(y)
    , nRows_(nRows)
    , nCols_(nCols)
    , tileTextures_(tileTextures)
    , pieceRenderer_(pieceRenderer)
    , ghostRenderer_(ghostRenderer)
    , spriteRenderer_(spriteRenderer)
    , backgroundShader_(kColoredPrimitiveVertexShader, kColoredPrimitiveFragmentShader) {
    backgroundShader_.use();
    backgroundShader_.setMat4("projection", projection);

    // Calculate the width and height of the board in pixels
    GLfloat width = nCols_ * tileSize_;
    GLfloat height = nRows_ * tileSize_;


    /**
     * The vertices of the background of the board. The order of the vertices
     * is:
     * 1. Top left
     * 2. Top right
     * 3. Bottom right
     * 4. Bottom left
     */
    verticesBackground_ = {x_, y_, x_, y_ + height, x_ + width, y_, x_ + width, y_ + height};
    // The y-coordinate of the top of the grid
    GLfloat yGrid = y_;
    for (int row = 0; row < nRows_ + 1; ++row) {
        // The vertices of the horizontal grid lines
        verticesBackground_.push_back(x_);
        verticesBackground_.push_back(yGrid);
        verticesBackground_.push_back(x_ + width);
        verticesBackground_.push_back(yGrid);
        yGrid += tileSize_;
    }
    // The x-coordinate of the left of the grid
    GLfloat xGrid = x_;
    for (int col = 0; col < nCols_ + 1; ++col) {
        // The vertices of the vertical grid lines
        verticesBackground_.push_back(xGrid);
        verticesBackground_.push_back(y_);
        verticesBackground_.push_back(xGrid);
        verticesBackground_.push_back(y_ + height);
        xGrid += tileSize_;
    }

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glGenVertexArrays(1, &vaoBackground_);

    glBindVertexArray(vaoBackground_);
    // Bind the VBO to the VAO
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    // Set the data in the VBO
    glBufferData(GL_ARRAY_BUFFER, verticesBackground_.size() * sizeof(GLfloat), verticesBackground_.data(),
                 GL_STATIC_DRAW);
    // Set the vertex attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*) 0);
    // Enable the vertex attribute
    glEnableVertexAttribArray(0);
    // Unbind the VAO
    glBindVertexArray(0);
}

/**
 * \brief Renders the background of the Tetris board.
 *
 * This function uses the background shader to render the background
 * and grid lines of the Tetris board. The background is rendered as
 * a quad with a solid color, and the grid lines are rendered as lines
 * with a different color.
 */
void BoardRenderer::renderBackground() const {
    // Activate the background shader
    backgroundShader_.use();
    // Bind the vertex array object for the background
    glBindVertexArray(vaoBackground_);
    // Set the background color and render the background quad
    backgroundShader_.setVec3("inColor", kBackgroundColor);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    // Set the grid color and render the grid lines
    backgroundShader_.setVec3("inColor", kGridColor);
    glDrawArrays(GL_LINES, 4, 2 * (nRows_ + nCols_ + 2));
}

/**
 * \brief Renders the tiles of the Tetris board.
 *
 * This function renders the tiles of the Tetris board as sprites. The
 * sprite renderer is used to render the tiles, and the tile textures are
 * stored in the tileTextures_ map. The tiles are rendered in the order
 * they appear in the board, from top to bottom and left to right.
 *
 * \param board The board to render
 * \param alphaMultiplier The alpha multiplier to apply to the tiles
 */
void BoardRenderer::renderTiles(const Board& board, GLfloat alphaMultiplier) const {
    int row, col;
    GLfloat y, x;
    GLfloat y0 = y_;
    GLfloat x0 = x_;
    // Iterate over the rows of the board
    for (row = 0, y = y0; row < board.nRows; ++row, y += tileSize_) {
        // Iterate over the columns of the board
        for (col = 0, x = x0; col < board.nCols; ++col, x += tileSize_) {
            // Get the tile color at the current position
            TileColor tile = board.tileAt(row, col);
            // If the tile is empty, skip it
            if (tile == kEmpty) {
                continue;
            }
            // Render the tile sprite
            spriteRenderer_.render(tileTextures_.at(tile), x, y, tileSize_, tileSize_, 0, glm::vec3(), alphaMultiplier);
        }
    }
}

/**
 * \brief Renders a piece on the board.
 *
 * This function renders a Tetris piece at a specified board position. The
 * rendering takes into account the locking progress of the piece and applies
 * an alpha multiplier for transparency.
 *
 * \param piece The piece to render.
 * \param row The row index where the piece is to be rendered.
 * \param col The column index where the piece is to be rendered.
 * \param lockPercent The percentage of the locking process completed for the piece.
 * \param alphaMultiplier The alpha multiplier to apply for transparency.
 */
void BoardRenderer::renderPiece(const Piece& piece, int row, int col, double lockPercent,
                                double alphaMultiplier) const {
    // Calculate the starting row for rendering, ensuring it is within bounds
    int startRow = std::max(0, -row);
    // Compute the mix coefficient based on the locking percentage
    GLfloat mixCoeff = 0.5f * sin(M_PI_2 * lockPercent);
    // Render the shape of the piece using the piece renderer
     pieceRenderer_.renderShape(piece, x_ + col * tileSize_, y_ + row * tileSize_, mixCoeff, kColorBlack,
                                alphaMultiplier, startRow);
}

/**
 * \brief Renders the ghost of a piece on the board.
 *
 * This function renders the ghost of a piece at a specified board position,
 * taking into account the locking progress of the piece and applying an
 * alpha multiplier for transparency.
 *
 * \param piece The piece to render.
 * \param ghostRow The row index where the ghost is to be rendered.
 * \param col The column index where the ghost is to be rendered.
 */
void BoardRenderer::renderGhost(const Piece& piece, int ghostRow, int col) const {
    int startRow = std::max(0, -ghostRow);
    // Use the ghost renderer to render the ghost of the piece with a ghostly
    // alpha value of 0.7.
    ghostRenderer_.renderShape(piece, x_ + col * tileSize_, y_ + ghostRow * tileSize_, 0, kColorBlack, 0.7, startRow);
}


/**
 * \brief Constructor for TextRenderer.
 *
 * Initializes the text rendering shader, vertex array object, and 
 * vertex buffer object for rendering text glyphs.
 *
 * \param projection Projection matrix for rendering.
 * \param font The font glyphs used for rendering text.
 */
TextRenderer::TextRenderer(const glm::mat4& projection, const std::vector<Glyph>& font)
    : font_(font), shader_(kGlyphVertexShader, kGlyphFragmentShader) {
    // Use the shader program
    shader_.use();
    // Set the projection matrix uniform
    shader_.setMat4("projection", projection);
    // Generate and bind a Vertex Array Object
    glGenVertexArrays(1, &vao_);
    // Generate and bind a Vertex Buffer Object
    glGenBuffers(1, &vbo_);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    // Allocate memory for the vertex buffer (dynamic draw for updates)
    glBufferData(GL_ARRAY_BUFFER, 4 * 4 * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);
    // Define vertex attribute layout
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*) 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*) (2 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    // Unbind the buffer and array to prevent accidental modification
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

/**
 * \brief Renders a string of text at a given position using the given font.
 *
 * This function renders a string of text at a given position using the given
 * font. The text is rendered in a single color specified by the color
 * parameter.
 *
 * The text is rendered using a vertex array object and a vertex buffer
 * object. The vertex data is dynamic and is updated each time the function is
 * called.
 *
 * \param text The string of text to render.
 * \param x The x-coordinate of the top left corner of the text.
 * \param y The y-coordinate of the top left corner of the text.
 * \param color The color of the text.
 */
void TextRenderer::render(const std::string& text, GLfloat x, GLfloat y, glm::vec3 color) const {
    // Use the shader program
    shader_.use();
    // Set the text color uniform
    shader_.setVec3("textColor", color);
    // Bind the vertex array object
    glBindVertexArray(vao_);
    // Round the coordinates to the nearest integer
    x = std::round(x);
    y = std::round(y);
    // Render each character of the string
    for (char c : text) {
        Glyph glyph = font_.at(c);
        // Calculate the bounding box of the glyph
        GLfloat xBbox = x + glyph.bearing.x;
        GLfloat yBbox = y + (font_.at('A').bearing.y - glyph.bearing.y);

        GLfloat width = glyph.texture.width;
        GLfloat height = glyph.texture.height;
        // Create the vertex data for the glyph
        GLfloat vertices[] = {xBbox,         yBbox, 0, 0, xBbox,         yBbox + height, 0, 1,
                              xBbox + width, yBbox, 1, 0, xBbox + width, yBbox + height, 1, 1};
        // Bind the glyph texture
        glyph.texture.bind();
        // Bind the vertex buffer object
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        // Update the vertex data
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        // Unbind the vertex buffer object
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // Draw the glyph
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        // Update the x-coordinate for the next glyph
        x += glyph.advance;
    }
}

/**
 * \brief Renders a string of text centered within a given area.
 *
 * This function renders a string of text centered within a given area. The
 * text is rendered in a single color specified by the color parameter.
 *
 * The text is rendered using a vertex array object and a vertex buffer
 * object. The vertex data is dynamic and is updated each time the function is
 * called.
 *
 * The x-coordinate of the text is shifted so that the text is centered within
 * the area. The y-coordinate of the text is rounded to the nearest integer.
 *
 * \param text The string of text to render.
 * \param x The x-coordinate of the center of the area to center in.
 * \param y The y-coordinate of the top of the area to center in.
 * \param width The width of the area to center the text in.
 * \param color The color of the text.
 */
void TextRenderer::renderCentered(const std::string& text, GLfloat x, GLfloat y, GLfloat width,
                                  const glm::vec3& color) const {
    // Compute the width of the text
    GLfloat textWidth = computeWidth(text);
    // Calculate the shift to center the text
    GLfloat shift = 0.5f * (width - textWidth);
    // Render the text at the shifted position
    render(text, std::round(x + shift), std::round(y), color);
}

/**
 * \brief Computes the width of the text in pixels.
 *
 * This function computes the width of the given text in pixels by summing the
 * widths of all the glyphs in the string. The width of a glyph is the width of
 * the glyph's texture plus the advance of the glyph.
 *
 * The width of the last glyph in the string is special-cased because we don't
 * add the advance of the last glyph to the width of the string.
 *
 * \param text The string of text to compute the width of.
 * \return The width of the text in pixels.
 */
GLint TextRenderer::computeWidth(const std::string& text) const {
    GLint width = 0;
    for (auto c = text.begin(); c != text.end() - 1; ++c) {
        width += font_.at(*c).advance;
    }
    width += font_.at(text.back()).texture.width;
    return width;
}

/**
 * \brief Computes the height of the text in pixels.
 *
 * This function computes the maximum height of the given text in pixels by
 * examining the heights of all the glyphs in the string. The height of a glyph
 * is determined by its texture height and its vertical bearing relative to 
 * a reference glyph (e.g., 'H').
 *
 * \param text The string of text to compute the height of.
 * \return The maximum height of the text in pixels.
 */
GLint TextRenderer::computeHeight(const std::string& text) const {
    GLint height = 0;
    // Iterate through each character in the text
    for (char c : text) {
        Glyph glyph = font_.at(c); // Retrieve the glyph for the character
        // Calculate the texture height of the glyph
        auto textureHeight = static_cast<GLint>(glyph.texture.height);
        // Update the maximum height based on the current glyph's dimensions
        height = std::max(height, font_.at('H').bearing.y - glyph.bearing.y + textureHeight);
    }
    return height; // Return the computed maximum height
}

