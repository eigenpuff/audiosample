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
	static GraphicsSubmodule * sInstance;

public:
	GraphicsSubmodule()
	{
		BX_ASSERT(sInstance == nullptr);
		sInstance = this;
	}
	
	static GraphicsSubmodule * Instance() { return sInstance; }
	
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
	static UiSubmodule * sInstance;
	
public:
	UiSubmodule()
	{
		BX_ASSERT(sInstance == nullptr);
		sInstance = this;
	}
	
	static UiSubmodule * Instance() { return sInstance; }
	
	uint16_t m_width;
	uint16_t m_height;

	entry::MouseState m_mouseState;
	
	void Init(uint32_t width, uint32_t height);
	
	void Update();
	void Shutdown();
};

class DebugSubmodule
{
	static DebugSubmodule * sInstance;
	const char * debugStr = nullptr;
	
public:
	uint32_t m_debug;
	
	DebugSubmodule()
	{
		BX_ASSERT(sInstance == nullptr);
		sInstance = this;
	}
	
	static DebugSubmodule *Instance() { return sInstance; }
	
	void SetDebugString(const char * debugStr);
	const char * GetDebugString();
	
	void Init(uint32_t debug);
	void Update();
	void Shutdown();
};

#endif /* presentation_h */
