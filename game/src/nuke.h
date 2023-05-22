#pragma once

// Importing modules from headers
#include <engine.h>

class nuke : public engine::game_object {

public:
	nuke(const engine::game_object_properties props);
	~nuke();

	void init();
	void update(glm::vec3 c, float dt);
	bool active() {
		return nuke_active;
	}

	static engine::ref<nuke> create(const engine::game_object_properties& props);

private:
	bool nuke_active; // Indicates if pickup is active
};
