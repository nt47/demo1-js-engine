//这是阻止 devirtualization 的关键
//main.cpp 编译时看不到 Dog 的定义 → 动态类型未知 → 强制走 vtable

#include "Animal.h"

Animal* g_animal = nullptr;

class Dog : public Animal
{
public:
	__declspec(noinline)
		void Speak() override
	{
		std::cout << "Dog::Speak()" << std::endl;
	}

	__declspec(noinline)
		int GetAge() override
	{
		std::cout << "Dog::GetAge()" << std::endl;
		return 5;
	}

	__declspec(noinline)
		const char* GetType(int n, const char* name) override
	{
		std::cout << "Dog::GetType()" << n << name << std::endl;
		return "Dog";
	}
};

Animal* CreateAnimal()
{
	return new Dog();   // main.cpp 无法看到 Dog 的定义
}
