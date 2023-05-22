#include "example_layer.h"
#include "platform/opengl/gl_shader.h"
#include <random>

#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/type_ptr.hpp>
#include "engine/events/key_event.h"
#include "engine/utils/track.h"

#include "juggernog.h"
#include "pickup.h"
#include "nuke.h"

example_layer::example_layer():m_2d_camera(-1.6f, 1.6f, -0.9f, 0.9f), m_3d_camera((float)engine::application::window().width(), (float)engine::application::window().height()) {

	// Hide the mouse and lock it inside the window
    engine::application::window().hide_mouse_cursor();

	// Initialise audio and play background music
	m_audio_manager = engine::audio_manager::instance();
	m_audio_manager->init();
	m_audio_manager->load_sound("assets/audio/bounce.wav",					engine::sound_type::spatialised, "bounce");		// Royalty free sound from freesound.org
	m_audio_manager->load_sound("assets/audio/backgroundmusic.mp3",			engine::sound_type::track, "music");			// https://www.chosic.com/download-audio/25499/
	m_audio_manager->load_sound("assets/audio/gunshot.mp3",					engine::sound_type::event, "shoot");
	m_audio_manager->load_sound("assets/audio/powerup.wav",					engine::sound_type::event, "powerup");			// https://freesound.org/people/GameAudio/sounds/220173/
	m_audio_manager->load_sound("assets/audio/destroyed.mp3",				engine::sound_type::event, "destroyed");		// https://pixabay.com/sound-effects/explosion-sound-effect-4-10999/
	m_audio_manager->load_sound("assets/audio/empty.mp3",					engine::sound_type::event, "empty");			// https://pixabay.com/sound-effects/rifle-clip-empty-98832/
	m_audio_manager->play("music");

	// Initialise the shaders, materials and lights
	auto mesh_shader = engine::renderer::shaders_library()->get("mesh");
	auto text_shader = engine::renderer::shaders_library()->get("text_2D");

	m_directionalLight.Color = glm::vec3(1.0f, 1.0f, 1.0f);
	m_directionalLight.AmbientIntensity = 0.25f;
	m_directionalLight.DiffuseIntensity = 0.6f;
	m_directionalLight.Direction = glm::normalize(glm::vec3(-0.5f, -1.0f, -0.5f));

	// set color texture unit
	std::dynamic_pointer_cast<engine::gl_shader>(mesh_shader)->bind();
	std::dynamic_pointer_cast<engine::gl_shader>(mesh_shader)->set_uniform("lighting_on", true);
	std::dynamic_pointer_cast<engine::gl_shader>(mesh_shader)->set_uniform("gColorMap", 0);

	m_directionalLight.submit(mesh_shader);

	std::dynamic_pointer_cast<engine::gl_shader>(mesh_shader)->set_uniform("gMatSpecularIntensity", 1.f);
	std::dynamic_pointer_cast<engine::gl_shader>(mesh_shader)->set_uniform("gSpecularPower", 10.f);
	std::dynamic_pointer_cast<engine::gl_shader>(mesh_shader)->set_uniform("transparency", 1.0f);

	std::dynamic_pointer_cast<engine::gl_shader>(text_shader)->bind();
	std::dynamic_pointer_cast<engine::gl_shader>(text_shader)->set_uniform("projection", glm::ortho(0.f, (float)engine::application::window().width(), 0.f, (float)engine::application::window().height()));

	m_material = engine::material::create(1.0f, glm::vec3(1.0f, 0.1f, 0.07f), glm::vec3(1.0f, 0.1f, 0.07f), glm::vec3(0.5f, 0.5f, 0.5f), 1.0f);
	m_mannequin_material = engine::material::create(1.0f, glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.5f, 0.5f, 0.5f), 1.0f);

	bool game = false;

	// Skybox Textrue from https://www.subpng.com/png-drbqtr/
	m_skybox = engine::skybox::create(50.f, {
		engine::texture_2d::create("assets/textures/skybox/front.bmp", true),	// Front
		engine::texture_2d::create("assets/textures/skybox/right.bmp", true),	// Right
		engine::texture_2d::create("assets/textures/skybox/back.bmp", true),	// Back
		engine::texture_2d::create("assets/textures/skybox/left.bmp", true),	// Left
		engine::texture_2d::create("assets/textures/skybox/top.bmp", true),		// Top
		engine::texture_2d::create("assets/textures/skybox/bottom.bmp", true)	// Bottom
	});

	engine::ref<engine::skinned_mesh> m_skinned_mesh = engine::skinned_mesh::create("assets/models/animated/mannequin/free3Dmodel.dae");
	m_skinned_mesh->LoadAnimationFile("assets/models/animated/mannequin/walking.dae");
	m_skinned_mesh->LoadAnimationFile("assets/models/animated/mannequin/idle.dae");
	m_skinned_mesh->LoadAnimationFile("assets/models/animated/mannequin/jump.dae");
	m_skinned_mesh->LoadAnimationFile("assets/models/animated/mannequin/standard_run.dae");
	m_skinned_mesh->switch_root_movement(false);

	engine::game_object_properties mannequin_props;
	mannequin_props.animated_mesh = m_skinned_mesh;
	mannequin_props.scale = glm::vec3(1.f/ glm::max(m_skinned_mesh->size().x, glm::max(m_skinned_mesh->size().y, m_skinned_mesh->size().z)));
	mannequin_props.position = glm::vec3(11.8f, 0.5f, 24.8f);
	mannequin_props.type = 0;
	mannequin_props.bounding_shape = m_skinned_mesh->size() / 2.f * mannequin_props.scale.x;
	m_mannequin = engine::game_object::create(mannequin_props);

	// Main menu background https://www.moddb.com/games/spring-breeze/images/main-menu-background
	std::vector<engine::ref<engine::texture_2d>> intro_texture = { engine::texture_2d::create("assets/screens/mainmenu.png", false) };
	engine::ref<engine::terrain> intro_shape = engine::terrain::create(10.f, 0.5f, 6.f);
	engine::game_object_properties intro_props;
	intro_props.position = glm::vec3(0.f, -20.f, 0.f);
	intro_props.meshes = { intro_shape->mesh() };
	intro_props.textures = intro_texture;
	intro_props.is_static = true;
	intro_props.bounding_shape = glm::vec3(10.f, 0.5f, 6.f);
	intro_props.rotation_amount = glm::half_pi<float>();
	intro_props.rotation_axis = glm::vec3(1, 0, 0);
	m_mainmenu = engine::game_object::create(intro_props);

	// Load the tank model. Create a tank object. Set its properties https://www.turbosquid.com/de/3d-models/free-3ds-mode-wiesel-2-ozelot-anti-air/361920
	engine::ref <engine::model> tank_model = engine::model::create("assets/models/static/Ozelot.obj");
	engine::game_object_properties tank_props;
	tank_props.meshes = tank_model->meshes();
	tank_props.textures = tank_model->textures();
	float tank_scale = 0.0125f;
	tank_props.position = { 26.9f, 0.5f, 8.0f };
	tank_props.scale = glm::vec3(tank_scale);
	tank_props.bounding_shape = tank_model->size() / 2.0f * tank_scale;
	m_tank = engine::game_object::create(tank_props);

	// Create the tank collision box
	m_tank_box.set_box(tank_props.bounding_shape.x * 2, tank_props.bounding_shape.y * 1.1, tank_props.bounding_shape.z * 2, tank_props.position);

	// Load the dumpster model. Create a dumpster object. Set its properties https://www.cgtrader.com/free-3d-models/industrial/other/20ft-shipping-container-a4206e2a-46b9-4f1a-81d9-9b5d3e8f74e1
	engine::ref <engine::model> dumpster_model = engine::model::create("assets/models/static/container.obj");
	engine::game_object_properties dumpster_props;
	dumpster_props.meshes = dumpster_model->meshes();
	dumpster_props.textures = dumpster_model->textures();
	float dumpster_scale = 1.0f;
	dumpster_props.position = {34.8, 0.5, 8.f}; // -7.f, 0.5f, 15.f
	dumpster_props.scale = glm::vec3(dumpster_scale);
	dumpster_props.bounding_shape = dumpster_model->size() / 2.0f * dumpster_scale;
	m_dumpster = engine::game_object::create(dumpster_props);

	// Create the dumpster collision box
	m_dumpster_box.set_box(dumpster_props.bounding_shape.x * 2, dumpster_props.bounding_shape.y * 2, dumpster_props.bounding_shape.z * 2, dumpster_props.position);

	// Medkit texture from https://www.textures.com/download/manmadeboxes0007/105116
	engine::ref<engine::cuboid> pickup_shape = engine::cuboid::create(glm::vec3(0.5f), false);
	engine::ref<engine::texture_2d> pickup_texture = engine::texture_2d::create("assets/textures/medkit.jpg", true);
	engine::game_object_properties pickup_props;
	pickup_props.position = { 9.f, 1.0f, 10.f };
	pickup_props.meshes = { pickup_shape->mesh() };
	pickup_props.textures = { pickup_texture };
	m_pickup = pickup::create(pickup_props);
	m_pickup->init();

	// Nuke texture from https://www.pngitem.com/middle/JhRhmR_tactical-nuke-addon-moab-png-transparent-png/
	engine::ref<engine::cuboid> nuke_shape = engine::cuboid::create(glm::vec3(0.50f), false);
	engine::ref<engine::texture_2d> nuke_texture = engine::texture_2d::create("assets/textures/nuke.jpg", true);
	engine::game_object_properties nuke_props;
	nuke_props.position = {16.f, 1.0f, 16.0f };
	nuke_props.meshes = { nuke_shape->mesh() };
	nuke_props.textures = { nuke_texture };
	nuke_pickup = nuke::create(nuke_props);
	nuke_pickup->init();

	// Edited Juggernog texture from https://www.deviantart.com/assyrianic/art/Juggernog-211870908
	engine::ref<engine::cuboid> juggernog_shape = engine::cuboid::create(glm::vec3(0.50f), false);
	engine::ref<engine::texture_2d> juggernog_texture = engine::texture_2d::create("assets/textures/juggernog.jpg", true);
	engine::game_object_properties juggernog_props;
	juggernog_props.position = {16.f, 1.0f, 6.f };
	juggernog_props.meshes = { juggernog_shape->mesh() };
	juggernog_props.textures = { juggernog_texture };
	juggernog_pickup = juggernog::create(juggernog_props);
	juggernog_pickup->init();

	// Load the terrain texture and create a terrain mesh. Create a terrain object. Set its properties
	std::vector<engine::ref<engine::texture_2d>> terrain_textures = { engine::texture_2d::create("assets/textures/rocky-trail.jpg", false) }; // https://polyhaven.com/a/rocky_trail
	engine::ref<engine::terrain> terrain_shape = engine::terrain::create(100.f, 0.5f, 100.f);
	engine::game_object_properties terrain_props;
	terrain_props.meshes = { terrain_shape->mesh() };
	terrain_props.textures = terrain_textures;
	terrain_props.is_static = true;
	terrain_props.type = 0;
	terrain_props.bounding_shape = glm::vec3(100.f, 0.5f, 100.f);
	terrain_props.restitution = 0.92f;
	m_terrain = engine::game_object::create(terrain_props);

	// Load the cow model. Create a cow object. Set its properties
	engine::ref <engine::model> cow_model = engine::model::create("assets/models/static/cow4.3ds");
	engine::game_object_properties cow_props;
	cow_props.meshes = cow_model->meshes();
	cow_props.textures = cow_model->textures();
	float cow_scale = 1.f / glm::max(cow_model->size().x, glm::max(cow_model->size().y, cow_model->size().z));
	cow_props.position = {10.f, 0.5f, 2.5f};
	cow_props.scale = glm::vec3(cow_scale);
	cow_props.bounding_shape = cow_model->size();
	m_cow = engine::game_object::create(cow_props);

	// Create the cow collision box
	m_cow_box.set_box(cow_props.bounding_shape.x * cow_scale, cow_props.bounding_shape.y * cow_scale, cow_props.bounding_shape.z * cow_scale, cow_props.position);

	// Load the tree model. Create a tree object. Set its properties
	engine::ref <engine::model> tree_model = engine::model::create("assets/models/static/elm.3ds");
	engine::game_object_properties tree_props;
	tree_props.meshes = tree_model->meshes();
	tree_props.textures = tree_model->textures();
	float tree_scale = 3.f / glm::max(tree_model->size().x, glm::max(tree_model->size().y, tree_model->size().z));
	tree_props.position = { 4.f, 0.5f, -5.f };
	tree_props.bounding_shape = tree_model->size() / 2.f * tree_scale;
	tree_props.scale = glm::vec3(tree_scale);
	m_tree = engine::game_object::create(tree_props);

	// Load the bullet model. Create a bullet object. Set its properties
	engine::ref <engine::model> sphere_shape = engine::model::create("assets/models/static/newSoccerBall.3ds");
	engine::game_object_properties sphere_props;
	sphere_props.position = { -100.0f, -100.0f, -100.0f };
	sphere_props.meshes = sphere_shape->meshes();
	sphere_props.textures = sphere_shape->textures();
	sphere_props.type = 1;
	float sphere_scale = 1.f / glm::max(sphere_shape->size().x, glm::max(sphere_shape->size().y, sphere_shape->size().z));
	sphere_props.bounding_shape = sphere_shape->size() / 2.0f;
	sphere_props.restitution = 0.92f;
	sphere_props.scale = glm::vec3(sphere_scale);
	sphere_props.mass = 0.45f;
	sphere_props.rolling_friction = 0.1f;
	m_ball = engine::game_object::create(sphere_props);

	// Create the bullet collision box
	m_football_box.set_box(sphere_props.bounding_shape.x * 2, sphere_props.bounding_shape.y * -0.75, sphere_props.bounding_shape.z * 2, sphere_props.position);

	m_game_objects.push_back(m_terrain);
	m_game_objects.push_back(m_ball);
	m_game_objects.push_back(m_mainmenu);

	m_physics_manager = engine::bullet_manager::create(m_game_objects);
	m_text_manager = engine::text_manager::create();

	m_skinned_mesh->switch_animation(1);

	// Initialize the bullet object
	m_football.initialise(m_ball);

	// Initalize the enemy object
	m_enemy.initialise(m_cow, cow_props.position, glm::vec3(1.f, 0.f, 0.f));
	m_enemy2.initialise(m_tank, tank_props.position, glm::vec3(1.f, 0.f, 0.f));
}

