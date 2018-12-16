#ifndef SYSTEM_H
#define SYSTEM_H

#include <Akka.h>
#include <Config.h>

class System : public Actor {

  public:
    static const MsgClass Exit;
    static const MsgClass ConfigRequest;
    static const MsgClass ConfigReply;

    System(va_list args);
    ~System();

    Receive& createReceive();
};

#endif // SYSTEM_H
