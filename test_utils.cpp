// #include <iostream>
#include <string>
#include <vector>

bool brackets_match(std::string data) {
    std::vector<char> stack;
    for (int i = 0; i < data.length(); i++) {
        switch (data[i]) {
            case '{': {
                stack.push_back(data[i]);
                break;
            }
            case '[': {
                stack.push_back(data[i]);
                break;
            }
            case '}': {
                if (stack.empty() || stack.back() != '{') {
                    return false;
                }
                stack.pop_back();
                break;
            }
            case ']': {
                if (stack.empty() || stack.back() != '[') {
                    return false;
                }
                stack.pop_back();
                break;
            }
        }
    }
    return stack.empty();
}


/*int main() {
    while (true) {
        std::string data;
        std::cout << "Enter a string with '[,],{,}': ";
        std::cin >> data;

        switch (brackets_match(data)) {
            case 0: {
                std::cout << "Failed\n";
                break;
            }
            case 1: {
                std::cout << "Passed\n";
                break;
            }
        }
    }
}*/
