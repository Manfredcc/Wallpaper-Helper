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
#include <cstdlib>
#include <random>

#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/inotify.h>

#include "ThreadPool/ThreadPool.h"

using namespace std;

class wpHelper {
public:
    wpHelper();
    ~wpHelper();
    static wpHelper* instantiate();

    enum class changeType {
        NEXT,
        PREV,
        RANDOM,
    };
    void change(changeType type);
    void change(const string& wp);
    void addLib(string& path) {
        lock_guard<mutex> lock(mInfoLock);
        mLib.push_back(path);
    }
    void setAutoSwitch(int interval, bool enable, changeType type);
    void showLibs();

private:
    void loadInfo();
    int writeInfo(const string& elem, bool add);

    /* thread pool */
    vector<future<int>> results;
    ThreadPool mPool;
    void OnThreads();

    // auto switch wallpaper
    void autoSwitchT();
    typedef struct _autoSwitch {
        int interval;
        bool enable;
        changeType type;
    } autoSwitch;
    autoSwitch mAutoSwitch;
    mutex mAutoSwitchLock;

    vector<string> mLib;
    map<int, string> mWpList;
    int mWpNum;
    vector<int> mHistory;
    int mCurWp;
    mutex mInfoLock;
};







#endif /* __WP_HELPER_H__ */