#pragma once

#include "Common.hpp"

namespace Thr
{

/* Specify character format on the screen.
*  That include: cell width and height, offset between neighbour cells
*  on x and y axis.
*/
class RenderFormat
{
public:
	RenderFormat() = default;
	RenderFormat(int window_width,
				 int window_height,
				 int cell_width,
				 int cell_height,
				 int cell_offset_x,
				 int cell_offset_y);

	RenderFormat& operator=(const RenderFormat&) = default;

	glm::ivec2 setWindowSize(glm::ivec2 size);
	glm::ivec2 setCellSize(glm::ivec2 size);
	glm::ivec2 setCellOffset(glm::ivec2 offset);

	glm::ivec2 getWindowSize() const;
	glm::ivec2 getCellSize() const;
	glm::ivec2 getCellOffset() const;

	int getCellCountVertical() const;
	int getCellCountHorizontal() const;
private:
	glm::ivec2 _window_size;
	glm::ivec2 _cell_size;
	glm::ivec2 _cell_offset;
};

} // namespace Thr
