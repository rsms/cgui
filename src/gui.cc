#include "gui.h"
#include "gui_internal.hh"
#include "font_inter_medium.h"
#include <rbase/rbase.h>

DIAGNOSTIC_IGNORE_PUSH(-Wimplicit-fallthrough)
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
DIAGNOSTIC_IGNORE_POP


u8* gui_img_load_file(char const* filename, int* w, int* h, int* comp, int req_comp) {
  u8* pixels = stbi_load(filename, w, h, comp, req_comp);
  if (!pixels)
    errlog("img_load_file \"%s\" failed: %s", filename, stbi_failure_reason());
  return pixels;
}


void gui_img_free_pixels(void* pixels) {
  stbi_image_free(pixels);
}


void _gui_init_main(int argc, const char** argv, gui_t* g) {
  // initialize gui_t struct
  g->clearColor[0] = 0.45f;
  g->clearColor[1] = 0.55f;
  g->clearColor[2] = 0.60f;
  g->clearColor[3] = 1.00f;
  g->isAnimating = true;
  g->isShowingUI = true;
  g->pxscale = 1.0;
}


static void _gui_load_fonts(gui_t* g) {
  // Load Fonts
  // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
  // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
  // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
  // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
  // - Read 'docs/FONTS.txt' for more instructions and details.
  // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !

  ImGuiIO& io = ImGui::GetIO();
  io.Fonts->Clear();
  // io.Fonts->AddFontDefault();

  // char filename[1024];
  // snprintf(filename, sizeof(filename), "%s/font/Inter-Medium.otf", gui_res_dir());
  // dlog("load font %s", filename);
  // ImFont* inter_medium = io.Fonts->AddFontFromFileTTF(filename, 14.0f * g->pxscale);
  // // if (!inter_medium->IsLoaded()) {
  // //   fprintf(stderr, "failed to load font %s\n", filename);
  // //   abort();
  // // }
  // // dlog("inter_medium->IsLoaded() => %d", inter_medium->IsLoaded());

  ImFontConfig* font_cfg = NULL;
  ImWchar* glyph_ranges = NULL;
  ImFont* inter_medium = io.Fonts->AddFontFromMemoryCompressedTTF(
    inter_medium_compressed_data,
    inter_medium_compressed_size,
    14.0f * g->pxscale, font_cfg, glyph_ranges);

  io.Fonts->Build();
  assert(inter_medium->IsLoaded());
  g->_font_pxscale = g->pxscale;
}


void _gui_onFramebufferDidChange(gui_t* g) {
  if (g->_font_pxscale != g->pxscale)
    _gui_load_fonts(g);
  if (g->onFramebufferDidChange)
    g->onFramebufferDidChange(g);
}


void _gui_before_start(gui_t* g) {
  ImGuiIO& io = ImGui::GetIO();

  // io.MouseDrawCursor = true;
  //   // Draw custom pointer cursors with imgui

  io.ConfigMacOSXBehaviors = true;
    // OS X style: Text editing cursor movement using Alt instead of Ctrl,
    // Shortcuts using Cmd/Super instead of Ctrl, Line/Text Start and End using Cmd+Arrows
    // instead of Home/End, Double click selects by word instead of selecting whole text,
    // Multi-selection in lists uses Cmd/Super instead of Ctrl.

  io.ConfigDragClickToInputText = true;
    // [BETA] Enable turning DragXXX widgets into text input with a simple mouse click-release
    // (without moving). Not desirable on devices without a keyboard.

  io.ConfigWindowsMoveFromTitleBarOnly = true;
    // Enable allowing to move windows only when clicking on their title bar.
    // Does not apply to windows without a title bar.

  // ImGui::StyleColorsDark();
  ImGui::StyleColorsLight();

  _gui_load_fonts(g);

  // Customize style
  ImGuiStyle& style = ImGui::GetStyle();
  // style.ScaleAllSizes(io.DisplayFramebufferScale.x);
  style.WindowTitleAlign = {0.5, 0.5};
}
