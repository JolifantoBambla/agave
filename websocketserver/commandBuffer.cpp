#include "commandBuffer.h"

#include "command.h"

#include <vector>
#include <assert.h>

#if HAVE_BYTESWAP_H
#include <byteswap.h>
#else
#define bswap_16(value) \
	((((value) & 0xff) << 8) | ((value) >> 8))

#define bswap_32(value) \
	(((uint32_t)bswap_16((uint16_t)((value) & 0xffff)) << 16) | (uint32_t)bswap_16((uint16_t)((value) >> 16)))

#define bswap_64(value) \
	(((uint64_t)bswap_32((uint32_t)((value) & 0xffffffff)) << 32) | (uint64_t)bswap_32((uint32_t)((value) >> 32)))
#endif

commandBuffer::commandBuffer(size_t len, const uint8_t* buf) 
:_length(len), _headPos(buf)
{

}

commandBuffer::~commandBuffer() {

}
CommandBufferIterator::CommandBufferIterator(commandBuffer* buf)
	: _commandBuffer(buf), 
	_currentPos(const_cast<uint8_t*>(buf->head()))
{
}

//////////////////
// forward declare.
Command* parseSessionCommand(CommandBufferIterator* c);
Command* parseAssetPathCommand(CommandBufferIterator* c);
Command* parseLoadOmeTifCommand(CommandBufferIterator* c);
Command* parseSetCameraPosCommand(CommandBufferIterator* c);
Command* parseSetCameraUpCommand(CommandBufferIterator* c);
Command* parseSetCameraTargetCommand(CommandBufferIterator* c);
Command* parseSetCameraTargetCommand(CommandBufferIterator* c);
Command* parseSetCameraApertureCommand(CommandBufferIterator* c);
Command* parseSetCameraFovYCommand(CommandBufferIterator* c);
Command* parseSetCameraFocalDistanceCommand(CommandBufferIterator* c);
Command* parseSetCameraExposureCommand(CommandBufferIterator* c);
Command* parseSetDiffuseColorCommand(CommandBufferIterator* c);
Command* parseSetSpecularColorCommand(CommandBufferIterator* c);
Command* parseSetEmissiveColorCommand(CommandBufferIterator* c);
Command* parseSetRenderIterationsCommand(CommandBufferIterator* c);
Command* parseSetStreamModeCommand(CommandBufferIterator* c);
Command* parseRequestRedrawCommand(CommandBufferIterator* c);
Command* parseSetResolutionCommand(CommandBufferIterator* c);
Command* parseSetDensityCommand(CommandBufferIterator* c);
Command* parseFrameSceneCommand(CommandBufferIterator* c);
Command* parseSetGlossinessCommand(CommandBufferIterator* c);
Command* parseEnableChannelCommand(CommandBufferIterator* c);
Command* parseSetWindowLevelCommand(CommandBufferIterator* c);

#define CMD_CASE(N, CMDCLASS) \
	case N:\
		return parse##CMDCLASS(&iterator);\
		break;

void commandBuffer::processBuffer()
{
	CommandBufferIterator iterator(this);
	while (!iterator.end()) {
		// new command. 
		// read its int32 enum value.
		int32_t cmd = iterator.parseInt32();

		// lambda that takes our iterator and the cmd id to initialize the command object.
		Command* c = [cmd, &iterator]() -> Command* {
			try {
				switch (cmd) {
					CMD_CASE(0, SessionCommand);
					CMD_CASE(1, AssetPathCommand);
					CMD_CASE(2, LoadOmeTifCommand);
					CMD_CASE(3, SetCameraPosCommand);
					CMD_CASE(4, SetCameraTargetCommand);
					CMD_CASE(5, SetCameraUpCommand);
					CMD_CASE(6, SetCameraApertureCommand);
					CMD_CASE(7, SetCameraFovYCommand);
					CMD_CASE(8, SetCameraFocalDistanceCommand);
					CMD_CASE(9, SetCameraExposureCommand);
					CMD_CASE(10, SetDiffuseColorCommand);
					CMD_CASE(11, SetSpecularColorCommand);
					CMD_CASE(12, SetEmissiveColorCommand);
					CMD_CASE(13, SetRenderIterationsCommand);
					CMD_CASE(14, SetStreamModeCommand);
					CMD_CASE(15, RequestRedrawCommand);
					CMD_CASE(16, SetResolutionCommand);
					CMD_CASE(17, SetDensityCommand);
					CMD_CASE(18, FrameSceneCommand);
					CMD_CASE(19, SetGlossinessCommand);
					CMD_CASE(20, EnableChannelCommand);
					CMD_CASE(21, SetWindowLevelCommand);
				default:
					// ERROR UNRECOGNIZED COMMAND SIGNATURE.  
					// PRINT OUT PREVIOUS! BAIL OUT! OR DO SOMETHING CLEVER AND CORRECT!
					return nullptr;
					break;
				}
			}
			catch (...) {
				// buffer error?
				return nullptr;
			}
			return nullptr;
		} ();

		if (c) {
			// good! add to list of commands
			_commands.push_back(c);
		}
		else {
			// error! do something.
		}
	}
}

void commandBuffer::execute(ExecutionContext* c)
{
	// let's run all the commands now
	for (auto i = _commands.begin(); i != _commands.end(); ++i) {
		(*i)->execute(c);
	}
}

bool CommandBufferIterator::end() {
	return (_currentPos >= _commandBuffer->head() + _commandBuffer->length());
}

int32_t CommandBufferIterator::parseInt32() {
	int32_t value = bswap_32(*((int32_t*)(_currentPos)));
	_currentPos += sizeof(int32_t);
	return value;
}

