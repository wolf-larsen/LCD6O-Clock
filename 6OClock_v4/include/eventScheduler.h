#ifndef __EVENTSCHED_H__
#define __EVENTSCHED_H__

#include <Arduino.h>
#include <vector>

class EventScheduler {
  private:
    struct Event {
      void (*callback)();
      unsigned long interval;
      unsigned long lastRun;
      bool active;
      bool oneShot;
    };

    std::vector<Event> events;

  public:
    // Ajouter un événement récurrent
    void addEvent(void (*callback)(), unsigned long interval) {
      events.push_back({callback, interval, millis(), true, false});
    }

    // Ajouter un événement temporisé (une seule exécution)
    void addOneShotEvent(void (*callback)(), unsigned long delay) {
//Serial.printf("one shot recorded at %ld \n",millis());
      events.push_back({callback, delay, millis(), true, true});
    }

    // Supprimer un événement
    bool removeEvent(void (*callback)()) {
      for (auto event = events.begin(); event != events.end(); ++event) {
        if (event->callback == callback) {
          event = events.erase(event);
          return true;
        }
      }
      return false;
    }

    // Mise à jour des événements
    void update() {
//      unsigned long currentMillis = millis();
      for (auto event = events.begin(); event != events.end(); ) {
//        if (event->active && currentMillis - event->lastRun >= event->interval) {
        if (event->active && millis() - event->lastRun >= event->interval) {
          auto eventc = event->callback;
          if (event->oneShot) {
//Serial.printf("one shot trigered at %ld \n",millis());
            event = events.erase(event);
            eventc();
            continue;
          } else {
//            event->lastRun = currentMillis;
            event->lastRun = millis();
            eventc();
          }
        }
        ++event;
      }
    }
};

#endif