example_layer::~example_layer() {
	
}

void example_layer::on_update(const engine::timestep& time_step) {

	// Setting up the 3d camera
	m_3d_camera.on_update(time_step);
	m_football.on_update(time_step);

	// When the object moves update it's hitbox
	m_football_box.on_update(m_football.object()->position() - glm::vec3(0.f, m_football.object()->offset().y, 0.f) * m_football.object()->scale());

	// Creating the cow
	m_cow_box.on_update(m_cow->position() - glm::vec3(0.f, m_cow->offset().y, 0.f) * m_cow->scale());
	m_cow_box.on_update(m_enemy.object()->position());

	// Creating the tank
	m_tank_box.on_update(m_tank->position() - glm::vec3(0.f, m_tank->offset().y, 0.f) * m_tank->scale());
	m_tank_box.on_update(m_enemy2.object()->position());

	m_dumpster_box.on_update(m_dumpster->position() - glm::vec3(0.f, m_dumpster->offset().y, 0.f) * m_dumpster->scale());

	// Create the enemy instance
	m_enemy.on_update(time_step, m_3d_camera.position());
	m_enemy2.on_update(time_step, m_3d_camera.position());

	// Make the powerups appear
	m_pickup->update(m_3d_camera.position(), time_step);
	nuke_pickup->update(m_3d_camera.position(), time_step);
	juggernog_pickup->update(m_3d_camera.position(), time_step);

	// Check if the bullet has hit the tank
	if (m_tank_box.collision(m_football_box)) {

		// Increase the player's score
		m_score += 5;

		// Make the tank disappear
		m_tank->set_position(glm::vec3(-9999.f, -9999.f, -9999.f));

		m_audio_manager->play("destroyed");
	}

	// Check if the bullet has hit the cow
	else if (m_cow_box.collision(m_football_box)) {

		// Increase the player's score
		m_score += 1;

		// Make the cow disappear
		m_cow->set_position(glm::vec3(-9999.f, -9999.f, -9999.f));

		m_audio_manager->play("destroyed");
	}

	m_physics_manager->dynamics_world_update(m_game_objects, double(time_step));
	m_audio_manager->update_with_camera(m_3d_camera);

	m_mannequin->animated_mesh()->on_update(time_step);

	check_bounce();
} 

