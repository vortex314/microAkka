
#include <Log.h>
#include <Uid.h>
#include <malloc.h>

// LinkedList<UidEntry*> Uid::list;
Uid UID;

Uid::Uid() {}

UidEntry::UidEntry(const char* name) : _id(H(name)) {
    DEBUG(" new : label %s:0x%X\n", name, name);
    _label = (const char*)malloc(strlen(name) + 1);
    strcpy((char*)_label, name);
}

uid_type Uid::get(const char* label) {
    UidEntry* uid = 0;
    uid = find(label);
    if (uid) {
        return uid->id();
    } else {
        uid = new UidEntry(label);
        add(uid);
        return uid->id();
    }
}

UidEntry* Uid::find(const char* label) {
    UidEntry* p = findFirst(
        [label](UidEntry* uid) { return (strcmp(uid->_label, label) == 0); });
    return p;
}

uid_type Uid::id(const char* label) {
    UidEntry* p = find(label);
    if (p)
        return p->id();
    return 0;
}

UidEntry* Uid::find(uid_type id) {
    UidEntry* p = findFirst([id](UidEntry* uid) { return (uid->id() == id); });
    return p;
}

const char* Uid::label(uid_type id) {
    UidEntry* p = find(id);
    if (p)
        return p->_label;
    return "UNKNOWN";
}
