#include <string>

class Role
{
public:
	Role();
	~Role();

private:
	std::string name;
};

Role::Role()
{
}

Role::~Role()
{
	delete this;
}