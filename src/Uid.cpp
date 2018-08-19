#include <Log.h>
#include <Uid.h>
#include <malloc.h>

LinkedList<Uid*> Uid::_uids;

Uid UID("UID");

Uid::Uid(const char* name) :
		_id(H(name)) {
	if (find(_id) == 0) {
		_label = (const char*) malloc(strlen(name) + 1);
		strcpy((char*) _label, name);
		_uids.add(this);
	}
}

uid_type Uid::id() {
	return _id;
}

uid_type Uid::hash(const char* label) {
	Uid* uid = 0;
	uid = find(label);
	if (uid) {
		return uid->id();
	} else {
		uid = new Uid(label);
		return uid->id();
	}
}

Uid* Uid::find(const char* label) {
	Uid* p = _uids.findFirst(
			[label](Uid* uid) {return (strcmp(uid->_label, label) == 0);});
	return p;
}

Uid* Uid::find(uid_type id) {
	return _uids.findFirst([id](Uid* uid) {return (uid->id() == id);});
}

const char* Uid::label(uid_type id) {
	Uid* p = find(id);
	if (p)
		return p->_label;
	return "UNKNOWN";
}
