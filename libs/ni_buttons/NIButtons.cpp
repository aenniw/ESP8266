#include "NIButtons.h"

void NIButton::begin() {
    _button.setup(_gpio, false);
}

NIButton *NIButton::on_click(CallbackFunction _on_pressed) {
    _button.setOnClicked(_on_pressed);
    return this;
}

NIButton *NIButton::on_hold(CallbackFunction _on_hold) {
    _button.setOnHolding(_on_hold);
    return this;
}

void NIButton::cycle_routine() {
    _button.update();
}