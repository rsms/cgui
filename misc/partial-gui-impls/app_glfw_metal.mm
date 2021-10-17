// Dear ImGui: standalone example application for GLFW + Metal, using programmable pipeline
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#include "app.hh"

#include "imgui_impl_glfw.h"
#include "imgui_impl_metal.h"
#include <stdio.h>
#include <cmath>

#define GLFW_INCLUDE_NONE
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>

#define DLOG_PREFIX "\e[1;34m[server]\e[0m "

#ifdef DEBUG
  #define dlog(format, ...) ({ \
    fprintf(stderr, DLOG_PREFIX format " \e[2m(%s %d)\e[0m\n", \
      ##__VA_ARGS__, __FUNCTION__, __LINE__); \
    fflush(stderr); \
  })
  #define errlog(format, ...) \
    (({ fprintf(stderr, "E " format " (%s:%d)\n", ##__VA_ARGS__, __FILE__, __LINE__); \
        fflush(stderr); }))
#else
  #define dlog(...) do{}while(0)
  #define errlog(format, ...) \
(({ fprintf(stderr, "E " format "\n", ##__VA_ARGS__); fflush(stderr); }))
#endif

double monotime() {
  return glfwGetTime();
}

static void glfw_error_callback(int error, const char* description) {
  fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

struct MtlBackendState {
  id<MTLDevice>            device;
  id<MTLCommandQueue>      commandQueue;
  CAMetalLayer*            layer;
  MTLRenderPassDescriptor* renderPassDescriptor;
};

static MtlBackendState mtlBackend;
static App app;
static int fb_width, fb_height;


const char* App::resFileDir() {
  return "./res";
}


static void new_frame(int width, int height) {
  @autoreleasepool {
    // [rsms] this appears to block on vsync
    mtlBackend.layer.drawableSize = CGSizeMake(width, height);
    id<CAMetalDrawable> drawable = [mtlBackend.layer nextDrawable];

    id<MTLCommandBuffer> commandBuffer = [mtlBackend.commandQueue commandBuffer];
    mtlBackend.renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(
      app.clearColor[0] * app.clearColor[3],
      app.clearColor[1] * app.clearColor[3],
      app.clearColor[2] * app.clearColor[3],
      app.clearColor[3]);
    mtlBackend.renderPassDescriptor.colorAttachments[0].texture = drawable.texture;
    mtlBackend.renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
    mtlBackend.renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
    id <MTLRenderCommandEncoder> renderEncoder =
      [commandBuffer renderCommandEncoderWithDescriptor:mtlBackend.renderPassDescriptor];
    [renderEncoder pushDebugGroup:@"imgui"];

    // App's main update
    app.frameComposeTime.start();
    // Start the Dear ImGui frame
    ImGui_ImplMetal_NewFrame(mtlBackend.renderPassDescriptor);
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    app.renderFrame();
    app.frameComposeTime.end();

    // Render
    app.frameRenderTime.start();
    ImGui::Render();
    ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(), commandBuffer, renderEncoder);
    [renderEncoder popDebugGroup];
    [renderEncoder endEncoding];
    [commandBuffer presentDrawable:drawable];
    [commandBuffer commit];
    app.frameRenderTime.end();
  }
}


// onWindowFramebufferResize is called when a window's framebuffer has changed size.
// width & height are in pixels (the framebuffer size)
static void onWindowFramebufferResize(GLFWwindow* window, int width, int height) {
  fb_width = width;
  fb_height = height;
  app.framebufferDidChange();
  new_frame(width, height);
}


int main(int, char**) {
  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
  //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

  // Setup style
  // ImGui::StyleColorsDark();
  // ImGui::StyleColorsLight();
  //ImGui::StyleColorsClassic();


  // Setup window
  glfwSetErrorCallback(glfw_error_callback);
  if (!glfwInit())
    return 1;

  // Create window with graphics context
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_TRUE);
  GLFWwindow* window = glfwCreateWindow(780, 600, "Meow", NULL, NULL);
  if (window == NULL)
    return 1;

  // [rsms] move window to bottom right corner of screen and reactive editor
  glfwSetWindowPos(window, 1780, 835); // 2nd screen, bottom left corner
  system("osascript -e 'activate application \"Sublime Text\"'");

  // update display scale now as we need it for initial config
  glfwGetWindowContentScale(window, &io.DisplayFramebufferScale.x, &io.DisplayFramebufferScale.y);
  glfwSetFramebufferSizeCallback(window, onWindowFramebufferResize);

  glfwGetFramebufferSize(window, &fb_width, &fb_height);

  // initial configuration
  app.init();

  // Setup Metal rendering backend
  mtlBackend.device = MTLCreateSystemDefaultDevice();
  mtlBackend.commandQueue = [mtlBackend.device newCommandQueue];

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplMetal_Init(mtlBackend.device);

  NSWindow* nswin = glfwGetCocoaWindow(window);
  mtlBackend.layer = [CAMetalLayer layer];
  mtlBackend.layer.device = mtlBackend.device;
  mtlBackend.layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
  nswin.contentView.layer = mtlBackend.layer;
  nswin.contentView.wantsLayer = YES;

  mtlBackend.renderPassDescriptor = [MTLRenderPassDescriptor new];

  //glfwSwapInterval(0);

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

    glfwPollEvents();
    // glfwWaitEventsTimeout(0.007);
    // glfwWaitEventsTimeout(1.0 / 60.0);

    new_frame(fb_width, fb_height);
  }

  // Cleanup
  ImGui_ImplMetal_Shutdown();
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
