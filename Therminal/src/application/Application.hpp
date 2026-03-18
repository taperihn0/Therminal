#pragma once

#include "Common.hpp"
#include "window/Window.hpp"
#include "filesys/Filepath.hpp"
#include "memory/Memory.hpp"
#include "io/InputBuffer.hpp"
#include "io/OutputBuffer.hpp"
#include "io/InputTranslator.hpp"
#include "io/Worker.hpp"
#include "screen/Grid.hpp"
#include "io/OutputParser.hpp"
#include "gl/TextRender.hpp"
#include "gl/RenderFormat.hpp"
#include "shell/Shell.hpp"

namespace Thr 
{

class Application 
{
public:
	Application() = delete;
	Application(int argc, char* argv[]);
	~Application() = default;

	void run();
private:
	void init();
	void getPrimaryMonitorRes(int& width, int& height);

	/* custom event callbacks */
	static void winErrorCallback(ErrorEvent ev);
	static void winResizeCallback(WindowResizeEvent ev);
	static void winMoveCallback(WindowMoveEvent ev);
	static void winFocusCallback(WindowFocusEvent ev);
	static void winCloseCallback();
	static void winKeyPressCallback(KeyPressEvent ev);
	static void winKeyReleaseCallback(KeyReleaseEvent ev);
	static void winKeyRepeatCallback(KeyRepeatEvent ev);
	static void winKeyTypeCallback(KeyTypeEvent ev);
	static void winMousePressCallback(MousePressEvent ev);
	static void winMouseReleaseCallback(MouseReleaseEvent ev);
	static void winMouseMoveCallback(MouseMoveEvent ev);
	static void winMouseScrollCallback(MouseScrollEvent ev);
	
	FilePath                  _cwd;
	std::unique_ptr<Window>   _window;
	int                       _monitor_width;
	int                       _monitor_height;
	std::shared_ptr<Grid>     _grid;
	OutputParser			  _parser;
	RenderFormat 			  _render_fmt;
	TextRender				  _text_render;
	Shell 				      _shell;
	std::shared_ptr<IOBridge> _io_bridge;
	static IOAppClient		  _client;
};

} // namespace Thr
