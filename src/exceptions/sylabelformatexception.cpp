#include "exceptions/sylabelformatexception.h"

SylabelFormatException::SylabelFormatException(QString sylabel, Reason reason)
    : sylabel_(sylabel), reason_(reason)
{
    switch (reason) {
        case Reason::EmptyLine:
            message_ = "Empty sylabel line";
            break;
        case Reason::InvalidType:
            message_ = QString("Invalid sylabel type in sylabel '%1'").arg(sylabel).toStdString();
            break;
        case Reason::NotEnoughData:
            message_ = QString("Sylabel '%1' contained not enough data").arg(sylabel).toStdString();
            break;
        case Reason::InvalidNumber:
            message_ = QString("Sylabel '%1' contained an invalid number").arg(sylabel).toStdString();
        case Reason::ExtraData:
            message_ = QString("Sylabel '%1' contained extra data").arg(sylabel).toStdString();
    }
}

