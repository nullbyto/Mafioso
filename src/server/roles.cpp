#include <string>

class Roles
{
private:
    std::string name;

public:
    Roles(std::string name);
    ~Roles();

    std::string getName() {
        return this->name;
    }

    void action(){}
};

Roles::Roles(std::string name)
{
    this->name = name;
}

Roles::~Roles()
{

}
