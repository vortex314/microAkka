#ifndef CONFIGACTOR_H
#define CONFIGACTOR_H
#include <Akka.h>

class ConfigActor {
  public:
    static MsgClass Set;
    ConfigActor();
    ~ConfigActor();
};

#endif // CONFIGACTOR_H
