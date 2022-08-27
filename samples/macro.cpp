#include <iostream>
#include <vector>

#include "../macro_utils.h"

using namespace std;

int main()
{
    vector<int> values(20);

    for (auto &&i : values)
    {
        i = rand();
    }

    FOR_EACH (values)
    {
        cout << it;
    }

    return 0;
}
