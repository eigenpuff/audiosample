
/*
 * Based of of bgfx examples source, provides platform independent
 * entrypoint for graphical applications.
 */

#include "presentation_modules.h"
#include "audio_module.h"

namespace
{

class AppWrapper : public entry::AppI
{
	GraphicsSubmodule m_graphics;
	UiSubmodule m_ui;
	DebugSubmodule m_debug;
	AudioSubmodule m_audio;
	
public:
	AppWrapper(const char* _name, const char* _description, const char* _url)
		: entry::AppI(_name, _description, _url)
	{
	}

	void init(int32_t _argc, const char* const* _argv, uint32_t _width, uint32_t _height) override
	{
		Args args(_argc, _argv);

		m_graphics.Init(args.m_type, args.m_pciId, _width, _height, BGFX_RESET_VSYNC);
		m_debug.Init(BGFX_DEBUG_TEXT);
		m_ui.Init(_width, _height);
		
		m_audio.Init();
	}

	virtual int shutdown() override
	{
		m_audio.Shutdown();
		m_ui.Shutdown();
		m_debug.Shutdown();
		m_graphics.Shutdown();

		return 0;
	}

	bool update() override
	{
		bool stop = entry::processEvents(
			m_graphics.m_width,
			m_graphics.m_height,
			m_debug.m_debug,
			m_graphics.m_reset,
			&m_ui.m_mouseState
		);
		
		if (!stop)
		{
			m_ui.Update();
			m_graphics.Update();
			m_audio.Update();
			return true;
		}

		return false;
	}
};

} // namespace

ENTRY_IMPLEMENT_MAIN(
	  AppWrapper
	, "App"
	, "GFX and SFX Initialization."
	, ""
	);
