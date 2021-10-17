super duper simle gui for C, wrapping imgui and stb

You can use it as a static library with cmake.
See the `example` directory for a complete example.

Example:

```c
#include <stdio.h>
#include <assert.h>
#include <gui.h>

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
  g->onRenderFrame = render_frame;
  gui_win_set_title(g->mainWindow, "Example");
}
```

Try it: `cd example && ../ext/ckit/bin/ckit watch -run example`
