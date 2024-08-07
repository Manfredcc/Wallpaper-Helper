#include "wpHelper.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

static const string fifoPath = "/tmp/wpHelperFifo";

static vector<string> supportCmdList = {
    "[-h]  |            ...              | show support cmd list",
    "[-c]  |         [filePath]          | switch filePath to be a wallpaper",
    "[-n]  |            ...              | switch to next wallpaper",
    "[-p]  |            ...              | switch to previous wallpaper",
    "[-r]  |            ...              | switch to a random wallpaper",
    "[-s]  |  [interval, enable, type]   | setting auto-swtich",
};

static void establishListen(const string& fifoPath, wpHelper * wp);

int main()
{
    wpHelper* wp = wpHelper::instantiate();

    establishListen(fifoPath, wp); // main() will be block here in normal
    cout << "wpHelperService exits" << endl;
}

int listenFifoT(const string& path)
{
    int fifoFd = open(path.c_str(), O_RDONLY);
    if (-1 == fifoFd) {
        cerr << "filed to open " << path << endl;
        return fifoFd;
    }

    char buf[512] = {};
    for (;;) {
        if (int size = read(fifoFd, buf, sizeof(buf)) > 0) {
            /* process with cmd from shell */
            if (buf[size] == '\n') {
                buf[size] = '\0';
            }
            cout << "cmd is " << buf << endl;
            system(buf);

            // process cmd
            memset(buf, '\0', sizeof(buf));
        }
    }
}

void callCmd(const string& jsonString)
{
nlohmann::json cmd = nlohmann::json::parse(jsonString);
}

static void establishListen(const string& path, wpHelper * wp)
{
    if (-1 == access(path.c_str(), F_OK)) {
        if (-1 == mkfifo(path.c_str(), 0666)) {
            cerr << "failed to create " << path << endl;
            return;
        } else {
            cout << "created " << path << endl;
        }
    } else {
        cout << path << " exists, no need to create again" << endl;
    }

    auto f = async(launch::async, listenFifoT, path);
    f.get();
}