float CommandBufferIterator::parseFloat32() {
	// assuming sizeof float == sizeof int32 == 4
	//float value = (float)bswap_32(*((int32_t*)(_currentPos)));
	float value = *((float*)(_currentPos));
	_currentPos += sizeof(float);
	return value;
}

std::string CommandBufferIterator::parseString() {
	int32_t len = parseInt32();
	std::string s(reinterpret_cast<char const*>(_currentPos), (size_t)len);
	_currentPos += len;
	return s;
}

////////////////////////////////////////////
Command* parseSessionCommand(CommandBufferIterator* c) {
	SessionCommandD data;
	data._name = c->parseString();
	return new SessionCommand(data);
}
Command* parseAssetPathCommand(CommandBufferIterator* c) {
	AssetPathCommandD data;
	data._name = c->parseString();
	return new AssetPathCommand(data);
}
Command* parseLoadOmeTifCommand(CommandBufferIterator* c) {
	LoadOmeTifCommandD data;
	data._name = c->parseString();
	return new LoadOmeTifCommand(data);
}
Command* parseSetCameraPosCommand(CommandBufferIterator* c) {
	SetCameraPosCommandD data;
	data._x = c->parseFloat32();
	data._y = c->parseFloat32();
	data._z = c->parseFloat32();
	return new SetCameraPosCommand(data);
}
Command* parseSetCameraUpCommand(CommandBufferIterator* c) {
	SetCameraUpCommandD data;
	data._x = c->parseFloat32();
	data._y = c->parseFloat32();
	data._z = c->parseFloat32();
	return new SetCameraUpCommand(data);
}
Command* parseSetCameraTargetCommand(CommandBufferIterator* c) {
	SetCameraTargetCommandD data;
	data._x = c->parseFloat32();
	data._y = c->parseFloat32();
	data._z = c->parseFloat32();
	return new SetCameraTargetCommand(data);
}
Command* parseSetCameraApertureCommand(CommandBufferIterator* c) {
	SetCameraApertureCommandD data;
	data._x = c->parseFloat32();
	return new SetCameraApertureCommand(data);
}
Command* parseSetCameraFovYCommand(CommandBufferIterator* c) {
	SetCameraFovYCommandD data;
	data._x = c->parseFloat32();
	return new SetCameraFovYCommand(data);
}
Command* parseSetCameraFocalDistanceCommand(CommandBufferIterator* c) {
	SetCameraFocalDistanceCommandD data;
	data._x = c->parseFloat32();
	return new SetCameraFocalDistanceCommand(data);
}
Command* parseSetCameraExposureCommand(CommandBufferIterator* c) {
	SetCameraExposureCommandD data;
	data._x = c->parseFloat32();
	return new SetCameraExposureCommand(data);
}
Command* parseSetDiffuseColorCommand(CommandBufferIterator* c) {
	SetDiffuseColorCommandD data;
	data._channel = c->parseInt32();
	data._r = c->parseFloat32();
	data._g = c->parseFloat32();
	data._b = c->parseFloat32();
	data._a = c->parseFloat32();
	return new SetDiffuseColorCommand(data);
}
Command* parseSetSpecularColorCommand(CommandBufferIterator* c) {
	SetSpecularColorCommandD data;
	data._channel = c->parseInt32();
	data._r = c->parseFloat32();
	data._g = c->parseFloat32();
	data._b = c->parseFloat32();
	data._a = c->parseFloat32();
	return new SetSpecularColorCommand(data);
}
Command* parseSetEmissiveColorCommand(CommandBufferIterator* c) {
	SetEmissiveColorCommandD data;
	data._channel = c->parseInt32();
	data._r = c->parseFloat32();
	data._g = c->parseFloat32();
	data._b = c->parseFloat32();
	data._a = c->parseFloat32();
	return new SetEmissiveColorCommand(data);
}
Command* parseSetRenderIterationsCommand(CommandBufferIterator* c) {
	SetRenderIterationsCommandD data;
	data._x = c->parseInt32();
	return new SetRenderIterationsCommand(data);
}
Command* parseSetStreamModeCommand(CommandBufferIterator* c) {
	SetStreamModeCommandD data;
	data._x = c->parseInt32();
	return new SetStreamModeCommand(data);
}
Command* parseRequestRedrawCommand(CommandBufferIterator* c) {
	RequestRedrawCommandD data;
	return new RequestRedrawCommand(data);
}
Command* parseSetResolutionCommand(CommandBufferIterator* c) {
	SetResolutionCommandD data;
	data._x = c->parseInt32();
	data._y = c->parseInt32();
	return new SetResolutionCommand(data);
}
Command* parseSetDensityCommand(CommandBufferIterator* c) {
	SetDensityCommandD data;
	data._x = c->parseFloat32();
	return new SetDensityCommand(data);
}
Command* parseFrameSceneCommand(CommandBufferIterator* c) {
	FrameSceneCommandD data;
	return new FrameSceneCommand(data);
}
Command* parseSetGlossinessCommand(CommandBufferIterator* c) {
	SetGlossinessCommandD data;
	data._channel = c->parseInt32();
	data._glossiness = c->parseFloat32();
	return new SetGlossinessCommand(data);
}
Command* parseEnableChannelCommand(CommandBufferIterator* c) {
	EnableChannelCommandD data;
	data._channel = c->parseInt32();
	data._enabled = c->parseInt32();
	return new EnableChannelCommand(data);
}
Command* parseSetWindowLevelCommand(CommandBufferIterator* c) {
	SetWindowLevelCommandD data;
	data._channel = c->parseInt32();
	data._window = c->parseFloat32();
	data._level = c->parseFloat32();
	return new SetWindowLevelCommand(data);
}
