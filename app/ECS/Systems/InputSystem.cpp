#include "ECS/Systems/InputSystem.h"

#include <unordered_map>

std::unordered_map<int, KeyState> keyStates;

bool isHolded(int key)
{
    return keyStates[key] != KeyState::RELEASED;
}

bool isPressed(int key)
{
    return keyStates[key] == KeyState::PRESSED;
}

bool isPressedOrRepeated(int key)
{
    return keyStates[key] == KeyState::PRESSED || keyStates[key] == KeyState::REPEATED;
}

void markKeyStatesHold()
{
    for (auto& [key, value]: keyStates)
    {
        if (value != KeyState::RELEASED)
            value = KeyState::HOLD;
    }
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    switch (action)
    {
        case GLFW_PRESS:
            keyStates[key] = KeyState::PRESSED;
            break;
        case GLFW_REPEAT:
            keyStates[key] = KeyState::REPEATED;
            break;
        case GLFW_RELEASE:
            keyStates[key] = KeyState::RELEASED;
            break;
    }
}
