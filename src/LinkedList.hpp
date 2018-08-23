#ifndef _LINKED_LIST_HPP_
#define _LINKED_LIST_HPP_

#include <functional>

template <typename T> class Node {
  public:
    Node* _next;
    T _data;
    Node<T>(T data) {
        _data = data;
        _next = 0;
    }
    inline Node<T>* next() { return _next; }
	inline void next(Node<T>* n) { _next=n; }
};

template <typename T> class LinkedList {
  public:
    Node<T>* _first = 0;
    //    uid_t _id;
    //    const char *_label;

    Node<T>* first() { return _first; }

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
            //           INFO(" cursor : %X  _next : %X", cursor,
            //           cursor->_next);
            while (cursor) {
                if (cursor->next() == 0) {
                    cursor->_next = new Node<T>(t);
                    break;
                }
                cursor = cursor->next();
            }
        }
    }

    void remove(T t) {
        if (_first->_data == t) {
            Node<T>* todel = _first;
            _first = _first->_next;
            delete todel;
            return;
        }
        //           INFO(" cursor : %X  _next : %X", cursor, cursor->_next);
        Node<T>* cursor = _first;

        while (cursor) {
			if ( cursor->next() && cursor->next().data ==t ) {
				Node<T>* todel = cursor->next();
				cursor->next( cursor->next()->next() );
				delete todel;
			}
            cursor = cursor->next();
            if (cursor->next() == 0) {
                break;
            }
        }
    }

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
