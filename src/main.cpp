#include "../.pio/libdeps/esp32doit-devkit-v1/LCDIC2/LCDIC2.h"
#include <cstdlib>
#include "../.pio/libdeps/esp32doit-devkit-v1/BlueFairy/src/bluefairy.h"


#include "../.pio/libdeps/esp32doit-devkit-v1/EasyStringStream/src/EasyStringStream.h"

#include "../.pio/libdeps/esp32doit-devkit-v1/LinkedList/LinkedList.h"

#include <Arduino.h>

#include <utility>

//-----------------------------------------------------------
//-------------------Configuration section-------------------
//-----------------------------------------------------------


int listenDelay = 10;

//END OF CONFIGURATION
void onClick(int id) {
    switch (id) {
        case 3:
            break;
    }


    Serial.println("Button clicked with id of: " + String(id));
}

void onRelease(int id) {
    Serial.println(id);
}

//-----------------------------------------------------------
//----------------------Internal Library---------------------
//-----------------------------------------------------------
String repeatChar(char ch, int amount) {
    char repeated[amount + 1];  //The plus one is due to a bug in the lib used.
    EasyStringStream ss(repeated, amount + 1);
    for (int x = amount; x > 0; x--) {
        ss << ch;
    }
    return repeated;
}

LinkedList<int> u = LinkedList<int>();
LinkedList<int> p = LinkedList<int>();
bluefairy::Scheduler scheduler;

void startListener() {
    scheduler.every(listenDelay, []() {
        for (int index = 0; index < u.size(); index++) {
            int pin = u.get(index);
            if (!digitalRead(pin)) {
                p.add(pin);
                u.remove(index);
                onClick(pin);
            }
        }
        for (int index = 0; index < p.size(); index++) {
            int pin = p.get(index);
            if (digitalRead(pin)) {
                u.add(pin);
                p.remove(index);
                onRelease(pin);
            }
        }
    });
}

void listenTo(int id) {
    pinMode(id, INPUT_PULLUP);
    u.add(id);

}


class LCD {
public:
    int lines;
    int chars;
    int address;

    LCDIC2 *display;
    bool scrollDirection = false;
    int scrollDelay = 2000;

    LCD(int l, int c, int a) {
        display = new LCDIC2(a, c, l);
        lines = l;
        chars = c;
        address = a;
        display->begin();
        clearLcd();
    }

    void writeAt(String str, int x, int y) const {
        display->setCursor(x, y);
        display->print(std::move(str));
    }


    void writeCenter(const String &str, int y) const {
        if (str.length() >= (chars - 1)) {
            writeAt(str, 0, y);
        }
        writeAt(str, ((chars - str.length()) / 2), y);
    }

    String writeCenter(String str, int y, char filler) {
        if (str.length() >= (chars - 1)) {
            return str;
        }
        String fill = repeatChar(filler, ((chars - str.length()) / 2));
        Serial.println(((chars - str.length()) / 2));
        Serial.println(fill);
        writeAt(fill + str + fill, 0, y);
    }

    bool toggled = false;
    bluefairy::TaskNode *scrollTask;

    void autoScroll(bool enabled) {
        if ((enabled && toggled) || (!enabled && !toggled)) {
            return;
        }
        if (toggled) {
            scheduler.removeTask(scrollTask);
            toggled = false;
        } else {
            toggled = true;
            if (scrollDirection) {
                scrollTask = scheduler.every(scrollDelay, [this]() {
                    display->moveRight();
                });
            } else {
                scrollTask = scheduler.every(scrollDelay, [this]() {
                    display->moveLeft();
                });
            }
        }
    }

    void clearLcd() const {
        display->clear();
        display->home();
        display->setCursor(false);
        display->setShift(false);
        display->setCursor(0, 0);
    }

    ~LCD() {
        delete display;
    }
};

class MenuItem {
public:
    String group;
    std::function<void()> action;

    MenuItem() = default;

    MenuItem(String group, std::function<void()> action) {
        this->group = std::move(group);
        this->action = action;
    }

    virtual void actions() = 0;
};
class Group{
    LinkedList<String> list;
};
class Menu {
};


void setup() {
    Serial.begin(115200);
    LCD screen = LCD(2, 16, 0x27);

    screen.writeCenter("Welcome here!", 1);
}

void loop() {
    scheduler.loop();
}
