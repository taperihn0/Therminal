#include "RenderFormat.hpp"

namespace Thr
{

RenderFormat::RenderFormat(int window_width,
                		   int window_height,
                		   int cell_width,
                		   int cell_height,
                		   int cell_offset_x,
                		   int cell_offset_y)
	: _window_size(window_width, window_height)
	, _cell_size(cell_width, cell_height)
	, _cell_offset(cell_offset_x, cell_offset_y)
{}

glm::ivec2 RenderFormat::setWindowSize(glm::ivec2 size)
{
	return _window_size = size;
}

glm::ivec2 RenderFormat::setCellSize(glm::ivec2 size)
{
	return _cell_size = size;
}

glm::ivec2 RenderFormat::setCellOffset(glm::ivec2 offset)
{
	return _cell_offset = offset;
}

glm::ivec2 RenderFormat::getWindowSize() const
{
	return _window_size;
}

glm::ivec2 RenderFormat::getCellSize() const
{
	return _cell_size;
}

glm::ivec2 RenderFormat::getCellOffset() const
{
	return _cell_offset;
}

int RenderFormat::getCellCountVertical() const
{
	return _window_size.x / (_cell_size.x + _cell_offset.x);
}

int RenderFormat::getCellCountHorizontal() const
{
	return _window_size.y / (_cell_size.y + _cell_offset.y);
}

} // namespace Thr
