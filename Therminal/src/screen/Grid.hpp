#pragma once

#include "Common.hpp"
#include "Line.hpp"
#include "gl/RenderFormat.hpp"
#include "Scrollback.hpp"

namespace Thr
{
 
class Grid
{
public:
	Grid(const RenderFormat& format);

	void specifyScrollbackBuffer(std::shared_ptr<ScrollbackBuffer>& scrollback);
	
	void setCursorPos(glm::u64vec2 pos);
	void eraseRightFromCursor();
	void eraseLeftFromCursor();
	void eraseAll();
	void eraseRightLineFromCursor();
	void eraseLeftLineFromCursor();
	void eraseAllLine();
	void clearScrollbackBuffer();

	glm::u64vec2 getCursorPos() const;
	std::shared_ptr<const LineView> getLineView() const;
	
	void putGraphemeCluster(const GraphemeCluster& cluster, const EscapeState* state);
private:
	void init();
	Line& getActiveLine();
	void safeAdvanceWritePosY();

	CircularBuff<Line> 				  _ln_buf;
	glm::u32vec2 	   				  _write_pos;
	std::shared_ptr<LineView> 		  _ln_ptrs;
	size_t 							  _after_nl_pos;
	glm::u32vec2 				 	  _cell_cnt;
	RenderFormat 					  _fmt;
	bool 							  _formated;
	std::shared_ptr<ScrollbackBuffer> _scrollback;
};

} // namespace Thr
