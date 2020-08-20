//
//  presentation.h
//  probmon
//
//  Created by Mike Gonzales on 8/17/20.
//

#ifndef presentation_h
#define presentation_h

#include <stdint.h>
#include <bx/uint32_t.h>
#include "common.h"
#include "bgfx_utils.h"

class GraphicsSubmodule
{
public:
	uint32_t m_width;
	uint32_t m_height;
	uint32_t m_reset;

	void Init(
		bgfx::RendererType::Enum type,
		uint16_t pciId,
		uint32_t width,
		uint32_t height,
		uint32_t reset
	);
	void Update();
	void Shutdown();
};

class UiSubmodule
{
public:
	uint16_t m_width;
	uint16_t m_height;

	entry::MouseState m_mouseState;
	
	void Init(uint32_t width, uint32_t height);
	
	void Update();
	void Shutdown();
};

class DebugSubmodule
{
public:
	uint32_t m_debug;
	
	void Init(uint32_t debug);
	void Update();
	void Shutdown();
};

#endif /* presentation_h */
