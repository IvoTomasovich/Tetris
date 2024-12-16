/*
    This header file Tetris_util.h provides utilities classes and functions for rendering and shader management in Tetris game 
or similar OpenGL project. It contains OpenGL (GL/glew.h) for graphics and GLM library for mathematical functions such as vector 
and matrix operations. Texture: An OpenGL texture object contains its width and height, and has a constructor that initializes 
the texture with the size and pixels information. It allows bind the texture with bind method, using private member id_ for the 
OpenGL texture ID. Shader class wraps an OpenGL shader program and exposes utilities for initializing many kinds of uniform 
variables (float, mat4, vec3, vec2) with names and values, and turning on the shader using use method. Shader program has private
member id_, it’s instantiated when you build a shader program from vertex and fragment shader code. It also defines the Glyph 
struct to implement font rendering which contains a Texture object for the image of the glyph, a glm::ivec2 for the bearing of 
the glyph (offset from the baseline) and a GLint64 for the advance width (distance to the next glyph). Utility methods are declared 
to load resources: loadFont reads a font from a file and extracts the glyphs into a vector of Glyph objects (the height of each glyph
is defined using the parameter glyphHeight) loadRgbaTexture reads an RGBA texture from a file. All together, this header gives you 
the basic components to manage textures, shaders and fonts in an OpenGL application with necessary abstractions for rendering.

*/

#ifndef TETRIS_UTIL_H
#define TETRIS_UTIL_H

#include <GL/glew.h>
#include "glm/glm.hpp"
#include <glm/gtc/type_ptr.hpp>

/**
 * \class Texture
 *
 * \brief A class that represents a texture.
 *
 * This class encapsulates the properties and behavior of an OpenGL texture,
 * including its ID, width, height, and methods to bind the texture.
 */
class Texture {
public:
    /// Width and height of the texture in pixels.
    const GLuint width, height;
    /**
     * \brief Default constructor.
     *
     * Initializes the texture with zero width and height.
     */
    Texture() : width(0), height(0) {};
    /**
     * \brief Parameterized constructor.
     *
     * Initializes the texture with the given format, width, height, and image data.
     *
     * \param format The format of the texture.
     * \param width The width of the texture.
     * \param height The height of the texture.
     * \param image The image data to initialize the texture with.
     */
    Texture(GLenum format, GLuint width, GLuint height, GLubyte* image);
/**
     * \brief Binds the texture.
     *
     * Binds the texture to the current OpenGL context.
     */
    void bind() const { glBindTexture(GL_TEXTURE_2D, id_); }

private:
    /// The ID of the texture.
    GLuint id_ = 0;
};

/**
 * \class Shader
 *
 * \brief A class that encapsulates the properties and behavior of a shader program.
 *
 * This class loads a vertex and fragment shader, links them into a shader program, and
 * provides methods to set uniforms and activate the shader program.
 */
class Shader {
public:
    /**
     * \brief Constructor.
     *
     * \param sourceVertex The source code for the vertex shader.
     * \param sourceFragment The source code for the fragment shader.
     */
    Shader(const GLchar* sourceVertex, const GLchar* sourceFragment);
    /**
     * \brief Sets a float uniform value.
     *
     * \param name The name of the uniform.
     * \param value The value to set the uniform to.
     */
    void setFloat(const GLchar* name, GLfloat value) const { glUniform1f(glGetUniformLocation(id_, name), value); }
    /**
     * \brief Sets a matrix uniform value.
     *
     * \param name The name of the uniform.
     * \param matrix The value to set the uniform to.
     */
    void setMat4(const GLchar* name, const glm::mat4& matrix) const {
        glUniformMatrix4fv(glGetUniformLocation(id_, name), 1, GL_FALSE, glm::value_ptr(matrix));
    }
    /**
     * \brief Sets a vector uniform value.
     *
     * \param name The name of the uniform.
     * \param vec The value to set the uniform to.
     */
    void setVec3(const GLchar* name, glm::vec3 vec) const {
        glUniform3f(glGetUniformLocation(id_, name), vec.x, vec.y, vec.z);
    }
    /**
     * \brief Sets a vector uniform value.
     *
     * \param name The name of the uniform.
     * \param vec The value to set the uniform to.
     */
    void setVec2(const GLchar* name, glm::vec2 vec) const {
        glUniform2f(glGetUniformLocation(id_, name), vec.x, vec.y);
    }
    /**
     * \brief Activates the shader program.
     */
    void use() const { glUseProgram(id_); }

private:
    /// The ID of the shader program.
    GLuint id_;
};

/**
 * \struct Glyph
 * 
 * \brief Represents a single glyph in a font.
 * 
 * This structure encapsulates the texture, bearing, and advance information
 * necessary for rendering a single glyph from a font.
 */
struct Glyph {
    /// The texture of the glyph.
    Texture texture;
    /// The offset from the baseline to the left/top of the glyph.
    glm::ivec2 bearing;
    /// The horizontal offset to the next glyph.
    GLint64 advance;
};

std::vector<Glyph> loadFont(const std::string& path, unsigned int glyphHeight);
Texture loadRgbaTexture(const std::string& path);

#endif  // TETRIS_UTIL_H
