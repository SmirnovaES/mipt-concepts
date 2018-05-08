// Inheritance.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include <map>
#include <set>
#include <iostream>
#include <string>

static std::map< std::string, std::set<std::string> >  func_table; //имя функции; полное имя функции
static std::map< std::string, std::string> virt_table;

#define VIRTUAL_CLASS(ClassName) \
	class ClassName { \
	public: \
		ClassName(): class_name(#ClassName) {} \
		std::string class_name;

#define VIRTUAL_CLASS_DERIVED(ClassName, ParentName) \
	class ClassName { \
	public: \
		ClassName(): class_name(#ClassName), parent_name(#ParentName) { \
			virt_table.insert(std::make_pair(#ClassName, #ParentName)); \
		} \
		std::string class_name; \
		std::string parent_name; 

#define END_CLASS };

#define DECLARE_METHOD(ClassName, func_name) { \
	auto class_name_it = func_table.find(#ClassName); \
	if (class_name_it != func_table.end()) { \
		class_name_it->second.insert(#func_name); \
	} else { \
		func_table.insert(std::make_pair(#ClassName, std::set<std::string>())); \
		func_table.find(#ClassName)->second.insert(#func_name); \
	} \
}

#define VIRTUAL_CALL(class_ptr, func_name) { \
	std::string class_name = (*class_ptr).class_name; \
	auto class_iter = virt_table.find(class_name); \
	if (class_iter == virt_table.end()) { \
		if (func_table.find(class_name)->second.find(#func_name) == func_table.find(class_name)->second.end()) { \
			throw std::exception("Function was not declared!\n"); \
		} else { \
			std::cout << class_name << "::" << #func_name << " was called!\n"; \
		} \
	} else { \
		std::string parent_name = class_iter->second; \
		if (func_table.find(class_name)->second.find(#func_name) == func_table.find(class_name)->second.end()) { \
			if (func_table.find(parent_name)->second.find(#func_name) == func_table.find(parent_name)->second.end()) { \
				throw std::exception("Function was not declared!\n"); \
			} else { \
				std::cout << parent_name << "::" << #func_name << " was called!\n"; \
			} \
		} else { \
			std::cout << class_name << "::" << #func_name << " was called!\n"; \
		} \
	} \
}

VIRTUAL_CLASS(Base)
int a;
END_CLASS

// класс-наследник
VIRTUAL_CLASS_DERIVED(Derived, Base)
int b;
END_CLASS


int main() {
	DECLARE_METHOD(Base, Both)
	DECLARE_METHOD(Base, OnlyBase)
	DECLARE_METHOD(Derived, Both)
	DECLARE_METHOD(Derived, OnlyDerived)
	Base base;
	Derived derived;
	Base* reallyDerived = reinterpret_cast<Base*>(&derived);

	VIRTUAL_CALL(&base, Both);
	VIRTUAL_CALL(reallyDerived, Both);
	VIRTUAL_CALL(reallyDerived, OnlyBase); 
	VIRTUAL_CALL(reallyDerived, OnlyDerived);
	try {
		VIRTUAL_CALL(&base, OnlyDerived);
	}
	catch (...) {
		std::cerr << "Function was not declared!\n";
	}
	return 0;
}

