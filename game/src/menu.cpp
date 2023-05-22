#include "menu.h"
#include "platform/opengl/gl_shader.h"

#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/type_ptr.hpp>
#include "engine/events/key_event.h"
#include "engine/utils/track.h"

menu::menu() : m_2d_camera(-1.6f, 1.6f, -0.9f, 0.9f), m_3d_camera((float)engine::application::window().width(), (float)engine::application::window().height()) {

	// Hide the mouse and lock it inside the window
	engine::application::window().hide_mouse_cursor();

	// Initialise audio and play background music
	m_audio_manager = engine::audio_manager::instance();
	m_audio_manager->init();
	m_audio_manager->load_sound("assets/audio/backgroundmusic.mp3", engine::sound_type::track, "music");  // Royalty free music from http://www.nosoapradio.us/
	m_audio_manager->play("music");

	// Initialise the shaders for 2D text
	auto text_shader = engine::renderer::shaders_library()->get("text_2D");

	std::dynamic_pointer_cast<engine::gl_shader>(text_shader)->bind();
	std::dynamic_pointer_cast<engine::gl_shader>(text_shader)->set_uniform("projection", glm::ortho(0.f, (float)engine::application::window().width(), 0.f, (float)engine::application::window().height()));

	m_text_manager = engine::text_manager::create();
}

menu::~menu() {

}

void menu::on_update(const engine::timestep& time_step) {

}

void menu::on_render() {

	// OpenGL RGBA
	int R = 0.0f;
	int G = 0.0f;
	int B = 0.0f;
	int A = 1.0f;

	// Background color for menu
	engine::render_command::clear_color({
		R, G, B, A
		});

	engine::render_command::clear();

	// Creating the rendered to render the menu text
	const auto text_shader = engine::renderer::shaders_library()->get("text_2D");

	// Position the text in the middle of the screen
	m_text_manager->render_text(text_shader, "Press Enter to Start", 500.f, (float)engine::application::window().height() / 2, 0.5f, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	m_text_manager->render_text(text_shader, "Press ESC to Quit", 500.f, (float)engine::application::window().height() / 2 - 50, 0.5f, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	m_text_manager->render_text(text_shader, "Press Space to view Controls", 500.f, (float)engine::application::window().height() / 2 - 100, 0.5f, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
}

void menu::on_event(engine::event& event) {

}
