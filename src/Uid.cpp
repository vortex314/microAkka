#include <Log.h>
#include <Uid.h>
#include <malloc.h>

list<Uid*> Uid::_uids;

Uid::Uid(uid_type id, const char* label) {
    _label = (const char*)malloc(strlen(label) + 1);
    _id = id;
    strcpy((char*)_label, label);
    _uids.push_back(this);
}

Uid::Uid(const char* label) : _id(H(label)) {
    if (find(_id) == 0) {
        _label = (const char*)malloc(strlen(label) + 1);
        strcpy((char*)_label, label);
        _uids.push_back(this);
    }
}

uid_type Uid::id() { return _id; }

uid_type Uid::hash(const char* label) {
    uid_type id = H(label);
    if (find(id) == 0) {
        new Uid(id, label);
    }
    return id;
}

Uid* Uid::find(const char* label) {
    for (Uid* uid : _uids) {
        if ((strcmp(uid->_label, label) == 0))
            return uid;
    }
    return 0;
}

Uid* Uid::find(uid_type id) {
    for (Uid* uid : _uids) {
        if (uid->id() == id)
            return uid;
    }
    return 0;
}

const char* Uid::label(uid_type id) {
    Uid* p = find(id);
    if (p)
        return p->_label;
    return 0;
}
