#pragma once
#include <exception>
#include <QString>

class SylabelFormatException : public std::exception
{
public:
    enum class Reason {
        EmptyLine, InvalidType, NotEnoughData, InvalidNumber, ExtraData
    };

    SylabelFormatException(QString sylabel, Reason reason);

    Reason reason() const { return reason_ ; }
    QString sylabel() const { return sylabel_; }
    const char* what() const noexcept override { return message_.c_str(); }

private:
    QString sylabel_;
    Reason reason_;
    std::string message_;
};
