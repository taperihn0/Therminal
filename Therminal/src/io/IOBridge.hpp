#pragma once

#include "Common.hpp"
#include "io/InputTranslator.hpp"
#include "io/InputBuffer.hpp"
#include "io/OutputBuffer.hpp"
#include "io/OutputTranslator.hpp"

namespace Thr
{

THR_DECLARE class IoAppClient;
THR_DECLARE class IoShellClient;

/* Specifies a bridge between app client
*  and shell client. Can be easily used in order
*  to pass events to shell and pass incoming byte stream
*  from the shell.
*/
class IOBridge
{
public:
	friend class IOAppClient;
	friend class IOShellClient;

	IOBridge(size_t input_buf_size, size_t output_buf_size);

	InputRingBuffer& getInputBuf();
	const InputRingBuffer& getInputBuf() const;

	OutputBuffer& getOutputBuf();
	const OutputBuffer& getOutputBuf() const;
private:
	InputRingBuffer _input_circ_buff;
	OutputBuffer    _output_buff;
};

template <typename T>
struct Buf
{
	T*  ptr;
	int n;
};

using BytesBuf = Buf<const byte>;
using MutBytesBuf = Buf<byte>;

class _IOClient
{
public:
	_IOClient() = default;
	
	/* We should bind global IO bridge
	*/
	void bindBridge(std::shared_ptr<IOBridge>& bridge);

	const InputRingBuffer& getInputBuf() const;
	const OutputBuffer& getOutputBuf() const;
protected:
	std::shared_ptr<IOBridge> _bridge = nullptr;
};

/* That is the app channel.
*  Sending events to the bounded client
*  means sending it to the bounded shell.
*/
class IOAppClient : public _IOClient
{
public:
	IOAppClient() = default;

	/* Encodes incoming 'ev' event 
	*  and sends them to the bounded IO bridge.
	*/
	template <EventCode C>
	void sendEvent(KeyButtonEvent<C>& ev);
	
	/* Returns incoming data stream
	*  since last 'readBytes' call.
	*/
	void readBytes(BytesBuf& buf);
private:
    InputEvTransl _input_ev_transl;
};

/* Shell-side client.
*/
class IOShellClient : public _IOClient
{
public:
	IOShellClient() = default;

	/* Writes given bytes to the bounded output.
	*/
	void writeBytes(BytesBuf buf);

	/* Reads bytes.
	*  Returns true on valid read operation.
	*  If input buffer is empty, returns false.
	*/
	bool readBytes(MutBytesBuf& buf);
private: 
};

} // namespace Thr
