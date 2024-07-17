#include "wpHleper.h"

/* ============= configuration start ============= */
const char *wpRecycleBin = "~/.wpRecycleBin";

const char *wpChangeCmd = "feh --bg-fill --recursive";

vector<string> defaultLib = {
    "/home/ll/1_resource/02_photo/wallpapers",
    "/home/ll/.wpHelper",
};

/* ============== configuration end ============== */

static bool wpDebugEnable = true;
#define wpDebug(fmt, ...)   \
    do {    \
        if (wpDebugEnable) {    \
            printf("[wpHelper]<%d>[%s]: " fmt "\n", __LINE__, __func__, ##__VA_ARGS__);	\
        }   \
    } while (0)



wpHelper::wpHelper()
{
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
        pWp->addLib(it);
        wpDebug("add defaultLib:%s", it.c_str());
    }

    pWp->loadInfo();

    return pWp;
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
        mWpList[index++] = elem;
    } else { /* remove wallpaper to wpRecycleBin */
        for (auto it = mWpList.begin(); it != mWpList.end(); it++) {
            if (it->second == elem) {
                it = mWpList.erase(it);
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
static inline void change(const string& wp)
{
    string cmd = string_format("%s %s", wpChangeCmd, wp.c_str());
    wpDebug("integrated cmd is:%s", cmd.c_str());
    system(cmd.c_str());
}

/*
 * change wallpaper in specific type
 * @type: next/previous/random
 */
void wpHelper::change(changeType type)
{
    list<string>::iterator it;

    switch (type) {
    case changeType::NEXT:
        break;
    case changeType::PREV:
        break;
    case changeType::RANDOM:
        break;
    default:
        wpDebug("bad logic case!");
        break;
    }
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