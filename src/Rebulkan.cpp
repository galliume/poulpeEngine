#include "Rebulk/Application.h"

int main(int argc, char** argv)
{
	std::unique_ptr<Rbk::Application>app = std::make_unique<Rbk::Application>(Rbk::Application());

	app->Run();

	return 0;
}