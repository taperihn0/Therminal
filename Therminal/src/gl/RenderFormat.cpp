#include "RenderFormat.hpp"

namespace Thr
{

RenderFormat::RenderFormat(int padding_x,
				 		   int padding_y,
						   int cell_count_x,
						   int cell_count_y)
	: _window_size(0, 0)
	, _cell_size(0, 0)
	, _padding(padding_x, padding_y)
	, _cell_count(cell_count_y, cell_count_x)
{}

glm::ivec2 RenderFormat::setWindowSize(glm::ivec2 size)
{
	return _window_size = size;
}

glm::ivec2 RenderFormat::setCellSize(glm::ivec2 size)
{
	return _cell_size = size;
}

glm::ivec2 RenderFormat::setPadding(glm::ivec2 padding)
{
	return _padding = padding;
}

glm::ivec2 RenderFormat::setCellCount(glm::ivec2 cell_count)
{
	return _cell_count = cell_count;
}

glm::ivec2 RenderFormat::getWindowSize() const
{
	return _window_size;
}

glm::ivec2 RenderFormat::getCellSize() const
{
	return _cell_size;
}

glm::ivec2 RenderFormat::getPadding() const
{
	return _padding;
}

glm::ivec2 RenderFormat::getCellCount() const
{
	return _cell_count;
}

} // namespace Thr
