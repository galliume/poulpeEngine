module;

//@todo check X11 too
#ifdef __unix__
#define GLFW_EXPOSE_NATIVE_WAYLAND
#else
#define GLFW_EXPOSE_NATIVE_WIN32
#endif

#define GLFW_INCLUDE_VULKAN

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

export module Engine.Core.GLFW;

#undef GLFW_FALSE
#undef GLFW_TRUE
#undef GLFW_DECORATED
#undef GLFW_VISIBLE
#undef GLFW_RESIZABLE
#undef GLFW_CONTEXT_VERSION_MAJOR
#undef GLFW_CONTEXT_VERSION_MINOR
#undef GLFW_OPENGL_CORE_PROFILE
#undef GLFW_CLIENT_API
#undef GLFW_NO_API
#undef GLFW_KEY_A
#undef GLFW_KEY_B
#undef GLFW_KEY_C
#undef GLFW_KEY_D
#undef GLFW_KEY_E
#undef GLFW_KEY_F
#undef GLFW_KEY_G
#undef GLFW_KEY_H
#undef GLFW_KEY_I
#undef GLFW_KEY_J
#undef GLFW_KEY_K
#undef GLFW_KEY_L
#undef GLFW_KEY_M
#undef GLFW_KEY_N
#undef GLFW_KEY_O
#undef GLFW_KEY_P
#undef GLFW_KEY_Q
#undef GLFW_KEY_R
#undef GLFW_KEY_S
#undef GLFW_KEY_T
#undef GLFW_KEY_U
#undef GLFW_KEY_V
#undef GLFW_KEY_W
#undef GLFW_KEY_X
#undef GLFW_KEY_Y
#undef GLFW_KEY_Z
#undef GLFW_KEY_0
#undef GLFW_KEY_1
#undef GLFW_KEY_2
#undef GLFW_KEY_3
#undef GLFW_KEY_4
#undef GLFW_KEY_5
#undef GLFW_KEY_6
#undef GLFW_KEY_7
#undef GLFW_KEY_8
#undef GLFW_KEY_9
#undef GLFW_KEY_LEFT_CONTROL
#undef GLFW_PRESS
#undef GLFW_RELEASE
#undef GLFW_REPEAT
#undef GLFW_CURSOR
#undef GLFW_CURSOR_DISABLED
#undef GLFW_CURSOR_NORMAL
#undef GLFW_MOUSE_BUTTON_LEFT
#undef GLFW_JOYSTICK_1
#undef GLFW_GAMEPAD_BUTTON_A
#undef GLFW_GAMEPAD_AXIS_LEFT_X
#undef GLFW_GAMEPAD_AXIS_LEFT_Y
#undef GLFW_GAMEPAD_AXIS_RIGHT_X
#undef GLFW_GAMEPAD_AXIS_RIGHT_Y
#undef GLFW_SRGB_CAPABLE

#undef HWND_TOP
#undef SWP_SHOWWINDOW

