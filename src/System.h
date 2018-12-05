#ifndef SYSTEM_H
#define SYSTEM_H

#include <Akka.h>

class System : public Actor {
    Str str;

  public:
    static const MsgClass EXIT;

    System(va_list args);
    ~System();

    Receive& createReceive();
};

#endif // SYSTEM_H
