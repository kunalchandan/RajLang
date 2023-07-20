#include <string>

class SourceCode
{
public:
    /* Raw data for the source code, contains Path and Source */
    std::string path;
    std::string raw_document;

    SourceCode();
    ~SourceCode();
};