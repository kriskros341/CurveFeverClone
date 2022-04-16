#include <iostream>

using std::cout; using std::endl;
template <typename T>
struct Node {
	T value;
	Node* next = 0;
	Node(T v) : value(v) {};
	~Node() {
		cout << "deleted";
	}
};


template <typename T>
struct LinkedListDynamic {
	Node<T*>* head = 0;

	//NOT IMPL
	Node<T*>* tail = 0;
	
	int length = 0;
	// add item to back of the list
	void push_back(T* v) {
		Node<T*>* newVal = new Node<T*>(v);
		length++;
		if (!head)
			head = newVal;
		else {
			Node<T*>* temp = head;
			while (temp->next) {
				temp = temp->next;
			}
			temp->next = newVal;
		}
	}
	// select value of node by index
	T* at(int index) {
		if (!head) {
			std::cout << "The list is empty" << std::endl;
		}
		if (index > length) {
			std::cout << "No such item in list" << std::endl;
		}
		int n = 0;
		Node<T*>* temp = head;
		while (n != index) {
			if (temp->next) {
				temp = temp->next;
			}
			n++;
		}
		if (temp) {
			return temp->value;
		}
		return head->value;
	};
	// select value of node by index
	T* operator [](int index) {
		return at(index);
	}

	// empty the list in memory safe way
	void clear() {
		length = 0;
		if (head == 0) {
			return;
		}
		Node<T*>* temp1 = head;
		Node<T*>* temp2 = 0;
		while (temp1->next) {
			temp2 = temp1;
			temp1 = temp2->next;
			delete temp2->value;
			delete temp2;
		}
		delete temp1->value;
		delete temp1;
		//delete head;
		head = 0;
		//delete temp1;
	}
	~LinkedListDynamic() {
		clear();
	};
};
