#include "wpHleper.h"

/* ============= configuration start ============= */
const char *wpRecycleBin = "~/.wpRecycleBin";

const char *wpChangeCmd = "feh --bg-fill --recursive";

vector<string> defaultLib = {
    "/home/ll/1_resource/02_photo/wallpapers",
    "/home/ll/.wpHelper",
};

static int autoSwitchInterval = 5 * 60; // seconds
static bool autoSwitchEnable = true;
static wpHelper::changeType autoSwitchtype = wpHelper::changeType::NEXT;

#define THREAD_NUMS 10

/* ============== configuration end ============== */

static bool wpDebugEnable = true;
#define wpDebug(fmt, ...)   \
    do {    \
        if (wpDebugEnable) {    \
            printf("[wpHelper]<%d>[%s]: " fmt "\n", __LINE__, __func__, ##__VA_ARGS__);	\
        }   \
    } while (0)



wpHelper::wpHelper() : mPool(THREAD_NUMS), mLibChangedInRuntime(false),
    mWpNum(0), mCurWp(0)
{
    mAutoSwitch = { autoSwitchInterval, autoSwitchEnable, autoSwitchtype };
    wpDebug("wpHelper");
}

wpHelper::~wpHelper()
{
    wpDebug("~wpHelper()");
}

wpHelper* wpHelper::instantiate()
{
    wpHelper *pWp = new wpHelper();

    for (auto& it : defaultLib) {
        pWp->modifyLib(it, true);
        wpDebug("add defaultLib:%s", it.c_str());
    }

    pWp->loadInfo();
    pWp->OnThreads();

    return pWp;
}

void wpHelper::modifyLib(string& path, bool flag)
{
    lock_guard<mutex> lock(mInfoLock);
    if (flag) {
        mLib.emplace_back(path);
    } else {
        mLib.erase(remove(mLib.begin(), mLib.end(), path), mLib.end());
    }

    mLibChangedInRuntime = true;
}

/*
 * Filter images by checking file suffixes
 */
static inline int filterWp(const string& str)
{
	vector<string> validWp = { ".png", ".jpg", ".jepg" };
	for (size_t i = 0; i < validWp.size(); i++) {
		if (str.length() >= validWp[i].length()) {
			return (str.compare(str.length() - validWp[i].length(), 
						validWp[i].length(), validWp[i]));
		}
	}

	return -1;
}

/*
 * Alternative solution for string.format()
 */
template<typename ... Args>
static inline string string_format(const string& str, Args ... args)
{
    auto size = snprintf(nullptr, 0, str.c_str(), args ...) + 1;
    unique_ptr<char> buf(new(nothrow) char[size]);
    if(!buf)
        return string("");
    
    snprintf(buf.get(), size, str.c_str(), args ...);
    return string(buf.get(), buf.get() + size - 1);
}

/*
 * Add/remove wallpaper in libs
 */
int wpHelper::writeInfo(const string& elem, bool add)
{
    lock_guard<mutex> lock(mInfoLock);
    if (0 != filterWp(elem)) {
        return -1;
    }

    if (add) {
        mWpList[mWpNum++] = elem;
    } else { /* remove wallpaper to wpRecycleBin */
        for (auto it = mWpList.begin(); it != mWpList.end(); it++) {
            if (it->second == elem) {
                it->second = "";
                wpDebug("found %s and delete it from wpList", elem.c_str());
                break;
            }
        }

        if (-1 == access(wpRecycleBin, F_OK)) {
            wpDebug("%s is not exists, creates it", wpRecycleBin);
            if (-1 == mkdir(wpRecycleBin, 0777)) {
                wpDebug("failed to create %s", wpRecycleBin);
                return -1;
            }
        }
        string cmd = string_format("mv %s %s", elem.c_str(), wpRecycleBin);
        system(cmd.c_str());
        wpDebug("delete %s", elem.c_str());
    }

    return 0;
}

/*
 * Read files from wallpaper lib to get 
 * valid photos as wallpapers
 */
void wpHelper::loadInfo()
{
    DIR *dir;
    struct dirent *ent;
    int ret = 0;

    for (auto &p : mLib) {
        /* create wallpaper lib path if it doesn't exist */
        if (-1 == access(p.c_str(), F_OK)) {
            wpDebug("%s is not exists, creates it", p.c_str());
            if (-1 == mkdir(p.c_str(), 0777)) {
                wpDebug("Failed to created %s, continue", p.c_str());
                continue;
            }
        }

        /* iterate over all wallpaper libs and load info */
        if (NULL != (dir = opendir(p.c_str()))) {
            while (NULL != (ent = readdir(dir))) {
                string elem = string_format("%s/%s", p.c_str(), ent->d_name);
                ret |= writeInfo(elem, true);
            }
        } else {
            wpDebug("Failed to open %s, continue", p.c_str());
            continue;
        }
    }
}

/*
 * provide with basic wallpaper change command
 */
void wpHelper::change(const string& wp)
{
    string cmd = string_format("%s %s", wpChangeCmd, wp.c_str());
    if (-1 == access(wp.c_str(), F_OK)) {
        wpDebug("can't access %s", wp.c_str());
        return;
    }

    wpDebug("integrated cmd is:%s", cmd.c_str());
    system(cmd.c_str());
}

/*
 * change wallpaper in specific type
 * @type: next/previous/random
 */
void wpHelper::change(changeType type)
{
    lock_guard<mutex> lock(mInfoLock);
    switch (type) {
    case changeType::NEXT: {
            mHistory.emplace_back(mCurWp);
            do {
                mCurWp = (mCurWp != mWpNum) ? (mCurWp + 1) : 0;
                if (mWpList[mCurWp] == "")
                    continue;
                
                break;
            } while (1);
            break;
        }

        case changeType::PREV: {
            mCurWp = mHistory.back();
            mHistory.pop_back();
            break;
        }

        case changeType::RANDOM: {
            mHistory.emplace_back(mCurWp);
            random_device rd;
            mt19937 gen(rd());
            int random_number = 0;
            uniform_int_distribution<> dis(0, mWpNum);
            do {
                random_number = dis(gen);
                if (mWpList[random_number] == "")
                    continue;

                mCurWp = random_number;
            } while (1);
            break;
        }

        default:{
            wpDebug("bad logic case!");
            break;
        }
    }

    wpDebug("mCurWp:%d", mCurWp);
    change(mWpList[mCurWp]);
}

void wpHelper::showLibs()
{
    for (auto& it : mLib) {
        cout << "LibName: " << it << endl;
    }

    for (auto& elem : mWpList) {
        cout << "elem: " << elem.second << endl;
    }
}

void wpHelper::autoSwitchT()
{
    for (;;) {
        mAutoSwitchLock.lock();
        autoSwitch tmp = mAutoSwitch;
        mAutoSwitchLock.unlock();
        if (tmp.enable) {
            changeType type = tmp.type;
            if (0 == mHistory.size() && tmp.type == changeType::PREV) {
                type = changeType::RANDOM;
            }
            change(type);
        }
        
        sleep(tmp.interval);
    }
}

#define EVENT_SIZE  (sizeof(struct inotify_event))
#define EVENT_BUF_LEN     (1024 * (EVENT_SIZE + 16))
int wpHelper::monitorLibT()
{
    int inotifyFd = inotify_init();
    if (inotifyFd < 0) {
        wpDebug("failed to initialize inotify fd");
    }

    map<string, int> nameWdMap;
    {
        lock_guard<mutex> lock(mInfoLock);
        size_t i = 0;
        for (; i < mLib.size(); i++) {
            int wd = inotify_add_watch(inotifyFd, mLib[i].c_str(), IN_CREATE | IN_DELETE);

            if (wd < 0) {
                wpDebug("failed to add %s to the monitor", mLib[i].c_str());
                continue;
            }
        }

        if (0 == i) {
            wpDebug("monitor action had been deeply failure!");
            return -1;
        }
    }

    fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(inotifyFd, &rfds);

    for (;;) {
        int ret = select(inotifyFd + 1, &rfds, NULL, NULL, NULL);
        if (ret < 0) {
			wpDebug("add fd-set to select fail");
			return ret;
		}

        if (FD_ISSET(inotifyFd, &rfds)) {
			char buffer[EVENT_BUF_LEN];
			int length = read(inotifyFd, buffer, EVENT_BUF_LEN);
			if (length < 0) {
                wpDebug("read error");
				return length;
			}

			for (int i = 0; i < length;) {
				struct inotify_event *event = (struct inotify_event *)&buffer[i];
				if (event->len) {
                    wpDebug("wp elem changed:%s", event->name);
				}

				i += EVENT_SIZE + event->len;
			}
		}
    }
}

void wpHelper::OnThreads()
{
    mPool.enqueue(bind(&wpHelper::autoSwitchT, this));
    mPool.enqueue(bind(&wpHelper::monitorLibT, this));
}

void wpHelper::setAutoSwitch(int interval, bool enable, changeType type)
{
    lock_guard<mutex> lock(mAutoSwitchLock);
    mAutoSwitch.interval = interval;
    mAutoSwitch.enable = enable;
    mAutoSwitch.type = type;
}