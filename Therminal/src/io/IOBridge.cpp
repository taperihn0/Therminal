#include "IOBridge.hpp"

namespace Thr
{

void _IOClient::bindBridge(std::shared_ptr<IOBridge>& bridge)
{
	_bridge = bridge;
}

const InputRingBuffer& _IOClient::getInputBuf() const
{
	THR_HARD_ASSERT_LOG(_bridge, "Failed to query unbounded client");
	return _bridge->getInputBuf();
}

const OutputBuffer& _IOClient::getOutputBuf() const
{
	THR_HARD_ASSERT_LOG(_bridge, "Failed to query unbounded client");
	return _bridge->getOutputBuf();
}

template <EventCode C>
void IOAppClient::sendEvent(KeyButtonEvent<C>& ev)
{
	if (!_bridge) {
		THR_LOG_FATAL("Unbounded IO bridge");
		return;
	}

	const std::string data = _input_ev_transl.translate(ev);

	if (!ev.isHandled())
		return;

	std::for_each(data.begin(), data.end(), [&](char c) {
		_bridge->_input_circ_buff.put(static_cast<byte>(c));
	});
}

void IOAppClient::readBytes(BytesBuf& buf)
{
	if (!_bridge) {
		THR_LOG_FATAL("Unbounded IO bridge");
		return;
	}

	const byte* ptr = _bridge->_output_buff.read(buf.n);
	THR_ASSERT(ptr);

	buf.ptr = ptr;

	_bridge->_output_buff.swap();
}

void IOShellClient::writeBytes(BytesBuf buf)
{
	if (!_bridge) {
		THR_LOG_FATAL("Unbounded IO bridge");
		return;
	}

	_bridge->_output_buff.write(buf.ptr, buf.n);
}

bool IOShellClient::readBytes(MutBytesBuf& buf)
{
	if (!_bridge) {
		THR_LOG_FATAL("Unbounded IO bridge");
		return false;
	}

	if (!_bridge->_input_circ_buff.isReady())
		return false;

	const int n = _bridge->_input_circ_buff.getIncomingCnt();

	if (buf.n < n) {
		THR_LOG_FATAL("Too little buffer size for incoming bytes");
		return  false;
	}

	buf.n = 0;

	while (_bridge->_input_circ_buff.isReady()) {
		const byte c = _bridge->_input_circ_buff.get();	
		buf.ptr[buf.n++] = c;
	}

	return true;
}

InputRingBuffer& IOBridge::getInputBuf()
{
	return _input_circ_buff;
}

const InputRingBuffer& IOBridge::getInputBuf() const
{
	return _input_circ_buff;
}

OutputBuffer& IOBridge::getOutputBuf()
{
	return _output_buff;	
}

const OutputBuffer& IOBridge::getOutputBuf() const
{
	return _output_buff;
}

IOBridge::IOBridge(size_t input_buf_size, size_t output_buf_size)
	: _input_circ_buff(input_buf_size)
	, _output_buff(output_buf_size)
{}

template void IOAppClient::sendEvent(KeyPressEvent& ev);
template void IOAppClient::sendEvent(KeyTypeEvent& ev);
template void IOAppClient::sendEvent(KeyRepeatEvent& ev);

} // namespace Thr
