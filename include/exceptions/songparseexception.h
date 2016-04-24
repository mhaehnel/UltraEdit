#pragma once

#include <exception>
#include <QString>

class SongParseException : public std::exception
{
public:
    enum class Reason {
        FileDoesNotExist, PermissionDenied
    };

    SongParseException(QString filename, Reason reason)
        : filename_(filename), reason_(reason) {}

    Reason reason();
    QString filename();
    QString message();

private:
    QString filename_;
    Reason reason_;

};
