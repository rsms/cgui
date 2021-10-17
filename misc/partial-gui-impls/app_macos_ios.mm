// Dear ImGui: standalone example application for OSX + Metal.
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#import <Foundation/Foundation.h>

#if TARGET_OS_OSX
#import <Cocoa/Cocoa.h>
#else
#import <UIKit/UIKit.h>
#endif

#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

#include "app.hh"
#include "imgui.h"
#include "imgui_impl_metal.h"

#if TARGET_OS_OSX
#include "imgui_impl_osx.h"

static App app;

// const char* App::resFileDir() {
//   static const char* path = NULL;
//   static dispatch_once_t onceToken;
//   dispatch_once(&onceToken, ^{
//     const char* s = [[NSBundle mainBundle].resourcePath cStringUsingEncoding:NSUTF8StringEncoding];
//     path = strdup(s);
//   });
//   return path;
// }
const char* App::resFileDir() {
  return "./res";
}

@interface ViewController : NSViewController {
  BOOL _appInitialized;
}
@end
#else
@interface ViewController : UIViewController
@end
#endif

@interface ViewController () <MTKViewDelegate>
@property (nonatomic, readonly) MTKView *mtkView;
@property (nonatomic, strong) id <MTLDevice> device;
@property (nonatomic, strong) id <MTLCommandQueue> commandQueue;
@end

@implementation ViewController

- (instancetype)initWithNibName:(nullable NSString *)nibNameOrNil bundle:(nullable NSBundle *)nibBundleOrNil {
  self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];

  _device = MTLCreateSystemDefaultDevice();
  _commandQueue = [_device newCommandQueue];

  if (!self.device) {
    NSLog(@"Metal is not supported");
    abort();
  }

  // Setup Dear ImGui context
  // FIXME: This example doesn't have proper cleanup...
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
  //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

  // Setup Dear ImGui style
  //ImGui::StyleColorsDark();
  //ImGui::StyleColorsClassic();

  // Setup Renderer backend
  ImGui_ImplMetal_Init(_device);

  // Load Fonts
  // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
  // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
  // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
  // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
  // - Read 'docs/FONTS.txt' for more instructions and details.
  // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
  //io.Fonts->AddFontDefault();
  //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
  //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
  //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
  //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
  //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
  //IM_ASSERT(font != NULL);

  return self;
}

- (void)initImGUI {
  NSLog(@"initImGUI");
  self.mtkView.device = self.device;
  self.mtkView.delegate = self;

#if TARGET_OS_OSX
  // Add a tracking area in order to receive mouse events whenever the mouse is within the bounds of our view
  NSTrackingArea *trackingArea =
  [[NSTrackingArea alloc] initWithRect:NSZeroRect
                               options:NSTrackingMouseMoved | NSTrackingInVisibleRect | NSTrackingActiveAlways
                                 owner:self
                              userInfo:nil];
  [self.view addTrackingArea:trackingArea];

  // If we want to receive key events, we either need to be in the responder chain of the key view,
  // or else we can install a local monitor. The consequence of this heavy-handed approach is that
  // we receive events for all controls, not just Dear ImGui widgets. If we had native controls in our
  // window, we'd want to be much more careful than just ingesting the complete event stream.
  // To match the behavior of other backends, we pass every event down to the OS.
  NSEventMask eventMask = NSEventMaskKeyDown | NSEventMaskKeyUp | NSEventMaskFlagsChanged | NSEventTypeScrollWheel;
  [NSEvent addLocalMonitorForEventsMatchingMask:eventMask handler:^NSEvent * _Nullable(NSEvent *event) {
    ImGui_ImplOSX_HandleEvent(event, self.view);
    return event;
  }];

  ImGui_ImplOSX_Init();
#endif

  [self _updateFramebuffer];
  app.init();
  _appInitialized = YES;
}

- (MTKView *)mtkView {
  return (MTKView *)self.view;
}

- (void)loadView {
  self.view = [[MTKView alloc] initWithFrame:CGRectMake(0, 0, 1200, 720)];
}

- (void)viewDidLoad {
  [super viewDidLoad];
  NSLog(@"viewDidLoad");
  [self initImGUI];
}

#pragma mark - MTKViewDelegate

- (void)_updateFramebuffer {
  NSLog(@"_updateFramebuffer");
  ImGuiIO& io = ImGui::GetIO();
#if TARGET_OS_OSX
  CGFloat framebufferScale = self.mtkView.window.screen.backingScaleFactor ?: NSScreen.mainScreen.backingScaleFactor;
#else
  CGFloat framebufferScale = self.mtkView.window.screen.scale ?: UIScreen.mainScreen.scale;
#endif
  io.DisplayFramebufferScale = ImVec2(framebufferScale, framebufferScale);
  if (_appInitialized) {
    app.framebufferDidChange();
  }
}