void example_layer::on_render() {

	engine::render_command::clear_color({ 0.2f, 0.3f, 0.3f, 1.0f });
	engine::render_command::clear();

	// Set up  shader. (renders textures and materials)
	const auto mesh_shader = engine::renderer::shaders_library()->get("mesh");
	engine::renderer::begin_scene(m_3d_camera, mesh_shader);

	// Rendering the collisions of objects
	m_cow_box.on_render(1.0f, 1.0f, 1.0f, mesh_shader);
	m_football_box.on_render(1.0f, 1.0f, 1.0f, mesh_shader);
	m_dumpster_box.on_render(1.0f, 1.0f, 1.0f, mesh_shader);
	m_tank_box.on_render(1.0f, 1.0f, 1.0f, mesh_shader);

	// Set up some of the scene's parameters in the shader
	std::dynamic_pointer_cast<engine::gl_shader>(mesh_shader)->set_uniform("gEyeWorldPos", m_3d_camera.position());

	// Position the skybox centred on the player and render it
	glm::mat4 skybox_tranform(1.0f);
	skybox_tranform = glm::translate(skybox_tranform, m_3d_camera.position());

	for (const auto& texture : m_skybox->textures()) {
		texture->bind();
	}

	engine::renderer::submit(mesh_shader, m_mainmenu);

	// Render the terrain and skybox
	engine::renderer::submit(mesh_shader, m_skybox, skybox_tranform);
	engine::renderer::submit(mesh_shader, m_terrain);

	// Render the dumpster and tank
	engine::renderer::submit(mesh_shader, m_dumpster);
	engine::renderer::submit(mesh_shader, m_tank);

	// Render the cow
	glm::mat4 cow_transform(1.0f);
	cow_transform = glm::translate(cow_transform, m_cow->position());
	cow_transform = glm::rotate(cow_transform, m_cow->rotation_amount(), m_cow->rotation_axis());
	cow_transform = glm::translate(cow_transform, -m_cow->offset() * m_cow->scale());
	cow_transform = glm::scale(cow_transform, m_cow->scale());
	engine::renderer::submit(mesh_shader, cow_transform, m_cow);

	m_football.on_render(mesh_shader);
	m_material->submit(mesh_shader);

	// Loop in order to render n amount of trees
	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 10; j++) {

			// Render the tree
			glm::mat4 tree_transform(1.0f);
			tree_transform = glm::translate(tree_transform, glm::vec3(4.0f + j * 2.0f, 0.5, -5.0f + i * 3.0f));
			tree_transform = glm::rotate(tree_transform, m_tree->rotation_amount(), m_tree->rotation_axis());
			tree_transform = glm::scale(tree_transform, m_tree->scale());
			engine::renderer::submit(mesh_shader, tree_transform, m_tree);
		}
	}

	// Check if the pickup item has been picked up
	if (m_pickup->active()) {
		std::dynamic_pointer_cast<engine::gl_shader>(mesh_shader)->set_uniform("has_texture", true);
		m_pickup->textures().at(0)->bind();
		glm::mat4 pickup_transform(1.0f);
		pickup_transform = glm::translate(pickup_transform, m_pickup->position());
		pickup_transform = glm::rotate(pickup_transform, m_pickup->rotation_amount(), m_pickup->rotation_axis());
		engine::renderer::submit(mesh_shader, m_pickup->meshes().at(0), pickup_transform);
		std::dynamic_pointer_cast<engine::gl_shader>(mesh_shader)->set_uniform("has_texture", false);
	}
	else {
		m_audio_manager->play("powerup");
	}

	// Check if the nuke pickup item has been picked up
	if (nuke_pickup->active()) {
		std::dynamic_pointer_cast<engine::gl_shader>(mesh_shader)->set_uniform("has_texture", true);
		nuke_pickup->textures().at(0)->bind();
		glm::mat4 pickup_transform(1.0f);
		pickup_transform = glm::translate(pickup_transform, nuke_pickup->position());
		pickup_transform = glm::rotate(pickup_transform, nuke_pickup->rotation_amount(), nuke_pickup->rotation_axis());
		engine::renderer::submit(mesh_shader, nuke_pickup->meshes().at(0), pickup_transform);
		std::dynamic_pointer_cast<engine::gl_shader>(mesh_shader)->set_uniform("has_texture", false);
	}
	else {
		m_audio_manager->play("powerup");
	}

	// Check if the juggernog pickup item has been picked up
	if (juggernog_pickup->active()) {
		std::dynamic_pointer_cast<engine::gl_shader>(mesh_shader)->set_uniform("has_texture", true);
		juggernog_pickup->textures().at(0)->bind();
		glm::mat4 pickup_transform(1.0f);
		pickup_transform = glm::translate(pickup_transform, juggernog_pickup->position());
		pickup_transform = glm::rotate(pickup_transform, juggernog_pickup->rotation_amount(), juggernog_pickup->rotation_axis());
		engine::renderer::submit(mesh_shader, juggernog_pickup->meshes().at(0), pickup_transform);
		std::dynamic_pointer_cast<engine::gl_shader>(mesh_shader)->set_uniform("has_texture", false);
	}
	else {
		m_audio_manager->play("powerup");
	}

	engine::renderer::end_scene();

	// Render text
	const auto text_shader = engine::renderer::shaders_library()->get("text_2D");

	// Creating the crosshair
	m_text_manager->render_text(text_shader, ".", (float)engine::application::window().width() / 2, (float)engine::application::window().height() / 2, 0.5f, glm::vec4(1.f, 0.5f, 0.f, 1.f));

	// Creating a timer
	std::clock_t duration;
	duration = std::clock() / (float) CLOCKS_PER_SEC;

	// Adding the timer to the screen
	m_text_manager->render_text(text_shader, "Timer: " + std::to_string(duration) + "s", (float)engine::application::window().width() / 2.175f, (float)engine::application::window().width() / 1.85f, 0.5f, glm::vec4(1.f, 0.5f, 0.f, 1.f));

	// Adding the score to the HUD
	m_text_manager->render_text(text_shader, "Score: " + std::to_string(m_score), 10.f, (float)engine::application::window().height() - 25.f, 0.5f, glm::vec4(1.f, 0.5f, 0.f, 1.f));

	// Adding the ammo count to the HUD
	m_text_manager->render_text(text_shader, "Ammo: " + std::to_string(m_ammunition), 10.f, (float)engine::application::window().height() - 50.f, 0.5f, glm::vec4(1.f, 0.5f, 0.f, 1.f));
	
	// Check if the player has any bullets in the magazine
	if (m_out) {

		// Show warning you have no ammo
		m_text_manager->render_text(text_shader, "Out of Ammo", (float)engine::application::window().width() / 2.225, (float)engine::application::window().height() / 1.90f, 0.5f, glm::vec4(1.f, 0.f, 0.f, 1.f));
	}
}

