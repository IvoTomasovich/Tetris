/*
    The implementation file defines the Shader, Texture and Glyph classes as well as the load textures and fonts functions. It contains 
a few libraries, like FreeType for font rendering (ft2build.h) and STB Image for image loading (stb_image.h), which is very important 
for resource management of an OpenGL app. Shader constructor Build and link vertex and fragment shader program, flag errors during 
each step, and saves the program ID. The Texture class constructor creates an OpenGL texture object, its parameters (wrapping and 
filtering) and loads the image. LoadRgbaTexture method is called by STB Image to read RGBA image from a file and return Texture object. 
The loadFont function makes use of FreeType to grab a font from a location and create Glyph objects for the first 128 ASCII characters. 
It texture every glyph, with the bitmap of the glyph and store the bearing and advance width. The function return vector of these Glyph 
objects to be used in text rendering. It manages the resources in the correct way by releasing memory (stbi_image_free for images, 
FT_Done_FreeType for FreeType library). This is the code that allows basic rendering features such as texture on 2D surfaces and a 
dynamic font rendering in OpenGL.

*/

#include <string>
#include <iostream>
#include <vector>

#include "util.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "stb_image.h"

/**
 * \brief Constructor for Shader.
 *
 * Creates a new shader program by compiling a vertex and a fragment shader
 * and linking them together.
 *
 * \param sourceVertex The source code for the vertex shader.
 * \param sourceFragment The source code for the fragment shader.
 */
Shader::Shader(const GLchar* sourceVertex, const GLchar* sourceFragment) {
    // Create a shader of type GL_VERTEX_SHADER, attach the source code to it,
    // and compile it.
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &sourceVertex, NULL);
    glCompileShader(vertexShader);
    // Check for errors while compiling the vertex shader.
    GLchar infoLog[512];
    GLint success;

    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (success == 0) {
        // If the compilation failed, print the error message.
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "Error compiling vector shader: " << infoLog << std::endl;
    }
    // Do the same for the fragment shader.
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &sourceFragment, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (success == 0) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "Error compiling fragment shader: " << infoLog << std::endl;
    }
    // Create a program, attach both shaders to it, and link it.
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    // Check for errors while linking the program.
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success == 0) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cerr << "Error linking shader program: " << infoLog << std::endl;
    }
    // Delete the shaders since they are linked into the program.
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    // Store the program id.
    id_ = program;
}

/**
 * \brief Loads a texture from an image file.
 *
 * This function loads a texture from a file path, and returns it as a Texture
 * object. The image is loaded with 4 color channels, and the texture is
 * allocated with the same format.
 *
 * \param path The path to the image file to load.
 * \return The loaded texture.
 */
Texture loadRgbaTexture(const std::string& path) {
    stbi_set_flip_vertically_on_load(1);
    int width, height, numChannels;
    // Load the image from the given path, with 4 color channels.
    GLubyte* image = stbi_load(path.c_str(), &width, &height, &numChannels, 4);
    // Create a Texture object with the loaded image.
    Texture texture(GL_RGBA, width, height, image);
    // Free the loaded image.
    stbi_image_free(image);
    return texture;
}

/**
 * \brief A class that represents a texture.
 *
 * The texture is represented by its ID, width, and height.
 */
Texture::Texture(GLenum format, GLuint width, GLuint height, GLubyte* image) : width(width), height(height) {
    // Generate a texture ID.
    glGenTextures(1, &id_);
    // Bind the texture to the current context.
    glBindTexture(GL_TEXTURE_2D, id_);
    // Set the texture image data.
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, image);
    // Set the texture wrapping parameters.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // Set the texture minification and magnification filters.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

/**
 * \brief Loads a font from a file path.
 *
 * This function loads a font from the given file path, and returns it as a
 * vector of Glyph objects. The glyphs are stored in the order of their ASCII
 * values (from 0 to 127). The function uses the FreeType library to load the
 * font.
 *
 * \param path The path to the font file to load.
 * \param glyphHeight The height of the glyphs in pixels.
 * \return The loaded font.
 */
std::vector<Glyph> loadFont(const std::string& path, unsigned int glyphHeight) {
    // Initialize the FreeType library.
    FT_Library ft;
    FT_Init_FreeType(&ft);
    // Load the font face.
    FT_Face face;
    FT_New_Face(ft, path.c_str(), 0, &face);
    // Set the pixel size of the font.
    FT_Set_Pixel_Sizes(face, 0, glyphHeight);

    // Set the unpack alignment to 1.
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    // Create a vector of Glyph objects.
    std::vector<Glyph> glyphs;
    for (GLubyte c = 0; c < 128; c++) {
        // Load the glyph for the given character.
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            std::cerr << "Failed to load glyph " << c << "." << std::endl;
            continue;
        }
        // Create a Texture object for the glyph.
        Texture texture(GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, face->glyph->bitmap.buffer);
        // Create a Glyph object for the glyph.
        Glyph glyph = {texture, glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                       face->glyph->advance.x >> 6};
        // Add the glyph to the vector.
        glyphs.push_back(glyph);
    }
    // Clean up the FreeType library.
    FT_Done_FreeType(ft);
    // Return the vector of glyphs.
    return glyphs;
}
