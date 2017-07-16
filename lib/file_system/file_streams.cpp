#include "file_streams.h"

MultiFileStream::MultiFileStream(std::initializer_list<const char *> fs) {
    files = new char *[fs.size()];
    for (auto f :fs) {
        files[files_len++] = (char *) f;
    }
}

uint32_t MultiFileStream::size() const {
    uint32_t len = 0;
    for (uint32_t i = 0; i < files_len; i++) {
        yield(); // WATCHDOG/WIFI feed
        File file = SPIFFS.open(files[i], "r");
        if (file) {
            len += file.size();
            file.close();
        }
    }
    return len;
}

int MultiFileStream::read(byte *buf, uint16_t nbyte) {
    int rd = 0, cp = 0;
    for (uint32_t i = 0; i < files_len && rd < nbyte; i++) {
        yield(); // WATCHDOG/WIFI feed
        File f = SPIFFS.open(files[i], "r");
        if (!f) {
            continue;
        }
        if (p > cp + f.size()) {
            cp += f.size();
        } else {
            f.seek(p - cp, SeekSet);
            rd += f.read(buf + rd, (size_t)(nbyte - rd));
        }
        f.close();
    }
    p += rd;
    return rd;
}

int MultiFileStream::available() const {
    const uint32_t s = size();
    return (int) ((s - p) > 0 ? s - p : 0);
};

const char *MultiFileStream::name() const { return ""; }

void MultiFileStream::rewind() { p = 0; }

MultiFileStream::~MultiFileStream() { delete files; }