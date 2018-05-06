#include <csetjmp>
#include <exception>
#include <iostream>

#define BAD 1
#define VERYBAD 2
#define PLSSTOP 3


#define TRY \
	StackElem next_se; \
	int exc = (int) setjmp(next_se.buf); \
	num_of_exceptions = 1; \
	num_of_catches = 0; \
	if (exc == 0)

#define CATCH(err)  \
	else if (err == exc && ++num_of_catches)

#define THROW(exc_)  \
	if (num_of_exceptions == 0) { \
		std::cerr << "Another exception during process. Continuing...\n"; \
	} else { \
		num_of_exceptions = 0; \
		untwist_stack(exc_); \
	}

#define FINALLY \
	if (num_of_catches == 0) { \
		THROW(exc); \
	}
		
static int num_of_exceptions = 0; //для проверки, что не выкидывается исключение во время раскрутки стека
static int num_of_catches = 0;

class Object {
public:
	Object();
	virtual ~Object();

	Object* prev; //предыдущий вызванный элемент
};

class StackElem {
public:
	StackElem();

	~StackElem();

	jmp_buf buf; //непосредственно буфер, необходимый для setjmp/longjmp
	StackElem* prev; //указатель на предыдущий элемент стека
	Object* obj; //указатель на объект, соответствующий данной ячейке стека
};

static StackElem* stack_root = NULL; //вершина стека

StackElem::StackElem() {
	obj = NULL;
	prev = stack_root;
	stack_root = this;
}

StackElem::~StackElem() {
	stack_root = prev; //меняем вершину
}


Object::Object() {//при создании объекта кладем его на стек
	if (stack_root != NULL) { //можем создать объект не в TRY/CATCH, и тогда корень еще не будет определен и делать ничего не надо
		prev = stack_root->obj;
		stack_root->obj = this;
	}
}

Object::~Object() {
	if (stack_root != NULL && prev != NULL) {
		stack_root->obj = prev;
		prev = NULL;
	}
}

void untwist_stack(int code) {
	if (code == 0) {
		return;
	}
	StackElem* curr_se = stack_root;
	Object* curr_obj = stack_root->obj;
	while (curr_obj != NULL) { //последовательно вызываем деструкторы объектов
		Object* tmp_obj = curr_obj;
		curr_obj = curr_obj->prev;
		tmp_obj->~Object();
	}
	if (curr_se != NULL) { //меняем вершину стека
		stack_root = curr_se->prev;
	}
	longjmp(curr_se->buf, code); //откатываемся
	return;
}

//Тестовые классы просто выводят на экран, когда они вызывают констуктор и деструктор
class A: public Object {
public:
	A(int i) : val(i) {
		std::cout << "created A with val " << val << std::endl;
	}
	virtual ~A() {
		std::cout << "deleted A with val " << val << std::endl;
	}
	int val;
};

class B : public Object {
public:
	B(int i) : val(i) {
		std::cout << "created B with val " << val << std::endl;
	}
	virtual ~B() {
		std::cout << "deleted B with val " << val << std::endl;
	}
	int val;
};

class C : public Object {
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
		THROW(BAD); 
	} CATCH(BAD) {
		std::cerr << "BAD has been caught!\n";
	} FINALLY
}

void testClasses2() {
	A a(1);
	TRY{
		C c(100);
		THROW(BAD);
	} CATCH(BAD) {
		std::cerr << "BAD has been caught!\n";
	} FINALLY
}

void testClasses3() {
	TRY{
		TRY{
			TRY {
				A a(1);
				THROW(BAD);
			} CATCH(VERYBAD) {
				std::cerr << "Inner VERYBAD has been caught! (not good)\n";
			} FINALLY
		} CATCH(BAD) {
			std::cerr << "BAD has been caught!\n";
		} FINALLY
	} CATCH(BAD) {
		std::cerr << "Next BAD has been caught! (not good)\n";
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
		std::cerr << "VERYBAD has been caught!\n";
	} FINALLY
}

int main() {
	testClasses2();
	return 0;
}

