// Import modules
#include "nuke.h"

nuke::nuke(const engine::game_object_properties props) : engine::game_object(props) {

}

nuke::~nuke() {

}

void nuke::init() {
	nuke_active = true;
}

void nuke::update(glm::vec3 c, float dt) {
	set_rotation_amount(rotation_amount() + dt * 1.5f);

	glm::vec3 d = position() - c;

	if (glm::length(d) < 1.0f) {
		nuke_active = false;
	}
}

engine::ref<nuke> nuke::create(const engine::game_object_properties& props) {
	return std::make_shared<nuke>(props);
}
