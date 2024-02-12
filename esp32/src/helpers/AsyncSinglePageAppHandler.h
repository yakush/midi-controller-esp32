#include <ESPAsyncWebServer.h>

static inline bool FILE_IS_REAL(File file)
{
#ifdef ESP32
    return (file == true && !file.isDirectory());
#else
    return file == true;
#endif
}

class AsyncSinglePageAppHandler : public AsyncWebHandler
{
protected:
    FS _fs;
    String _uri;
    String _path;
    String _default_file;
    AsyncStaticWebHandler *_staticHandler;

public:
    AsyncSinglePageAppHandler(const char *uri, FS &fs, const char *path, const char *cache_control)
        : _fs(fs), _uri(uri), _path(path), _default_file("index.htm")
    {
        this->_staticHandler = new AsyncStaticWebHandler(uri, fs, path, cache_control);

        // Ensure leading '/'
        if (_uri.length() == 0 || _uri[0] != '/')
            _uri = "/" + _uri;

        if (_path.length() == 0 || _path[0] != '/')
            _path = "/" + _path;

        // If path ends with '/' we assume a hint that this is a directory to improve performance.
        // However - if it does not end with '/' we, can't assume a file, path can still be a directory.
        //_isDir = _path[_path.length() - 1] == '/';

        // Remove the trailing '/' so we can handle default file
        // Notice that root will be "" not "/"
        if (_uri[_uri.length() - 1] == '/')
            _uri = _uri.substring(0, _uri.length() - 1);
        if (_path[_path.length() - 1] == '/')
            _path = _path.substring(0, _path.length() - 1);
    }
    ~AsyncSinglePageAppHandler()
    {
        delete this->_staticHandler;
    }

    virtual bool canHandle(AsyncWebServerRequest *request)
    {
        if (
            request->method() != HTTP_GET ||
            !request->url().startsWith(_uri) ||
            !request->isExpectedRequestedConnType(RCT_DEFAULT, RCT_HTTP))
        {
            return false;
        }

        if (this->_staticHandler->canHandle(request))
        {
            return true;
        }

        // real not found - serve the default file if exists
        if (_default_file.length() == 0)
            return false;

        File file;
        String path_file = _path;
        path_file += "/";
        path_file += _default_file;

        String path_gzip = path_file + ".gz";
        bool fileFound = false;
        bool gzipFound = false;

        // gzip first
        file = _fs.open(path_gzip, "r");
        gzipFound = FILE_IS_REAL(file);
        if (!gzipFound)
        {
            file = _fs.open(path_file, "r");
            fileFound = FILE_IS_REAL(file);
        }

        bool found = fileFound || gzipFound;
        if (!found)
        {
            return false;
        }

        //  keep file and file name in request
        size_t pathLen = path_file.length();
        char *_tempPath = (char *)malloc(pathLen + 1);
        snprintf(_tempPath, pathLen + 1, "%s", path_file.c_str());

        request->_tempObject = (void *)_tempPath;
        request->_tempFile = file;

        return true;
    }
    virtual void handleRequest(AsyncWebServerRequest *request)
    {
        return this->_staticHandler->handleRequest(request);
    }

    // forward methods to this->_staticHandler

    AsyncSinglePageAppHandler &setIsDir(bool isDir)
    {
        this->_staticHandler->setIsDir(isDir);
        return *this;
    }
    AsyncSinglePageAppHandler &setDefaultFile(const char *filename)
    {
        _default_file = String(filename);
        this->_staticHandler->setDefaultFile(filename);
        return *this;
    }
    AsyncSinglePageAppHandler &setCacheControl(const char *cache_control)
    {
        this->_staticHandler->setCacheControl(cache_control);
        return *this;
    }
    AsyncSinglePageAppHandler &setLastModified(const char *last_modified)
    {
        this->_staticHandler->setLastModified(last_modified);
        return *this;
    }
    AsyncSinglePageAppHandler &setLastModified(struct tm *last_modified)
    {
        this->_staticHandler->setLastModified(last_modified);
        return *this;
    }
#ifdef ESP8266
    AsyncSinglePageAppHandler &setLastModified(time_t last_modified)
    {
        this->_staticHandler->setLastModified(last_modified);
        return *this;
    }
    AsyncSinglePageAppHandler &setLastModified()
    {
        this->_staticHandler->setLastModified();
        return *this;
    }
#endif
    AsyncSinglePageAppHandler &setTemplateProcessor(AwsTemplateProcessor newCallback)
    {
        this->_staticHandler->setTemplateProcessor(newCallback);
        return *this;
    }
};