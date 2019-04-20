#include <iostream>
#include <string>
#include <boost/lexical_cast.hpp>

int main()
{
    int i = 90;
    // std::ostringstream st;
    // st << i;
    std::string str = "pointLights[" + boost::lexical_cast<std::string>(i) + "].position";
    std::cout << str << "\n";
    return 0;
}