#include "app.hh"
#include "image.h"

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <rbase/rbase.h>
#include <stdio.h>
#include <signal.h> // kill
#include <cmath>


#if defined(IMGUI_IMPL_OPENGL_ES2)
  #include <GLES2/gl2.h>
  // About Desktop OpenGL function loaders:
  //  Modern desktop OpenGL doesn't have a standard portable header file to load OpenGL function pointers.
  //  Helper libraries are often used for this purpose! Here we are supporting a few common ones (gl3w, glew, glad).
  //  You may use another loader/header of your choice (glext, glLoadGen, etc.), or chose to manually implement your own.
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
  #include <GL/gl3w.h>            // Initialize with gl3wInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
  #include <GL/glew.h>            // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
  #include <glad/glad.h>          // Initialize with gladLoadGL()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD2)
  #include <glad/gl.h>            // Initialize with gladLoadGL(...) or gladLoaderLoadGL()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
  #define GLFW_INCLUDE_NONE       // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
  #include <glbinding/Binding.h>  // Initialize with glbinding::Binding::initialize()
  #include <glbinding/gl/gl.h>
  using namespace gl;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
  #define GLFW_INCLUDE_NONE       // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
  #include <glbinding/glbinding.h>// Initialize with glbinding::initialize()
  #include <glbinding/gl/gl.h>
  using namespace gl;
#else
  #include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif


#include <GLFW/glfw3.h>


// // copied from imgui/backends/imgui_impl_glfw.cpp
// enum GlfwClientApi {
//   GlfwClientApi_Unknown,
//   GlfwClientApi_OpenGL,
//   GlfwClientApi_Vulkan
// };
// struct ImGui_ImplGlfw_Data {
//   GLFWwindow*             Window;
//   GlfwClientApi           ClientApi;
//   double                  Time;
//   bool                    MouseJustPressed[ImGuiMouseButton_COUNT];
//   GLFWcursor*             MouseCursors[ImGuiMouseCursor_COUNT];
//   bool                    InstalledCallbacks;
// };
// static ImGui_ImplGlfw_Data* ImGui_ImplGlfw_GetBackendData() {
//   return (
//     ImGui::GetCurrentContext() ? (ImGui_ImplGlfw_Data*)ImGui::GetIO().BackendPlatformUserData :
//     NULL
//   );
// }


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
  app.frameRenderTime.start();

  // Start the Dear ImGui frame
  if (app.isShowingUI) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
  }

  glClearColor(
    app.clearColor[0] * app.clearColor[3],
    app.clearColor[1] * app.clearColor[3],
    app.clearColor[2] * app.clearColor[3],
    app.clearColor[3]);
  glClear(GL_COLOR_BUFFER_BIT);

  app.renderFrame();

  // render
  if (app.isShowingUI) {
    ImGui::Render();
    // You may need to backup/reset/restore other state,
    // for current shader:
    GLint last_program;
    glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
    glUseProgram(0);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glUseProgram(last_program);
  }

  glfwMakeContextCurrent(window);
  app.frameRenderTime.end();
  glfwSwapBuffers(window); // blocks on vsync
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
  glViewport(0, 0, width, height);
  updateFramebufferScale(window);
  app.framebufferDidChange();
  new_frame(window, width, height);
}


// glfwScrollCallback is called on scroll input (e.g. moving a mouse's scroll wheel)
static void glfwScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
  // This is a customized version of ImGui_ImplGlfw_ScrollCallback.
  ImGuiIO& io = ImGui::GetIO();
  // From the imgui documentation:
  //   "MouseWheel: Mouse wheel Vertical: 1 unit scrolls about 5 lines text."
  // So we scale it back to DPs: (/5 = 5 lines -> 1 line, /10 = 10dp -> 1dp)
  io.MouseWheelH += (float)xoffset / 15;
  io.MouseWheel += (float)yoffset / 15;
}


static GLFWkeyfun imgui_glfwKeyCallback;

static void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  fprintf(stderr, "glfwKeyCallback key=%d scancode=%d action=%s mods=%x\n",
    key,
    scancode,
    ( action == GLFW_PRESS   ? "PRESS" :
      action == GLFW_RELEASE ? "RELEASE" :
      action == GLFW_REPEAT  ? "REPEAT" :
      "?" ),
    (uint32_t)mods);
  if (app.isShowingUI) {
    imgui_glfwKeyCallback(window, key, scancode, action, mods);
    if (ImGui::GetIO().WantCaptureKeyboard)
      return;
  }
  if (action == GLFW_PRESS) {
    switch (key) {
      case GLFW_KEY_U:
        // toggle UI
        app.isShowingUI = !app.isShowingUI;
        // glfwSetInputMode(window, GLFW_CURSOR,
        //   app.isShowingUI ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);
        break;
      default:
        break;
    }
  }
}


