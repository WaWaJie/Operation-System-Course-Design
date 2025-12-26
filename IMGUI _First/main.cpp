#define SDL_MAIN_HANDLED

#include"application.h"

int main(int argv, char** argc)
{

	Application::instance()->run(argv, argc);
}
