#include "Window.hpp"
#include "logger/Log.hpp"
#include "input/InputQueue.hpp"

namespace Thr
{

Window* Window::_glob_bind = nullptr;

Window::Window()
   : _width(-1)
   , _height(-1)
   , _focus(false)
   , _suspended(true)
   , _title(_DefaultWindowTitle)
   , _initialized(false)
   , _close(false)
   , _native_window(nullptr)
   , _input(nullptr)
   //, _swapchain(nullptr)
   //, _callback_ptrs({ this, nullptr })
{}

Window::Window(uint width, uint height, const std::string& title)
   : Window()
{ 
   bool status = init(width, height, title);
   THR_ASSERT_LOG(status, "Window initialization end with a failure");
}

Window::~Window()
{
   /* Close window state */

   delete _input;

   if (_native_window) 
      glfwDestroyWindow(_native_window);

   if (glfwIsInitialized())
      glfwTerminate();

   _initialized = false;
}

Window* Window::getGlobWindow() {
   return _glob_bind;
}

bool Window::init(uint width, uint height, const std::string& title) 
{
   if (_initialized) {
      THR_ASSERT_LOG(false, "Trying to initialize already initialized window!");
      return false;
   }

   // Check if we already created global window
   if (_glob_bind != nullptr) {
      THR_ASSERT_LOG(false, "Window already created and initialized!");
      return false;
   }

   // Initialize GLFW context
   if (glfwInit() == GLFW_FALSE) {
      THR_LOG_FATAL_FRAME_INFO("Failed to initialize GLFW context!");
      return false;
   }

   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
   glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
   glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
   glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
   glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);
   glfwWindowHint(GLFW_CENTER_CURSOR, GLFW_TRUE);
   glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);

#ifdef THR_DEBUG
   glfwWindowHint(GLFW_CONTEXT_DEBUG, GLFW_TRUE);
#else
   glfwWindowHint(GLFW_CONTEXT_DEBUG, GLFW_FALSE);
#endif

#ifdef THR_OS_WINDOWS
   glfwWindowHint(GLFW_WIN32_KEYBOARD_MENU, GLFW_TRUE);
#elif defined(THR_OS_MACOS)
   glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

   _width = width;
   _height = height;

   if ((_native_window = glfwCreateWindow(_width, _height, title.c_str(), nullptr, nullptr)) == NULL) {
      THR_LOG_FATAL_FRAME_INFO("Failed to create a new GLFW window!");
      return false;
   }

   glfwMakeContextCurrent(_native_window);

   // _callback_ptrs lifetime is same as the lifetime of *this object.
   //void* data = reinterpret_cast<void*>(&_callback_ptrs);
   //glfwSetWindowUserPointer(_native_window, data);

   // initialize render state
   // TODO: make it configurable
   //bool render_init_status = Render::initialize(RENDER_GRAPHICS_API_OPENGL);
   //THR_ASSERT_LOG(render_init_status, "Failed to initialize render");
   //_swapchain = std::make_unique<GL_Swapchain>(_native_window, _width, _height);

   // GLFW_FOCUSED set
   _focus = true;
   _suspended = false;
   _initialized = true;

   THR_LOG_INFO("Successfully initialized a new GLFW window.");
   _glob_bind = this;

   // We can now allocate our input handler queue
   _input = new WinInputQueue;

   return true;
}

void Window::update()
{
   THR_HARD_ASSERT(_initialized);
   _input->waitEvents();
   //_swapchain->present();
}

bool Window::isOpen() const
{
   THR_HARD_ASSERT(_initialized);
   return !_close;
}

bool Window::isSuspended() const
{
   THR_HARD_ASSERT(_initialized);
   return _suspended;
}


} // namespace Thr

