#include <list>
#include <string>

std::list<std::string> split(std::string str) {
    std::list<std::string> tokens;
    std::string s;
    for(char& c : str) {
        if(c == ' ') {
            tokens.push_back(s);
            s = "";
        } else {
            s += c;
        }
    }
    if(s != "") tokens.push_back(s);
    return tokens;
}