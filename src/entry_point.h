/*
 * Based of of bgfx examples source, provides platform independent
 * entrypoint for graphical applications.
 */
//
//  Created by Mike Gonzales on 8/21/20.
//

#ifndef entry_point_h
#define entry_point_h

#include "presentation_modules.h"
#include "audio_module.h"

class AppWrapper : public entry::AppI
{
	GraphicsSubmodule m_graphics;
	UiSubmodule m_ui;
	DebugSubmodule m_debug;
	AudioSubmodule m_audio;
	
	void StartLogic();
	void UpdateLogic(float dt);

public:

	// inherited from bgapp's entry application wrapper
	AppWrapper(const char* _name, const char* _description, const char* _url);
	void init(int32_t _argc, const char* const* _argv, uint32_t _width, uint32_t _height) override;
	virtual int shutdown() override;
	bool update() override;
};

#endif /* entry_point_h */
