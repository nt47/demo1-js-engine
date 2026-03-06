#pragma once
#include <iostream>

class Animal
{
public:
	virtual ~Animal() {}

	__declspec(noinline)
		virtual void Speak()
	{
		std::cout << "Animal::Speak()" << std::endl;
	}

	__declspec(noinline)
		virtual int GetAge()
	{
		std::cout << "Animal::GetAge()" << std::endl;
		return 1;
	}

	__declspec(noinline)
		virtual const char* GetType(int n, const char* name)
	{
		std::cout << "Animal::GetType()" << n << name << std::endl;
		return "Animal";
	}
};

// 工厂函数声明（关键）
Animal* CreateAnimal();

extern Animal* g_animal;