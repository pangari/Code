#include <stdio.h>

#include "FileGlobBase.H"

/**
Provides a storage list for all found files.
**/
class FileGlobList : public FileGlobBase, public std::list< std::string >
{
public:
    typedef std::list< std::string >::iterator Iterator;

    virtual void FoundMatch( const char* fileName )
    {
        push_back( fileName );
    }

protected:

private:
};

class FileGlobPrint : public FileGlobBase, public std::list< std::string >
{
public:
    FileGlobPrint(FILE* _file) : file(_file), count(0) {}

    virtual void FoundMatch( const char* fileName )
    {
        fprintf(file, "%s\n", fileName);
        count++;
    }

    virtual size_t Count()
    {
        return count;
    }

protected:
    FILE*   file;
    size_t  count;

private:
};
