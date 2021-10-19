/* Interactive Gym Environment and Education Kit (iGeek) @version 0.x
@link    https://github.com/KabukiStarship/iGeek.git
@file    /_Seams/02.OpenGLHelloWorld.inl
@author  Cale McCollough <https://cookingwithcale.org>
@license Copyright 2019 (C) Kabuki Starship <kabukistarship.com>; all rights
reserved (R). This Source Code Form is subject to the terms of the Mozilla
Public License, v. 2.0. If a copy of the MPL was not distributed with this file,
You can obtain one at <https://mozilla.org/MPL/2.0/>. */

//#include <_Config.h>

#include <SDL.h>

#include <fstream>
#include <iostream>
#include <string>
#undef main

#include <GL/glew.h>
#include <SDL_opengl.h>

#include <glm/glm.h>
#include <glm/gtc/matrix_transform.h>
#include <glm/gtc/type_ptr.h>
using namespace glm;

#include "Graphics/_Package.inl"
#include "shader.h"
using namespace _;

const GLint cScreenWidth = 800, cScreenHeight = 600;

int main(const char** args, int arg_count) {
  SDL_Init(SDL_INIT_EVERYTHING);

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                      SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

  SDL_Window* window = SDL_CreateWindow("OpenGL", 0, 0, cScreenWidth,
                                        cScreenHeight, SDL_WINDOW_OPENGL);

  SDL_GLContext context = SDL_GL_CreateContext(window);

  glewExperimental = GL_TRUE;

  if (glewInit() != GLEW_OK) {
    std::cout << "\n\nFailed to initialize GLEW.\n\n";
  }

  glViewport(0, 0, cScreenWidth, cScreenHeight);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  Shader shader_1("core.vs", "core.frag");

  // Set up vertex data (and buffer(s)) and attribute pointers
  // use with Orthographic Projection
  /*
  GLfloat vertexes[] = {
      -0.5f * 500, -0.5f * 500, -0.5f * 500,  0.0f, 0.0f,
      0.5f * 500, -0.5f * 500, -0.5f * 500,  1.0f, 0.0f,
      0.5f * 500,  0.5f * 500, -0.5f * 500,  1.0f, 1.0f,
      0.5f * 500,  0.5f * 500, -0.5f * 500,  1.0f, 1.0f,
      -0.5f * 500,  0.5f * 500, -0.5f * 500,  0.0f, 1.0f,
      -0.5f * 500, -0.5f * 500, -0.5f * 500,  0.0f, 0.0f,

      -0.5f * 500, -0.5f * 500,  0.5f * 500,  0.0f, 0.0f,
      0.5f * 500, -0.5f * 500,  0.5f * 500,  1.0f, 0.0f,
      0.5f * 500,  0.5f * 500,  0.5f * 500,  1.0f, 1.0f,
      0.5f * 500,  0.5f * 500,  0.5f * 500,  1.0f, 1.0f,
      -0.5f * 500,  0.5f * 500,  0.5f * 500,  0.0f, 1.0f,
      -0.5f * 500, -0.5f * 500,  0.5f * 500,  0.0f, 0.0f,

      -0.5f * 500,  0.5f * 500,  0.5f * 500,  1.0f, 0.0f,
      -0.5f * 500,  0.5f * 500, -0.5f * 500,  1.0f, 1.0f,
      -0.5f * 500, -0.5f * 500, -0.5f * 500,  0.0f, 1.0f,
      -0.5f * 500, -0.5f * 500, -0.5f * 500,  0.0f, 1.0f,
      -0.5f * 500, -0.5f * 500,  0.5f * 500,  0.0f, 0.0f,
      -0.5f * 500,  0.5f * 500,  0.5f * 500,  1.0f, 0.0f,

      0.5f * 500,  0.5f * 500,  0.5f * 500,  1.0f, 0.0f,
      0.5f * 500,  0.5f * 500, -0.5f * 500,  1.0f, 1.0f,
      0.5f * 500, -0.5f * 500, -0.5f * 500,  0.0f, 1.0f,
      0.5f * 500, -0.5f * 500, -0.5f * 500,  0.0f, 1.0f,
      0.5f * 500, -0.5f * 500,  0.5f * 500,  0.0f, 0.0f,
      0.5f * 500,  0.5f * 500,  0.5f * 500,  1.0f, 0.0f,

      -0.5f * 500, -0.5f * 500, -0.5f * 500,  0.0f, 1.0f,
      0.5f * 500, -0.5f * 500, -0.5f * 500,  1.0f, 1.0f,
      0.5f * 500, -0.5f * 500,  0.5f * 500,  1.0f, 0.0f,
      0.5f * 500, -0.5f * 500,  0.5f * 500,  1.0f, 0.0f,
      -0.5f * 500, -0.5f * 500,  0.5f * 500,  0.0f, 0.0f,
      -0.5f * 500, -0.5f * 500, -0.5f * 500,  0.0f, 1.0f,

      -0.5f * 500,  0.5f * 500, -0.5f * 500,  0.0f, 1.0f,
      0.5f * 500,  0.5f * 500, -0.5f * 500,  1.0f, 1.0f,
      0.5f * 500,  0.5f * 500,  0.5f * 500,  1.0f, 0.0f,
      0.5f * 500,  0.5f * 500,  0.5f * 500,  1.0f, 0.0f,
      -0.5f * 500,  0.5f * 500,  0.5f * 500,  0.0f, 0.0f,
      -0.5f * 500,  0.5f * 500, -0.5f * 500,  0.0f, 1.0f
  };
   */

  // use with Perspective Projection
  GLfloat vertexes[] = {
      -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.5f,  -0.5f, -0.5f, 1.0f, 0.0f,
      0.5f,  0.5f,  -0.5f, 1.0f, 1.0f, 0.5f,  0.5f,  -0.5f, 1.0f, 1.0f,
      -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f, -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,

      -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, 0.5f,  -0.5f, 0.5f,  1.0f, 0.0f,
      0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
      -0.5f, 0.5f,  0.5f,  0.0f, 1.0f, -0.5f, -0.5f, 0.5f,  0.0f, 0.0f,

      -0.5f, 0.5f,  0.5f,  1.0f, 0.0f, -0.5f, 0.5f,  -0.5f, 1.0f, 1.0f,
      -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
      -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, -0.5f, 0.5f,  0.5f,  1.0f, 0.0f,

      0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.5f,  0.5f,  -0.5f, 1.0f, 1.0f,
      0.5f,  -0.5f, -0.5f, 0.0f, 1.0f, 0.5f,  -0.5f, -0.5f, 0.0f, 1.0f,
      0.5f,  -0.5f, 0.5f,  0.0f, 0.0f, 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

      -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.5f,  -0.5f, -0.5f, 1.0f, 1.0f,
      0.5f,  -0.5f, 0.5f,  1.0f, 0.0f, 0.5f,  -0.5f, 0.5f,  1.0f, 0.0f,
      -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,

      -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f, 0.5f,  0.5f,  -0.5f, 1.0f, 1.0f,
      0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
      -0.5f, 0.5f,  0.5f,  0.0f, 0.0f, -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f};

  GLuint VBO, VAO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertexes), vertexes, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat),
                        (GLvoid*)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat),
                        (GLvoid*)(3 * sizeof(GLfloat)));
  glEnableVertexAttribArray(2);

  glBindVertexArray(0);

  GLuint texture;

  int width, height;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  unsigned char* image = SOIL_load_image("resources/images/image1.jpg", &width,
                                         &height, 0, SOIL_LOAD_RGBA);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, image);
  glGenerateMipmap(GL_TEXTURE_2D);
  SOIL_free_image_data(image);
  glBindTexture(GL_TEXTURE_2D, 0);

  glBindVertexArray(0);

  SDL_Event window_event;

  while (true) {
    if (SDL_PollEvent(&window_event)) {
      if (window_event.type == SDL_QUIT) break;
    }
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    shader_1.Use();

    glm::mat4 transform;
    transform = glm::translate(transform, glm::vec3(0.5f, 0.5f, 0.0f));
    unsigned int current_time;
    current_time = SDL_GetTicks();
    // if (current_time > last_time + 1000) {
    //  last_time = current_time;
    //}
    transform = glm::rotate(transform, (GLfloat)current_time * -5.0f,
                            glm::vec3(0.0f, 0.0f, 1.0f));

    GLint transform_location =
        glGetUniformLocation(shader_1.program, "transform");
    glUniformMatrix4fv(transform_location, 1, GL_FALSE,
                       glm::value_ptr(transform));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(glGetUniformLocation(shader_1.program, "texture_1"), 0);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    SDL_GL_SwapWindow(window);
  }

  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);

  SDL_GL_DeleteContext(context);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return EXIT_SUCCESS;
}