export
{
  inline constexpr int GLFW_FALSE = 0;
  inline constexpr int GLFW_TRUE = 1;
  inline constexpr int GLFW_DECORATED = 0x00020005;
  inline constexpr int GLFW_VISIBLE = 0x00020004;
  inline constexpr int GLFW_RESIZABLE = 0x00020003;
  inline constexpr int GLFW_CONTEXT_VERSION_MAJOR = 0x00022002;
  inline constexpr int GLFW_CONTEXT_VERSION_MINOR = 0x00022003;
  inline constexpr int GLFW_OPENGL_CORE_PROFILE = 0x00032001;
  inline constexpr int GLFW_CLIENT_API = 0x00022001;
  inline constexpr int GLFW_NO_API = 0;

  inline constexpr int GLFW_KEY_0 = 48;
  inline constexpr int GLFW_KEY_1 = 49;
  inline constexpr int GLFW_KEY_2 = 50;
  inline constexpr int GLFW_KEY_3 = 51;
  inline constexpr int GLFW_KEY_4 = 52;
  inline constexpr int GLFW_KEY_5 = 53;
  inline constexpr int GLFW_KEY_6 = 54;
  inline constexpr int GLFW_KEY_7 = 55;
  inline constexpr int GLFW_KEY_8 = 56;
  inline constexpr int GLFW_KEY_9 = 57;
  inline constexpr int GLFW_KEY_A = 65;
  inline constexpr int GLFW_KEY_B = 66;
  inline constexpr int GLFW_KEY_C = 67;
  inline constexpr int GLFW_KEY_D = 68;
  inline constexpr int GLFW_KEY_E = 69;
  inline constexpr int GLFW_KEY_F = 70;
  inline constexpr int GLFW_KEY_G = 71;
  inline constexpr int GLFW_KEY_H = 72;
  inline constexpr int GLFW_KEY_I = 73;
  inline constexpr int GLFW_KEY_J = 74;
  inline constexpr int GLFW_KEY_K = 75;
  inline constexpr int GLFW_KEY_L = 76;
  inline constexpr int GLFW_KEY_M = 77;
  inline constexpr int GLFW_KEY_N = 78;
  inline constexpr int GLFW_KEY_O = 79;
  inline constexpr int GLFW_KEY_P = 80;
  inline constexpr int GLFW_KEY_Q = 81;
  inline constexpr int GLFW_KEY_R = 82;
  inline constexpr int GLFW_KEY_S = 83;
  inline constexpr int GLFW_KEY_T = 84;
  inline constexpr int GLFW_KEY_U = 85;
  inline constexpr int GLFW_KEY_V = 86;
  inline constexpr int GLFW_KEY_W = 87;
  inline constexpr int GLFW_KEY_X = 88;
  inline constexpr int GLFW_KEY_Y = 89;
  inline constexpr int GLFW_KEY_Z = 90;
  inline constexpr int GLFW_KEY_LEFT_CONTROL = 341;

  inline constexpr int GLFW_PRESS = 1;
  inline constexpr int GLFW_RELEASE = 0;
  inline constexpr int GLFW_REPEAT = 2;

  inline constexpr int GLFW_CURSOR = 0x00033001;
  inline constexpr int GLFW_CURSOR_DISABLED = 0x00034003;
  inline constexpr int GLFW_CURSOR_NORMAL = 0x00034001;

  inline constexpr int GLFW_MOUSE_BUTTON_LEFT = 0;
  inline constexpr int GLFW_JOYSTICK_1 = 0;

  inline constexpr int GLFW_GAMEPAD_BUTTON_A = 0;
  inline constexpr int GLFW_GAMEPAD_AXIS_LEFT_X = 0;
  inline constexpr int GLFW_GAMEPAD_AXIS_LEFT_Y = 1;
  inline constexpr int GLFW_GAMEPAD_AXIS_RIGHT_X = 2;
  inline constexpr int GLFW_GAMEPAD_AXIS_RIGHT_Y = 3;
  inline constexpr int GLFW_SRGB_CAPABLE = 0x0002100E;

  using ::GLFWwindow;
  using ::GLFWmonitor;
  using ::GLFWimage;
  using ::GLFWgamepadstate;

  using ::glfwInit;
  using ::glfwTerminate;
  using ::glfwCreateWindow;
  using ::glfwDestroyWindow;
  using ::glfwWindowShouldClose;
  using ::glfwPollEvents;
  using ::glfwSwapBuffers;
  using ::glfwShowWindow;
  using ::glfwHideWindow;
  using ::glfwSetFramebufferSizeCallback;
  using ::glfwSetWindowUserPointer;
  using ::glfwGetWindowUserPointer;
  using ::glfwSetWindowIcon;
  using ::glfwGetPrimaryMonitor;
  using ::glfwWaitEvents;
  using ::glfwSetWindowShouldClose;
  using ::glfwGetTime;
  using ::glfwWindowHint;
  using ::glfwGetVideoMode;
  using ::glfwSetWindowPos;
  using ::glfwGetWindowSize;
  using ::glfwGetFramebufferSize;
  using ::glfwSetWindowTitle;
  using ::glfwMakeContextCurrent;
  using ::glfwSwapInterval;
  using ::glfwSetInputMode;
  using ::glfwGetCursorPos;
  using ::glfwJoystickIsGamepad;
  using ::glfwGetGamepadName;
  using ::glfwGetGamepadState;
  using ::glfwSetKeyCallback;
  using ::glfwSetCursorPosCallback;
  using ::glfwSetMouseButtonCallback;
  using ::glfwGetWindowPos;
  using ::glfwSetWindowPosCallback;

  using ::glfwVulkanSupported;
  using ::glfwGetRequiredInstanceExtensions;
  using ::glfwCreateWindow;
  using ::glfwCreateWindowSurface;

#ifdef _WIN32
  using ::glfwGetWin32Window;
  using ::HWND;
  using ::RECT;
  using ::GetClientRect;
  using ::GetWindowRect;
  using ::SetParent;
  using ::SetWindowPos;
  
  inline constexpr HWND HWND_TOP = static_cast<HWND>(nullptr);
  inline constexpr int SWP_SHOWWINDOW = 0x0040;
#endif
}
