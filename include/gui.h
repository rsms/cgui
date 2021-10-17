#pragma once
#ifdef __cplusplus
  #include <imgui.h>
  #define EXTERN extern "C"
#else
  #include <stdbool.h>
  #include <stdint.h>
  #define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
  #include <cimgui.h>
  #define EXTERN extern
#endif

typedef struct gui_win_st gui_win_t;
typedef struct gui_st gui_t;

struct gui_st {
  // callbacks
  void (*onRenderFrame)(gui_t*);
  void (*onFramebufferDidChange)(gui_t*);

  // read-only
  double       avgRenderTime; // average number of seconds spent rendering
  gui_win_t*   mainWindow;
  int          cliArgc; // arguments passed to main function
  const char** cliArgv;

  // state
  void* userdata;          // whatever you want
  float clearColor[4];     // background color (RGBA)
  bool  isAnimating;       // false to only call onRenderFrame on events
  bool  isShowingUI;       // ImGUI shown or not
  float pxscale;           // pixel scale (1 dp = pxscale px)
  int   fbWidth, fbHeight; // framebuffer size in pixels

  // internal state
  float _font_pxscale;
};

EXTERN void gui_start(gui_t* g); // entry; implemented by app
EXTERN double gui_monotime();   // seconds

EXTERN void gui_win_set_title(gui_win_t* win, const char* title);
EXTERN void gui_win_set_pos(gui_win_t* win, uint32_t x_dp, uint32_t y_dp);
EXTERN void gui_win_set_size(gui_win_t* win, uint32_t width_dp, uint32_t height_dp);

// comp - actual components (i.e. 4=RGBA, 3=RGB, 1=Greyscale)
// req_comp - expected components
EXTERN uint8_t* gui_img_load_file(
  char const* filename, int* w, int* h, int* comp, int req_comp);
EXTERN void gui_img_free_pixels(void* pixels);
