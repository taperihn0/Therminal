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
	RenderFormat(int padding_x,
				 int padding_y,
				 int cell_count_x,
				 int cell_count_y);

	RenderFormat& operator=(const RenderFormat&) = default;

	glm::ivec2 setWindowSize(glm::ivec2 size);
	glm::ivec2 setCellSize(glm::ivec2 size);
	glm::ivec2 setPadding(glm::ivec2 padding);
	glm::ivec2 setCellCount(glm::ivec2 cell_count);

	glm::ivec2 getWindowSize() const;
	glm::ivec2 getCellSize() const;
	glm::ivec2 getPadding() const;
	glm::ivec2 getCellCount() const;

	static inline constexpr int DefaultFontHeightPix = 20;
	static inline constexpr int DeafultPaddingPixX = 0;
	static inline constexpr int DeafultPaddingPixY = 0;
	static inline constexpr int DefaultCellCountX = 80;
	static inline constexpr int DefaultCellCountY = 24;
private:
	glm::ivec2 _window_size;
	glm::ivec2 _cell_size;
	glm::ivec2 _padding;
	glm::ivec2 _cell_count;
};

} // namespace Thr
