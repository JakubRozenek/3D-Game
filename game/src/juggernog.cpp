// Import modules
#include "juggernog.h"

juggernog::juggernog(const engine::game_object_properties props) : engine::game_object(props) {

}

juggernog::~juggernog() {

}

void juggernog::init() {
	juggernog_active = true;
}

void juggernog::update(glm::vec3 c, float dt) {
	set_rotation_amount(rotation_amount() + dt * 1.5f);

	glm::vec3 d = position() - c;

	if (glm::length(d) < 1.0f) {
		juggernog_active = false;
	}
}

engine::ref<juggernog> juggernog::create(const engine::game_object_properties& props) {
	return std::make_shared<juggernog>(props);
}
