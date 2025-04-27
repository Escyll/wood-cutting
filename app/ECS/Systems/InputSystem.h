#pragma once

#include <GLFW/glfw3.h>

enum class KeyState
{
    RELEASED,
    PRESSED,
    REPEATED,
    HOLD,
};

struct MouseState
{
    double x;
    double y;
    bool mouseDown;
};

bool isHolded(int key);
bool isPressed(int key);
bool isPressedOrRepeated(int key);
void markKeyStatesHold();
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
MouseState mouseState(GLFWwindow* window);
