#pragma once

#include <GLFW/glfw3.h>

enum class KeyState
{
    RELEASED,
    PRESSED,
    REPEATED,
    HOLD,
};

bool isHolded(int key);
bool isPressed(int key);
bool isPressedOrRepeated(int key);
void markKeyStatesHold();
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
