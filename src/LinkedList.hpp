#ifndef _LINKED_LIST_HPP_
#define _LINKED_LIST_HPP_

#include <functional>

template<typename T> class Node {
public:
	Node* _next;
	T _data;
	Node<T>(T data) {
		_data = data;
		_next = 0;
	}
	inline Node<T>* next() {
		return _next;
	}
	;
};

template<typename T>
class LinkedList {
public:
	Node<T>* _first;
	//    uid_t _id;
	//    const char *_label;

	Node<T>* first() {
		return _first;
	}

	Node<T>** lastLink() {
		Node<T>* cursor = _first;
		if (_first == 0)
			return &_first;
		while (cursor) {
			if (cursor->_next == 0) {
				return &cursor->next;
			}
			cursor = cursor->next();
		}
		return 0;
	}

	void add(T t) {
		//        INFO(" adding 0x%X _first: %X  ", t, _first);
		if (_first == 0) {
			_first = new Node<T>(t);
		} else {
			Node<T>* cursor = _first;
			//           INFO(" cursor : %X  _next : %X", cursor, cursor->_next);
			while (cursor) {
				if (cursor->next() == 0) {
					cursor->_next = new Node<T>(t);
					break;
				}
				cursor = cursor->next();
			}
		}
	}
	;
	void forEach(std::function<void(T&)> func) {
		for (Node<T>* ptr = _first; ptr; ptr = ptr->next()) {
			func(ptr->_data);
		}
	}

	T findFirst(std::function<bool(T&)> check) {
		for (Node<T>* ptr = _first; ptr; ptr = ptr->next()) {
			if (check(ptr->_data))
				return ptr->_data;
		}
		return 0;
	}
};

#endif