static bool initOpenGL3() {
  // Initialize OpenGL loader
  #if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
    bool err = gl3wInit() != 0;
  #elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
    bool err = glewInit() != GLEW_OK;
  #elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
    bool err = gladLoadGL() == 0;
  #elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD2)
    bool err = gladLoadGL(glfwGetProcAddress) == 0; // glad2 recommend using the windowing library loader instead of the (optionally) bundled one.
  #elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
    bool err = false;
    glbinding::Binding::initialize();
  #elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
    bool err = false;
    glbinding::initialize([](const char* name) {
      return (glbinding::ProcAddress)glfwGetProcAddress(name);
    });
  #else
    bool err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
  #endif
  if (err)
    fprintf(stderr, "Failed to initialize OpenGL loader!\n");
  return !err;
}


static const char* pidfile = ".snark.pid";


static void remove_pidfile() {
  unlink(pidfile);
}


static void write_pidfile() {
  int fd = open(pidfile, O_RDONLY);
  char buf[32];
  if (fd > -1) {
    long n = read(fd, buf, sizeof(buf));
    if (n < (long)sizeof(buf)) {
      buf[n] = 0;
      int pid = atoi(buf);
      if (kill(pid, 9) > -1)
        fprintf(stderr, "killed old process [%d] with sigkill\n", pid);
    }
    close(fd);
    unlink(pidfile);
  }
  fd = open(pidfile, O_WRONLY | O_CREAT | O_EXCL, 0666);
  int n = snprintf(buf, sizeof(buf), "%d\n", getpid());
  write(fd, buf, (unsigned long)n);
  close(fd);
  atexit(remove_pidfile);
}


int main(int, char**) {
  glfwSetErrorCallback(glfw_error_callback);
  if (!glfwInit())
    return 1;

  write_pidfile();
  unlink("imgui.ini");

  // Decide GL+GLSL versions
  #if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
  #elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
  #else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
  #endif

  #ifdef __APPLE__
  glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_TRUE);
  #endif

  GLFWwindow* window = glfwCreateWindow(730, 600, "snark", NULL, NULL);
  if (window == NULL)
    return 1;
  glfwMakeContextCurrent(window);

  bool vsync = false;
  glfwSwapInterval(vsync ? 1 : 0); // Enable vsync

  // [rsms] move window to bottom right corner of screen and reactive editor
  glfwSetWindowPos(window, 1830, 835); // 2nd screen, bottom left corner
  system("osascript -e 'activate application \"Sublime Text\"'");


  if (!initOpenGL3())
    return 1;


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
  ImGui_ImplOpenGL3_Init(glsl_version);

  // // set custom glfw cursors (imgui sets default ones in ImGui_ImplGlfw_Init so we do it here)
  // ImGui_ImplGlfw_Data* bd = ImGui_ImplGlfw_GetBackendData();
  // GLFWimage* img1 = (GLFWimage*)malloc(sizeof(GLFWimage));
  // char filename[1024];
  // snprintf(filename, sizeof(filename), "%s/cursor_arrow.png", app.resFileDir());
  // img1->pixels = img_load_file(filename, &img1->width, &img1->height, 0, /*RGBA*/4);
  // if (img1->pixels) {
  //   dlog("setting MouseCursors[ImGuiMouseCursor_Arrow]");
  //   int xhot = 0, yhot = 0;
  //   bd->MouseCursors[ImGuiMouseCursor_Arrow] = glfwCreateCursor(img1, xhot, yhot);
  //   // TODO: img_free_pixels(bd->MouseCursors[ImGuiMouseCursor_Arrow]->pixels);
  //   // TODO: glfwDestroyCursor(bd->MouseCursors[ImGuiMouseCursor_Arrow]);
  // }

  // replace imgui scroll callback with our own (must call after ImGui_ImplGlfw_Init*)
  glfwSetScrollCallback(window, glfwScrollCallback);
  imgui_glfwKeyCallback = glfwSetKeyCallback(window, glfwKeyCallback);

  // update display scale now as we need it for initial config
  updateFramebufferScale(window);
  glfwSetFramebufferSizeCallback(window, onWindowFramebufferResize);
  glfwGetFramebufferSize(window, &app.fbWidth, &app.fbHeight);

  // initial configuration
  glViewport(0, 0, app.fbWidth, app.fbHeight);
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

    if (is_animating || io.WantCaptureKeyboard /* blinking cursor */) {
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

    // fprintf(stderr, "io.WantCaptureKeyboard %s\n", io.WantCaptureKeyboard ? "true" : "false");
    // fprintf(stderr, "io.WantCaptureMouse    %s\n", io.WantCaptureMouse ? "true" : "false");

    // trick to make sure that we draw an extra frame after isAnimate transitions
    // from true -> false.
    if (!app.isAnimating)
      is_animating = false;

    new_frame(window, app.fbWidth, app.fbHeight);

    if (app.isAnimating)
      is_animating = true;
  }

  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
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
