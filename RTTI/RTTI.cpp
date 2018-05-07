// RTTI.cpp: определяет точку входа для консольного приложения.
//

#include <stdio.h>
#include <tchar.h>
#include <string>
#include <set>
#include <vector>
#include <map>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>

//Храним RTTI-таблицу, в которой для каждого класса лежат его название и сет имен родителей (родитель -- любой класс, от которого по цепочке можно скастоваться к данному)
static std::vector < std::pair< std::string, std::set < std::string>>> rtti_table;

//Проверяем наличие элемента в таблице. Если нет, то добавляем по следующему принципу: во-первых, родителей, во-вторых, родителей родителей. 
//Таким образом, возможность кастования можно проверить, просто посмотрев таблицу
#define INCLUDE_IN_RTTI(main_class, ...) { \
	bool to_add = true; \
	for (auto& elem : rtti_table) { \
		if (elem.first == #main_class) { \
			to_add = false; \
			break; \
		} \
	} \
	if (to_add) { \
		std::vector<std::string> curr_parents; \
		std::string parents = #__VA_ARGS__; \
		parents.erase(std::remove_if(parents.begin(), parents.end(), std::isspace), parents.end()); \
		std::istringstream f(parents); \
		std::string s; \
		while (std::getline(f, s, ',')) { \
			curr_parents.push_back(s); \
		} \
		std::set<std::string> new_parents(curr_parents.begin(), curr_parents.end()); \
		for (int i = 0; i < rtti_table.size(); i++) { \
			if (std::find(curr_parents.begin(), curr_parents.end(), rtti_table[i].first) != curr_parents.end()) { \
				new_parents.insert(rtti_table[i].second.begin(), rtti_table[i].second.end()); \
			} \
		} \
		rtti_table.push_back(std::pair<std::string, std::set <std::string>>(#main_class, new_parents)); \
	} \
}

//Возвращаем информацию об объекте, если он не скастовался ранее -- соответствующее имя класса
#define TYPEID(obj_ptr) \
	(obj_ptr != nullptr) ? obj_ptr->Object::info : TypeInfo("OBJECT WAS NOT CASTED SUCCESSFULLY", 0); 

//Проверяем, можно ли кастовать эти типы, и если да, просто делаем каст; иначе возвращаем nullptr
#define DYNAMIC_CAST(first_type, second_type, second_type_ptr) \
	can_cast(#first_type, #second_type, second_type_ptr) ? reinterpret_cast<first_type*>(second_type_ptr) : nullptr;
	
class TypeInfo {
public:
	TypeInfo() {}
	TypeInfo(std::string class_name_, int hash_) : class_name(class_name_), hash(hash_) {}
	std::string class_name;
	int hash;
};

class Object {
public:
	Object() {
		std::string class_name = rtti_table.back().first;
		std::hash<std::string> new_hash;
		int hash = new_hash(class_name);
		info = TypeInfo(class_name, hash);
	}
	virtual ~Object() {};

	TypeInfo info;
};


//Ищем наш класс в таблице, далее пытаемся найти таргет-класс в его родителях. Если находим, то скастовать можно; иначе нельзя.
template<typename T> bool can_cast(std::string first_type, std::string second_type, T ptr) {
	for (auto& elem : rtti_table) {
		if (elem.first.compare(second_type) != -1) {
			for (auto& e : elem.second) {
				if (!(e.compare(first_type))) {
					ptr->Object::info.class_name = first_type;
					return true;
				}
			}
			return false;
		}
	}
	return false;
}

class A : public Object {
public:
	A() {
		std::cout << "created A\n";
	}
	virtual ~A() {
		std::cout << "deleted A\n";
	}
};

class B : public Object {
public:
	B() {
		std::cout << "created B\n";
	}
	virtual ~B() {
		std::cout << "deleted B\n";
	}
};

class C : public Object, public A, public B {
public:
	C() {
		std::cout << "created C\n";
	}
	virtual ~C() {
		std::cout << "deleted C\n";
	}
};


void test1() {
	C* c = new C();
	C* c2 = new C();
	TypeInfo c_info = TYPEID(c);
	std::cout << "c class is " << c_info.class_name << std::endl;

	A* a = DYNAMIC_CAST(A, C, c);
	TypeInfo a_info = TYPEID(a);
	std::cout << "a class is " << a_info.class_name << std::endl;

	TypeInfo c2_info = TYPEID(c2);
	std::cout << "c2 class is " << c2_info.class_name << std::endl;
	delete c;
	delete c2;
}

class X : public Object {
public:
	X() {
		std::cout << "created X\n";
	}
	virtual ~X() {
		std::cout << "deleted X\n";
	}
};

class Y : public Object, public X {
public:
	Y() {
		std::cout << "created Y\n";
	}
	virtual ~Y() {
		std::cout << "deleted Y\n";
	}
};

class Z : public Object, public Y {
public:
	Z() {
		std::cout << "created Z\n";
	}
	virtual ~Z() {
		std::cout << "deleted Z\n";
	}
};


void test2() {
	Z* z = new Z();
	TypeInfo z_info = TYPEID(z);
	std::cout << "z class is " << z_info.class_name << std::endl;

	X* x = DYNAMIC_CAST(X, Z, z);

	TypeInfo x_info = TYPEID(x);
	std::cout << "x class is " << x_info.class_name << std::endl;

	delete z;
}

void test3() {
	Z* z = new Z();

	TypeInfo z_info = TYPEID(z);
	std::cout << "z class is " << z_info.class_name << std::endl;

	A* a = DYNAMIC_CAST(A, Z, z);

	TypeInfo a_info = TYPEID(a);
	std::cout << "a class is " << a_info.class_name << std::endl;

	std::cout << (a == nullptr) << std::endl;
}
int main()
{
	INCLUDE_IN_RTTI(C, A, B)
	INCLUDE_IN_RTTI(Y, X)
	INCLUDE_IN_RTTI(Z, Y)
	test3();
    return 0;
}

