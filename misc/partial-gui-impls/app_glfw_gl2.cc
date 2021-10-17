#include "app.hh"

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl2.h"
#include <stdio.h>
#include <cmath>

// #define GLFW_INCLUDE_NONE
// #define GLFW_EXPOSE_NATIVE_COCOA
#ifdef __APPLE__
  #define GL_SILENCE_DEPRECATION
#endif
#include <GLFW/glfw3.h>
// #include <GLFW/glfw3native.h>

double monotime() {
  return glfwGetTime();
}

static void glfw_error_callback(int error, const char* description) {
  fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

static App app;

const char* App::resFileDir() {
  return "./res";
}

static void new_frame(GLFWwindow* window, int width, int height) {
  // compose / update
  app.frameComposeTime.start();

  // Start the Dear ImGui frame
  ImGui_ImplOpenGL2_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  app.renderFrame();

  app.frameComposeTime.end();

  // render
  app.frameRenderTime.start();
  ImGui::Render();
  glViewport(0, 0, width, height);
  glClearColor(
    app.clearColor[0] * app.clearColor[3],
    app.clearColor[1] * app.clearColor[3],
    app.clearColor[2] * app.clearColor[3],
    app.clearColor[3]);
  glClear(GL_COLOR_BUFFER_BIT);

  // If you are using this code with non-legacy OpenGL header/contexts (which you should not, prefer using imgui_impl_opengl3.cpp!!),
  // you may need to backup/reset/restore other state, e.g. for current shader using the commented lines below.
  //GLint last_program;
  //glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
  //glUseProgram(0);
  ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
  //glUseProgram(last_program);

  glfwMakeContextCurrent(window);

  app.frameRenderTime.end();

  // blocks on vsync
  glfwSwapBuffers(window);
}


static void updateFramebufferScale(GLFWwindow* window) {
  ImGuiIO& io = ImGui::GetIO();
  glfwGetWindowContentScale(window, &io.DisplayFramebufferScale.x, &io.DisplayFramebufferScale.y);
  app.pxscale = io.DisplayFramebufferScale.x;
  // io.DisplayFramebufferScale.x *= 2.0;
  // io.DisplayFramebufferScale.y *= 2.0;
  io.FontGlobalScale = 1.0 / app.pxscale;
}


// onWindowFramebufferResize is called when a window's framebuffer has changed size.
// width & height are in pixels (the framebuffer size)
static void onWindowFramebufferResize(GLFWwindow* window, int width, int height) {
  app.fbWidth = width;
  app.fbHeight = height;
  updateFramebufferScale(window);
  app.framebufferDidChange();
  new_frame(window, width, height);
}


// glfwScrollCallback is called on scroll input (e.g. moving a mouse's scroll wheel)
void glfwScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
  // This is a customized version of ImGui_ImplGlfw_ScrollCallback.
  ImGuiIO& io = ImGui::GetIO();
  // From the imgui documentation:
  //   "MouseWheel: Mouse wheel Vertical: 1 unit scrolls about 5 lines text."
  // So we scale it back to DPs: (/5 = 5 lines -> 1 line, /10 = 10dp -> 1dp)
  io.MouseWheelH += (float)xoffset / 15;
  io.MouseWheel += (float)yoffset / 15;
}


int main(int, char**) {
  glfwSetErrorCallback(glfw_error_callback);
  if (!glfwInit())
    return 1;

  #ifdef __APPLE__
  glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_TRUE);
  // glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  // glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  #endif

  GLFWwindow* window = glfwCreateWindow(780, 600, "Meow", NULL, NULL);
  if (window == NULL)
    return 1;
  glfwMakeContextCurrent(window);

  bool vsync = false;
  glfwSwapInterval(vsync ? 1 : 0); // Enable vsync

  // [rsms] move window to bottom right corner of screen and reactive editor
  glfwSetWindowPos(window, 1780, 835); // 2nd screen, bottom left corner
  system("osascript -e 'activate application \"Sublime Text\"'");


  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
  //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

  // Setup style
  ImGui::StyleColorsLight();

  // Setup window
  glfwSetErrorCallback(glfw_error_callback);
  if (!glfwInit())
    return 1;

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL2_Init();

  // TODO: set custom glfw cursors (imgui sets default ones in ImGui_ImplGlfw_Init)
  // GLFWcursor* glfwCreateCursor(const GLFWimage* image, int xhot, int yhot);

  // replace imgui scroll callback with our own (must call after ImGui_ImplGlfw_Init*)
  glfwSetScrollCallback(window, glfwScrollCallback);

  // update display scale now as we need it for initial config
  updateFramebufferScale(window);
  glfwSetFramebufferSizeCallback(window, onWindowFramebufferResize);
  glfwGetFramebufferSize(window, &app.fbWidth, &app.fbHeight);

  // initial configuration
  app.init();

  bool is_animating = true;

  // Main loop
  while (!glfwWindowShouldClose(window)) {
    // Poll and handle events (inputs, window resize, etc.)
    // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell
    // if dear imgui wants to use your inputs.
    // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your
    //   main application.
    // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your
    //   main application.
    // Generally you may always pass all inputs to dear imgui,
    // and hide them from your application based on those two flags.

    if (is_animating) {
      if (vsync) {
        glfwPollEvents();
      } else {
        glfwWaitEventsTimeout(1.0 / (60.0 * 2.0));
      }
    } else {
      glfwWaitEvents();
    }

    // double xpos = 0, ypos = 0;
    // glfwGetCursorPos(window, &xpos, &ypos);
    // fprintf(stderr, "%f, %f\n", xpos, ypos);

    // update mouse position
    // NSPoint mlocOnScreen = [NSEvent mouseLocation];
    // NSRect mlockInWindow = [nswin convertRectFromScreen:(NSRect){.origin=mlocOnScreen}];
    // NSPoint mloc = [nswin.contentView convertPoint:mlockInWindow.origin fromView:nil];
    // double mouse_x, mouse_y;
    // glfwGetCursorPos(window, &mouse_x, &mouse_y);
    // io.MousePos = ImVec2((float)mouse_x, (float)mouse_y);
    // if (mouse_x != mloc.x)
    //   fprintf(stderr, "mpos %f, %f  (%f, %f)\n", mouse_x, mouse_y, mloc.x, mloc.y);


    // trick to make sure that we draw an extra frame after isAnimate transitions
    // from true -> false.
    if (!app.isAnimating)
      is_animating = false;

    new_frame(window, app.fbWidth, app.fbHeight);

    if (app.isAnimating)
      is_animating = true;
  }

  // Cleanup
  ImGui_ImplOpenGL2_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}

// // update mouse position
// NSPoint mlocOnScreen = [NSEvent mouseLocation];
// NSRect mlockInWindow = [nswin convertRectFromScreen:(NSRect){.origin=mlocOnScreen}];
// NSPoint mloc = [nswin.contentView convertPoint:mlockInWindow.origin fromView:nil];
// double mouse_x, mouse_y;
// glfwGetCursorPos(window, &mouse_x, &mouse_y);
// io.MousePos = ImVec2((float)mouse_x, (float)mouse_y);
// if (mouse_x != mloc.x)
//   fprintf(stderr, "mpos %f, %f  (%f, %f)\n", mouse_x, mouse_y, mloc.x, mloc.y);
