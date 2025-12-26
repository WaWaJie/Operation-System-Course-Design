#pragma once

template<typename T>
class Singleton
{
public:
	static T* instance()
	{
		if (!singleton)
			singleton = new T();

		return singleton;
	}

protected:
	Singleton() = default;
	~Singleton() = default;
	Singleton(Singleton&) = delete;
	Singleton operator=(Singleton&) = delete;

private:
	static T* singleton;
};

template<typename T>
T* Singleton<T>::singleton = nullptr;
