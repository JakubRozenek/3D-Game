#pragma once
#include <engine.h>

#include "football.h"
#include "enemy.h"
#include "engine/entities/bounding_box.h"
#include "pickup.h"
#include "nuke.h"
#include "juggernog.h"

class example_layer : public engine::layer {

	public:
		example_layer();
		~example_layer();

		void on_update(const engine::timestep& time_step) override;
		void on_render() override; 
		void on_event(engine::event& event) override;

	private:
		void check_bounce();

		float												m_prev_sphere_y_vel = 0.f;
		int													m_score {0};
		int													m_ammunition {4};
		bool												m_out = false;

		engine::ref<engine::skybox>							m_skybox{};

		engine::ref<engine::game_object>					m_terrain{};
		engine::ref<engine::game_object>					m_cow{};
		engine::ref<engine::game_object>					m_tree{};
		engine::ref<engine::game_object>					m_ball{};
		engine::ref<engine::game_object>					m_mannequin{};
		engine::ref<engine::game_object>					m_dumpster{};
		engine::ref<engine::game_object>					m_tank{};
		engine::ref<engine::game_object>					m_mainmenu{};
		engine::ref<engine::game_object>					m_bandit{};

		engine::ref<engine::material>						m_material{};
		engine::ref<engine::material>						m_mannequin_material{};

		engine::DirectionalLight							m_directionalLight;

		std::vector<engine::ref<engine::game_object>>		m_game_objects{};

		engine::ref<engine::bullet_manager>					m_physics_manager{};
		engine::ref<engine::audio_manager>					m_audio_manager{};
		engine::ref<engine::text_manager>					m_text_manager{};

		engine::orthographic_camera							m_2d_camera; 
		engine::perspective_camera							m_3d_camera;

		football											m_football;
		enemy												m_enemy{};
		enemy												m_enemy2{};

		engine::bounding_box								m_cow_box;
		engine::bounding_box								m_cow_box2;
		engine::bounding_box								m_football_box;
		engine::bounding_box								m_dumpster_box;
		engine::bounding_box								m_tank_box;
		engine::bounding_box								m_3d_camera_box;
		engine::bounding_box								m_bandit_box;

		engine::ref<pickup>									m_pickup{};
		engine::ref<nuke>									nuke_pickup{};
		engine::ref<juggernog>								juggernog_pickup{};
};
