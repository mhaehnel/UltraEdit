#include "exceptions/songparseexception.h"

SongParseException::SongParseException(QString filename, Reason reason, int line)
    : filename_(filename), reason_(reason) {

    switch (reason) {
        case Reason::FileDoesNotExist:
            message_ = QString("File %1 does not exist").arg(filename_).toStdString();
            break;
        case Reason::PermissionDenied:
            message_ = QString("Permission denied when opening %1 for read/write").arg(filename_).toStdString();
            break;
        case Reason::UnreadableLine:
            message_ = QString("Unable to read line %1 in %2").arg(line).arg(filename).toStdString();
            break;
    }
}



SongParseException::Reason SongParseException::reason() const {
    return reason_;
}

QString SongParseException::filename() const {
    return filename_;
}

const char* SongParseException::what() const noexcept {
    return message_.c_str();
}
