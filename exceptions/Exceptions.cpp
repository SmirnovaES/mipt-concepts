#include "stdafx.h"
#include <csetjmp>
#include <exception>
#include <iostream>

#define BAD 1
#define VERYBAD 2
#define PLSSTOP 3


#define TRY \
	stack_elem next_se; \
	int exc = (int) setjmp(next_se.buf); \
	num_of_exceptions = 1; \
	num_of_catches = 0; \
	if (exc == 0)

#define CATCH(err)  \
	else if (err == exc && ++num_of_catches)

#define THROW(exc_)  \
	std::cout << "num of exc " << num_of_exceptions << std::endl; \
	if (num_of_exceptions == 0) { \
		std::cerr << "THROW in THROW. Terminating...\n"; \
		std::terminate(); \
	} \
	num_of_exceptions = 0; \
	untwist_stack(exc_);

#define FINALLY \
	if (num_of_catches == 0) { \
		std::cerr << "No such exception in CATCH blocks. Terminating...\n"; \
		std::terminate(); \
	}

static int num_of_exceptions = 0; //для проверки, что не выкидывается исключение во время раскрутки стека
static int num_of_catches = 0;

class object {
public:
	object();
	virtual ~object();

	object* prev; //предыдущий вызванный элемент
};

class stack_elem {
public:
	stack_elem();

	~stack_elem();

	jmp_buf buf; //непосредственно буфер, необходимый для setjmp/longjmp
	stack_elem* prev; //указатель на предыдущий элемент стека
	object* obj; //указатель на объект, соответствующий данной ячейке стека
};

static stack_elem* stack_root = NULL; //вершина стека

stack_elem::stack_elem() {
	obj = NULL;
	prev = stack_root;
	stack_root = this;
}

stack_elem::~stack_elem() {
	stack_root = prev; //меняем вершину
}


object::object() {//при создании объекта кладем его на стек
	if (stack_root != NULL) { //можем создать объект не в TRY/CATCH, и тогда корень еще не будет определен и делать ничего не надо
		prev = stack_root->obj;
		stack_root->obj = this;
	}
}

object::~object() {
	if (stack_root != NULL && prev != NULL) {
		stack_root->obj = prev;
		prev = NULL;
	}
}

void untwist_stack(int code) {
	if (code == 0) {
		return;
	}
	stack_elem* curr_se = stack_root;
	object* curr_obj = stack_root->obj;
	while (curr_obj != NULL) { //последовательно вызываем деструкторы объектов
		object* tmp_obj = curr_obj;
		curr_obj = curr_obj->prev;
		tmp_obj->~object();
	}
	if (curr_se != NULL) { //меняем вершину стека
		stack_root = curr_se->prev;
	}
	longjmp(curr_se->buf, code); //откатываемся
	return;
}

//Тестовые классы просто выводят на экран, когда они вызывают констуктор и деструктор
class A: public object {
public:
	A(int i) : val(i) {
		std::cout << "created A with val " << val << std::endl;
	}
	virtual ~A() {
		std::cout << "deleted A with val " << val << std::endl;
	}
	int val;
};

class B : public object {
public:
	B(int i) : val(i) {
		std::cout << "created B with val " << val << std::endl;
	}
	virtual ~B() {
		std::cout << "deleted B with val " << val << std::endl;
	}
	int val;
};

class C : public object {
public:
	C(int i) : val(i) {
		std::cout << "created C with val " << val << std::endl;
	}
	virtual ~C() {
		THROW(PLSSTOP);
		std::cout << "deleted C with val " << val << std::endl;
	}
	int val;
};


void testClasses() {
	A a(1);
	TRY {
		A b(2);
		B c(5);
		std::cout << "exception soon" << std::endl;
		THROW(BAD); 
	} CATCH(BAD) {
		std::cerr << "BAD has been caught!\n";
	} FINALLY
}

void testClasses2() {
	TRY{
		C c(100);
	THROW(BAD);
	} CATCH(BAD) {
		std::cerr << "BAD has been caught!\n";
	} FINALLY
}

void bar(int val) {
	std::cout << "bar was called with val " << val << std::endl;
	A a(val);
	THROW(BAD);
}

void foo(int val) {
	std::cout << "foo was called with val " << val << std::endl;
	B b(val);
	bar(val);
	B c(val + 1);
}

void testFunc() {
	TRY {
		foo(5);
	} CATCH(VERYBAD) {
		std::cerr << "VERYBAD has been caught!\n" << std::endl;
	} FINALLY
}

int main() {
	testClasses2();
	return 0;
}