void example_layer::on_event(engine::event& event) { 
    if(event.event_type() == engine::event_type_e::key_pressed) { 
        auto& e = dynamic_cast<engine::key_pressed_event&>(event); 

		if(e.key_code() == engine::key_codes::KEY_TAB) { 
            engine::render_command::toggle_wireframe();
        }

		// Check if the magazine has any bullets
		if (m_ammunition > 0) {

			// Key to allow the player to shoot
			if (e.key_code() == engine::key_codes::KEY_SPACE) {

				// Play the shooting noise
				m_audio_manager->play("shoot");

				// Remove a bullet from the magazine
				m_ammunition -= 1;

				// Shoot the bullet
				m_football.kick(m_3d_camera, 4000.f);
			}
		}

		// If the magazine has no bullets
		else if (m_ammunition <= 0) {

			// Tell the boolean you have no bullets
			m_out = true;

			// Play out of ammo sound
			m_audio_manager->play("empty");
		}
    } 
}

void example_layer::check_bounce() {
	if (m_prev_sphere_y_vel < 0.1f && m_ball->velocity().y > 0.1f) {
		m_audio_manager->play_spatialised_sound("bounce", m_3d_camera.position(), glm::vec3(m_ball->position().x, 0.f, m_ball->position().z));
	}

	m_prev_sphere_y_vel = m_game_objects.at(1)->velocity().y;
}
