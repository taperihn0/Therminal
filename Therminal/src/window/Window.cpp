#include "Window.hpp"
#include "logger/Log.hpp"
#include "io/InputQueue.hpp"

namespace Thr
{

Window* Window::_glob_bind = nullptr;

Window::_CallbackData::_CallbackData(Window* win)
	: window(win)
	, callbacks(std::make_shared<_EventCallbacks>())
{}

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
	, _user_callback_data(std::make_unique<_CallbackData>(this))
	//, _swapchain(nullptr)
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
	if (!glfwIsInitialized()) { 
		if (glfwInit() == GLFW_FALSE) {
			THR_LOG_FATAL_FRAME_INFO("Failed to initialize GLFW context!");
			return false;
		}
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
	void* user_data = reinterpret_cast<void*>(_user_callback_data.get());
	glfwSetWindowUserPointer(_native_window, user_data);

	// initialize render state

	// GLFW_FOCUSED set
	_focus = true;
	_suspended = false;
	_initialized = true;

	THR_LOG_INFO("Successfully initialized a new GLFW window.");
	_glob_bind = this;

	// We can now allocate our input handler queue
	_input = std::make_unique<WinInputQueue>();

	// setup event callbacks pointers
	setEventCallbacks();

	return true;
}

void Window::update()
{
	THR_HARD_ASSERT(_initialized);
	_input->pollEvents();
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

void Window::__setWidth(uint width)
{
	THR_ASSERT(_initialized);
	_width = width;
}

void Window::__setHeight(uint height)
{
	THR_ASSERT(_initialized);
	_height = height;
}

void Window::__setFocus(bool value)
{
	THR_ASSERT(_initialized);
	_focus = value;
}

void Window::__setSuspended(bool value)
{
	THR_ASSERT(_initialized);
	_suspended = value;
}

void Window::__setClose(bool value)
{
	THR_ASSERT(_initialized);
	_close = value;
}

void Window::setErrorCallback(ErrorCallback callbck)
{
	_user_callback_data->callbacks->error_callback = callbck;
}

void Window::setWindowResizeCallback(WindowResizeCallback callbck)
{
	_user_callback_data->callbacks->window_resize_callback = callbck;
}

void Window::setWindowMoveCallback(WindowMoveCallback callbck)
{
	_user_callback_data->callbacks->window_move_callback = callbck;
}

void Window::setWindowFocusCallback(WindowFocusCallback callbck)
{
	_user_callback_data->callbacks->window_focus_callback = callbck;
}

void Window::setWindowCloseCallback(WindowCloseCallback callbck)
{
	_user_callback_data->callbacks->window_close_callback = callbck;
}

void Window::setKeyPressCallback(KeyPressCallback callbck)
{
	_user_callback_data->callbacks->key_press_callback = callbck;
}

void Window::setKeyReleaseCallback(KeyReleaseCallback callbck)
{
	_user_callback_data->callbacks->key_release_callback = callbck;
}

void Window::setKeyRepeatCallback(KeyRepeatCallback callbck)
{
	_user_callback_data->callbacks->key_repeat_callback = callbck;
}

void Window::setKeyTypeCallback(KeyTypeCallback callbck)
{
	_user_callback_data->callbacks->key_type_callback = callbck;
}

void Window::setMousePressCallback(MousePressCallback callbck) 
{
	_user_callback_data->callbacks->mouse_press_callback = callbck;
}

void Window::setMouseReleaseCallback(MouseReleaseCallback callbck) 
{
	_user_callback_data->callbacks->mouse_release_callback = callbck;
}

void Window::setMouseMoveCallback(MouseMoveCallback callbck) 
{
	_user_callback_data->callbacks->mouse_move_callback = callbck;
}

void Window::setMouseScrollCallback(MouseScrollCallback callbck) 
{
	_user_callback_data->callbacks->mouse_scroll_callback = callbck;
}

void Window::setEventCallbacks()
{
	THR_ASSERT(_initialized);
	
	glfwSetErrorCallback(
		[](int code, const char* desc)
		{
			ErrorEvent ev(code, desc);
			_EventCallbacks::error_callback(ev);
		}
	);

	glfwSetWindowSizeCallback(_native_window,
		[](GLFWwindow* platform_native_window, int width, int height)
		{
			WindowResizeEvent ev(width, height);

			Window::_CallbackData* data = reinterpret_cast<Window::_CallbackData*>(glfwGetWindowUserPointer(platform_native_window));
			Window* window = data->window;
			std::shared_ptr<_EventCallbacks> callbacks = data->callbacks;

			window->__setWidth(width);
			window->__setHeight(height);

			const bool suspend = (!width && !height);
			window->__setSuspended(suspend);
			
			if (suspend) {
				THR_LOG_INFO("Window suspended");
			}

			callbacks->window_resize_callback(ev);
		}
	);
	
	glfwSetWindowPosCallback(_native_window, 
		[](GLFWwindow* platform_native_window, int xpos, int ypos)
		{
			WindowMoveEvent ev(xpos, ypos);

			Window::_CallbackData* data = reinterpret_cast<Window::_CallbackData*>(glfwGetWindowUserPointer(platform_native_window));
			std::shared_ptr<_EventCallbacks> callbacks = data->callbacks;

			callbacks->window_move_callback(ev);
		}
	);
	
	glfwSetWindowFocusCallback(_native_window, 
		[](GLFWwindow* platform_native_window, int value)
		{
			WindowFocusEvent ev(value);
			
			Window::_CallbackData* data = reinterpret_cast<Window::_CallbackData*>(glfwGetWindowUserPointer(platform_native_window));
			Window* window = data->window;
			std::shared_ptr<_EventCallbacks> callbacks = data->callbacks;

			window->__setFocus(value);

			callbacks->window_focus_callback(ev);
		}
	);

	glfwSetWindowCloseCallback(_native_window, 
		[](GLFWwindow* platform_native_window)
		{         
			Window::_CallbackData* data = reinterpret_cast<Window::_CallbackData*>(glfwGetWindowUserPointer(platform_native_window));
			Window* window = data->window;
			std::shared_ptr<_EventCallbacks> callbacks = data->callbacks;

			window->__setClose(true);

			callbacks->window_close_callback();
		}
	);

	glfwSetKeyCallback(_native_window,     
		[](GLFWwindow* platform_native_window, int key, int scancode, int action, int mods)
		{
			Window::_CallbackData* data = reinterpret_cast<Window::_CallbackData*>(glfwGetWindowUserPointer(platform_native_window));
			std::shared_ptr<_EventCallbacks> callbacks = data->callbacks;

			switch (action) {
			case GLFW_PRESS: {
				KeyPressEvent ev(key, scancode, mods);
				callbacks->key_press_callback(ev);
				break;
			}
			case GLFW_REPEAT: {
				KeyRepeatEvent ev(key, scancode, mods);
				callbacks->key_repeat_callback(ev);
				break;
			}
			case GLFW_RELEASE: {
				KeyReleaseEvent ev(key, scancode, mods);
				callbacks->key_release_callback(ev);
				break;
			}
			default: break;
			}
		}
	);

	glfwSetCharCallback(_native_window, 
		[](GLFWwindow* platform_native_window, unsigned int codepoint)
		{
			KeyTypeEvent ev(codepoint, 0, 0);

			Window::_CallbackData* data = reinterpret_cast<Window::_CallbackData*>(glfwGetWindowUserPointer(platform_native_window));
			THR_UNUSED Window* window = data->window;
			std::shared_ptr<_EventCallbacks> callbacks = data->callbacks;

			callbacks->key_type_callback(ev);
		}
	);

	glfwSetMouseButtonCallback(_native_window, 
		[](GLFWwindow* platform_native_window, int button, int action, int mods)
		{
			Window::_CallbackData* data = reinterpret_cast<Window::_CallbackData*>(glfwGetWindowUserPointer(platform_native_window));
			std::shared_ptr<_EventCallbacks> callbacks = data->callbacks;

			switch (action) {
			case GLFW_PRESS: {
				MousePressEvent ev(button, mods);
				callbacks->mouse_press_callback(ev);
				break;
			}
			case GLFW_REPEAT: {
				// Repeat action for mouse button currently 
				// not supported by GLFW nor Pheasant 
				break;
			}
			case GLFW_RELEASE: {
				MouseReleaseEvent ev(button, mods);
				callbacks->mouse_release_callback(ev);
				break;
			}
			default: break;
			}
		}
	);

	glfwSetCursorPosCallback(_native_window, 
		[](GLFWwindow* platform_native_window, double xpos, double ypos)
		{
			MouseMoveEvent ev(xpos, ypos);

			Window::_CallbackData* data = reinterpret_cast<Window::_CallbackData*>(glfwGetWindowUserPointer(platform_native_window));
			std::shared_ptr<_EventCallbacks> callbacks = data->callbacks;

			callbacks->mouse_move_callback(ev);
		}
	);

	glfwSetScrollCallback(_native_window, 
		[](GLFWwindow* platform_native_window, double xoffset, double yoffset)
		{
			MouseScrollEvent ev(xoffset, yoffset);
			
			Window::_CallbackData* data = reinterpret_cast<Window::_CallbackData*>(glfwGetWindowUserPointer(platform_native_window));
			std::shared_ptr<_EventCallbacks> callbacks = data->callbacks;

			callbacks->mouse_scroll_callback(ev);
		}
	);
}

} // namespace Thr