- (void)drawInMTKView:(MTKView*)view {
  //NSLog(@"drawInMTKView %@", view);
  ImGuiIO& io = ImGui::GetIO();
  io.DisplaySize.x = view.bounds.size.width;
  io.DisplaySize.y = view.bounds.size.height;

  io.DeltaTime = 1 / float(view.preferredFramesPerSecond ?: 60);

  id<MTLCommandBuffer> commandBuffer = [self.commandQueue commandBuffer];

  MTLRenderPassDescriptor* renderPassDescriptor = view.currentRenderPassDescriptor;
  if (renderPassDescriptor != nil) {
    renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(
      app.clearColor[0] * app.clearColor[3],
      app.clearColor[1] * app.clearColor[3],
      app.clearColor[2] * app.clearColor[3],
      app.clearColor[3]);

    // Here, you could do additional rendering work, including other passes as necessary.

    id <MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
    [renderEncoder pushDebugGroup:@"imgui"];

    // Start the Dear ImGui frame
    ImGui_ImplMetal_NewFrame(renderPassDescriptor);
#if TARGET_OS_OSX
    ImGui_ImplOSX_NewFrame(view);
#endif
    ImGui::NewFrame();

    app.renderFrame();

    // Rendering
    ImGui::Render();
    ImDrawData* draw_data = ImGui::GetDrawData();
    ImGui_ImplMetal_RenderDrawData(draw_data, commandBuffer, renderEncoder);

    [renderEncoder popDebugGroup];
    [renderEncoder endEncoding];

    [commandBuffer presentDrawable:view.currentDrawable];
  }

  [commandBuffer commit];
}

- (void)mtkView:(MTKView*)view drawableSizeWillChange:(CGSize)size {
  [self _updateFramebuffer];
  app.framebufferDidChange();
}

#pragma mark - Interaction

#if TARGET_OS_OSX

- (void)mouseMoved:(NSEvent *)event {
  ImGui_ImplOSX_HandleEvent(event, self.view);
}

- (void)mouseDown:(NSEvent *)event {
  ImGui_ImplOSX_HandleEvent(event, self.view);
}

- (void)rightMouseDown:(NSEvent *)event {
  ImGui_ImplOSX_HandleEvent(event, self.view);
}

- (void)otherMouseDown:(NSEvent *)event {
  ImGui_ImplOSX_HandleEvent(event, self.view);
}

- (void)mouseUp:(NSEvent *)event {
  ImGui_ImplOSX_HandleEvent(event, self.view);
}

- (void)rightMouseUp:(NSEvent *)event {
  ImGui_ImplOSX_HandleEvent(event, self.view);
}

- (void)otherMouseUp:(NSEvent *)event {
  ImGui_ImplOSX_HandleEvent(event, self.view);
}

- (void)mouseDragged:(NSEvent *)event {
  ImGui_ImplOSX_HandleEvent(event, self.view);
}

- (void)rightMouseDragged:(NSEvent *)event {
  ImGui_ImplOSX_HandleEvent(event, self.view);
}

- (void)otherMouseDragged:(NSEvent *)event {
  ImGui_ImplOSX_HandleEvent(event, self.view);
}

- (void)scrollWheel:(NSEvent *)event {
  ImGui_ImplOSX_HandleEvent(event, self.view);
}

#else

// This touch mapping is super cheesy/hacky. We treat any touch on the screen
// as if it were a depressed left mouse button, and we don't bother handling
// multitouch correctly at all. This causes the "cursor" to behave very erratically
// when there are multiple active touches. But for demo purposes, single-touch
// interaction actually works surprisingly well.
- (void)updateIOWithTouchEvent:(UIEvent *)event {
  UITouch *anyTouch = event.allTouches.anyObject;
  CGPoint touchLocation = [anyTouch locationInView:self.view];
  ImGuiIO &io = ImGui::GetIO();
  io.MousePos = ImVec2(touchLocation.x, touchLocation.y);

  BOOL hasActiveTouch = NO;
  for (UITouch *touch in event.allTouches) {
    if (touch.phase != UITouchPhaseEnded && touch.phase != UITouchPhaseCancelled) {
      hasActiveTouch = YES;
      break;
    }
  }
  io.MouseDown[0] = hasActiveTouch;
}

- (void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
  [self updateIOWithTouchEvent:event];
}

- (void)touchesMoved:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
  [self updateIOWithTouchEvent:event];
}

- (void)touchesCancelled:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
  [self updateIOWithTouchEvent:event];
}

- (void)touchesEnded:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
  [self updateIOWithTouchEvent:event];
}

#endif

@end

#pragma mark - Application Delegate

#if TARGET_OS_OSX

@interface AppDelegate : NSObject <NSApplicationDelegate>
@property (nonatomic, strong) NSWindow *window;
@end

@implementation AppDelegate

- (instancetype)init {
  if (self = [super init]) {
    NSViewController *rootViewController = [[ViewController alloc] initWithNibName:nil bundle:nil];
    self.window = [[NSWindow alloc] initWithContentRect:NSZeroRect
                                              styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable
                                                backing:NSBackingStoreBuffered
                                                  defer:NO];
    self.window.contentViewController = rootViewController;
    [self.window orderFront:self];
    [self.window center];
    [self.window becomeKeyWindow];
  }
  return self;
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
  return YES;
}

@end

#else

@interface AppDelegate : UIResponder <UIApplicationDelegate>
@property (strong, nonatomic) UIWindow *window;
@end

@implementation AppDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary<UIApplicationLaunchOptionsKey,id> *)launchOptions {
  UIViewController *rootViewController = [[ViewController alloc] init];
  self.window = [[UIWindow alloc] initWithFrame:UIScreen.mainScreen.bounds];
  self.window.rootViewController = rootViewController;
  [self.window makeKeyAndVisible];
  return YES;
}

@end

#endif

#pragma mark - main()

#if TARGET_OS_OSX

int main(int argc, const char * argv[]) {
  // return NSApplicationMain(argc, argv);
  @autoreleasepool {
    NSApplication* app = [NSApplication sharedApplication];
    AppDelegate* appDelegate = [AppDelegate new];
    [app setDelegate:appDelegate];
    [app run];
  }
  return 0;
}

#else

int main(int argc, char * argv[]) {
  @autoreleasepool {
    return UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
  }
}

#endif
