#ifndef ESP8266_PROJECTS_NIBUTTONS_H
#define ESP8266_PROJECTS_NIBUTTONS_H

#include <SimpleButton.h>
#include <commons.h>

using namespace simplebutton;

typedef std::function<void(void)> CallbackFunction;

class NIButton : public Service {
private:
    int _gpio = 0;
    Button _button;

    CallbackFunction _on_click = nullptr;

    CallbackFunction _on_hold = nullptr;

public:
    explicit NIButton(int gpio) : _gpio(gpio), _button() {}

    void begin() override;

    NIButton *on_click(CallbackFunction _on_pressed);

    NIButton *on_hold(CallbackFunction _on_hold);

    void cycle_routine() override;
};


#endif
