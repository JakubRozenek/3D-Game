#pragma once

// Importing the engine
#include <engine.h>

class menu : public engine::layer {
public:
	menu();
	~menu();

	void on_update(const engine::timestep& time_step) override;
	void on_render() override;
	void on_event(engine::event& event) override;

private:

	// Rendering of audio
	engine::ref<engine::audio_manager>  m_audio_manager{};

	// Rendering text on the screen
	engine::ref<engine::text_manager>	m_text_manager{};

	// Rendering of different camera views
	engine::orthographic_camera			m_2d_camera;
	engine::perspective_camera			m_3d_camera;
};
