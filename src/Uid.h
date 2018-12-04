#ifndef _UID_H_
#define _UID_H_

#include <LinkedList.hpp>
typedef uint16_t uid_type;
#define UID_LENGTH 16
#define UID_MAX 200

#if UID_LENGTH == 16
#define FNV_PRIME 16777619
#define FNV_OFFSET 2166136261
#define FNV_MASK 0xFFFF
#endif

#if UID_LENGTH == 32
#define FNV_PRIME 16777619
#define FNV_OFFSET 2166136261
#define FNV_MASK 0xFFFFFFFFu
#endif

#if UID_LENGTH == 64
#define FNV_PRIME 1099511628211ull
#define FNV_OFFSET 14695981039346656037ull
#endif

constexpr uint32_t fnv1(uint32_t h, const char* s) {
	return (*s == 0) ?
			h : fnv1((h * FNV_PRIME) ^ static_cast<uint32_t>(*s), s + 1);
}

constexpr uint16_t H(const char* s) {
	//    uint32_t  h = fnv1(FNV_OFFSET, s) ;
	return (fnv1(FNV_OFFSET, s) & FNV_MASK);
}

class Uid {
	static LinkedList<Uid*> _uids;
	uid_type _id;
	const char* _label;
public:
	Uid(const char* label);
	Uid(uid_type id,const char* label);
	uid_type id();
	const char* label();
	static const char* label(uid_type id);
	static Uid* find(uid_type id);
	static Uid* find(const char* label);
	static uid_type hash(const char* label);
	static uint32_t count() { return _uids.count();}
};

extern Uid UID;

#endif
