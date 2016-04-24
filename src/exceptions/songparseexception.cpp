#include "exceptions/songparseexception.h"

QString SongParseException::filename() { return filename_; }
SongParseException::Reason SongParseException::reason() { return reason_; }
