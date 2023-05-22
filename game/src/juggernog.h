#pragma once

// Importing modules from headers
#include <engine.h>

class juggernog : public engine::game_object {

	public:
		juggernog(const engine::game_object_properties props);
		~juggernog();

		void init();
		void update(glm::vec3 c, float dt);
		bool active() {
			return juggernog_active;
		}

		static engine::ref<juggernog> create(const engine::game_object_properties& props);

	private:
		bool juggernog_active; // Indicates if pickup is active
};
