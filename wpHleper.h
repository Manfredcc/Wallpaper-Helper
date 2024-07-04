#ifndef __WP_HELPER_H__
#define __WP_HELPER_H__

#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <list>
#include <mutex>
#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <map>
#include <cstring>

#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/inotify.h>

using namespace std;

class wpHelper {
public:
    wpHelper();
    ~wpHelper();
    static wpHelper* instantiate();

    void change(int type);

private:
    void loadInfo();
    int writeInfo(string& libPath, string& entry, bool add);

    vector<string> mLibName;
    map<string, vector<string>> mLib;
    vector<string> history;
};







#endif /* __WP_HELPER_H__ */