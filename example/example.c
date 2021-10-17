#include <stdio.h>
#include <assert.h>
#include <gui.h>

#ifdef GUI_WITH_GLFW
#include <GLFW/glfw3.h>
#endif

static void render_frame(gui_t* g) {
  ImGuiIO* io = igGetIO();
  igBegin("window", NULL, ImGuiWindowFlags_NoTitleBar); {
    static float f = 0.0f;
    igText("Hello World!");
    igSliderFloat("float", &f, 0.0f, 1.0f, "%.3f", 0);
    if (g->isAnimating) {
      igText("Render: %.2f ms, Frame: %.1f ms (%.0f FPS)",
        g->avgRenderTime * 1000.0,
        1000.0f / io->Framerate,
        io->Framerate);
    } else {
      igText("Render: %.2f ms", g->avgRenderTime * 1000.0);
    }
  } igEnd();
  igShowDemoWindow(NULL);
}

void gui_start(gui_t* g) {
  printf("gui_start called. Using imgui v%s\n", igGetVersion());
  g->onRenderFrame = render_frame; // call render_frame on each frame

  gui_win_set_title(g->mainWindow, "Example");
  // gui_win_set_size(g->mainWindow, 800, 600);

  // can access impl if needed
  #ifdef GUI_WITH_GLFW
  GLFWwindow* win = (GLFWwindow*)g->mainWindow;
  glfwSetWindowSizeLimits(win, 200, 200, GLFW_DONT_CARE, GLFW_DONT_CARE);
  #endif
}
