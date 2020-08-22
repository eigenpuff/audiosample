
/*
 * Based of of bgfx examples source, provides platform independent
 * entrypoint for graphical applications.
 */
#include "entry_point.h"

AppWrapper::AppWrapper(const char* _name, const char* _description, const char* _url)
	: entry::AppI(_name, _description, _url)
{
}

void AppWrapper::init(int32_t _argc, const char* const* _argv, uint32_t _width, uint32_t _height) 
{
	Args args(_argc, _argv);

	m_graphics.Init(args.m_type, args.m_pciId, _width, _height, BGFX_RESET_VSYNC);
	m_debug.Init(BGFX_DEBUG_TEXT);
	m_ui.Init(_width, _height);
	
	m_audio.Init();
	StartLogic();
}

int AppWrapper::shutdown()
{
	m_audio.Shutdown();
	m_ui.Shutdown();
	m_debug.Shutdown();
	m_graphics.Shutdown();

	return 0;
}

bool AppWrapper::update()
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
		UpdateLogic(1.0f/60.0f);
	
		m_ui.Update();
		m_graphics.Update();
		m_audio.Update();
		return true;
	}

	return false;
}



ENTRY_IMPLEMENT_MAIN(
	  AppWrapper
	, "App"
	, "GFX and SFX Initialization."
	, ""
	);
