#include "fileExplorerService.h"

char* workSpacePath = ".";

enum pathType getServiceIsAvailable(char* uri)
{
    char fullPath[2000] = "";

    if(workSpacePath[strlen(workSpacePath) - 1] == '/')
        strncat(fullPath, workSpacePath, strlen(workSpacePath) - 1);
    else
        strcat(fullPath, workSpacePath);

    strcat(fullPath, uri);

    logger_info("FILE", "%s", &fullPath);

    struct stat sb;

    if (strstr(fullPath,"..") != NULL)
        return isNothing;

    if(stat(fullPath, &sb) == 0)
    {
        if (S_ISDIR(sb.st_mode))
            return  isDirectory;
        if (S_ISREG(sb.st_mode))
            return isFile;
    }

    return isNothing;
}
