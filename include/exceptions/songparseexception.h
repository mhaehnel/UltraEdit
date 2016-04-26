#pragma once

#include <exception>
#include <QString>

class SongParseException : public std::exception
{
public:
    enum class Reason {
        FileDoesNotExist, PermissionDenied, UnreadableLine
    };

    SongParseException(QString filename, Reason reason, int line = -1);

    Reason reason() const;
    QString filename() const;
    const char* what() const noexcept override;

private:
    QString filename_;
    Reason reason_;
    std::string message_;
};
