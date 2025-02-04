#ifndef RENDERER_SHADERS_H
#define RENDERER_SHADERS_H

// TODO JH: Load from disk and out of renderer, but in app
const char* vertexShaderSource = R"(#version 330 core
                                    layout (location = 0) in vec2 aPos;
                                    uniform mat4 transform;
                                    void main()
                                    {
                                        gl_Position = transform * vec4(aPos, 0.0, 1.0);
                                    })";

const char* fragmentShaderSource = R"(#version 330 core
                                      out vec4 FragColor;
                                      uniform vec4 color;
                                      void main()
                                      {
                                          FragColor = color;
                                      })";


#endif
