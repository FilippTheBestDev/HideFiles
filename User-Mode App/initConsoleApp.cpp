#include "initConsoleApp.h"

int main_console_entry_point() {
    printf("Loading driver.\n");
    if (!LoadDriver()) {
        printf("Could not load driver, exiting...\n");
        system("pause");
        return -1;
    }

    while (true) {
        std::string tmp;
        std::cout << "Enter name of file (with extension, ie \"HideMe.txt\") to modify (\"?\" to view current rules, \"*\" to clear all rules or \"@\" to stop driver and exit):\n";
        std::cin >> tmp;

        if (!tmp.length())
            continue;
        if (tmp.length() == 1 && tmp.at(0) == '@')
            break;
        if (tmp.length() == 1 && tmp.at(0) == '*') {
            DriverDeleteAllRules();
            continue;
        }
        if (tmp.length() == 1 && tmp.at(0) == '?') {
            DriverShowAllRules();
            continue;
        }

        std::string tmp2;
        unsigned char action;
        while (true) {
            std::cout << "Enter number of action:\n0 - Add to rules;\n1 - Delete from rules;\n";
            std::cin >> tmp2;
            if (tmp2.length() != 1)
                continue;

            action = tmp2.at(0) - '0';
            if (action > 1)
                continue;

            break;
        }

        if (action == 0)
            DriverAddRule(std::wstring(tmp.begin(), tmp.end()));
        else if (action == 1)
            DriverDeleteRule(std::wstring(tmp.begin(), tmp.end()));
    }

    if (!UnloadDriver())
        printf("Driver was not unloaded successfully.\n");
    system("pause");

    return 0;
}