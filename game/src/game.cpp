// --------- Entry Point ---------------
#include "global.h"
#include "menu.h"
#include "instructions.h"
#include "example_layer.h"
#include "engine/core/entry_point.h"
#include "engine/events/key_event.h"

class sandbox_app : public engine::application {
public:
    sandbox_app() {
        push_layer(new menu());
    }

    // Check if the game is currently running
    bool running = false;

    ~sandbox_app() = default;

    void on_event(engine::event& event) override {
        application::on_event(event);

        engine::event_dispatcher dispatcher(event);
        // dispatch event on window X pressed 
        dispatcher.dispatch<engine::key_pressed_event>(BIND_EVENT_FN(sandbox_app::on_key_pressed));
    }

    bool on_key_pressed(engine::key_pressed_event& event) {

        // Checks what keyboard input the user has pressed
        if (event.event_type() == engine::event_type_e::key_pressed) {

            // Quit the application when user presses escape
            if (event.key_code() == engine::key_codes::KEY_ESCAPE) {
                application::exit();
            }

            // If the user presses ENTER it will start the game
            else if (event.key_code() == engine::key_codes::KEY_ENTER && running != true) {

                // Push a new scene to the stack
                push_overlay(new example_layer());

                // Change the bool condition to true
                running = true;
            }

            // If the user presses SPACE it will show keyboard controls
            else if (event.key_code() == engine::key_codes::KEY_SPACE && running != true) {

                // Push a new scene to the stack
                push_overlay(new instructions());

                // Change the bool condition to false
                running = false;
            }

            // Backspace will go back to the original menu
            if (event.key_code() == engine::key_codes::KEY_BACKSPACE && running != true) {

                // Push a new scene to the stack
                push_overlay(new menu());

                // Change the bool condition to false
                running = false;
            }
        }
        return false;
    }
};

engine::application* engine::create_application() {
    return new sandbox_app();
}
