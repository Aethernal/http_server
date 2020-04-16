#include "fileExplorerService.h"

char* workSpacePath = ".";

char* getFullUri(char* uriPart)
{
    char* fullPath = malloc(2000);

    if(workSpacePath[strlen(workSpacePath) - 1] == '/')
        strncat(fullPath, workSpacePath, strlen(workSpacePath) - 1);
    else
        strcat(fullPath, workSpacePath);

    if(uriPart[strlen(uriPart) - 1] == '/')
        strncat(fullPath, uriPart, strlen(uriPart) - 1);
    else
        strcat(fullPath, uriPart);

    return fullPath;
}

enum pathType getServiceIsAvailable(char* uri)
{
    struct stat sb;

    if (strstr(uri,"..") != NULL)
        return isNothing;

    if(stat(uri, &sb) == 0)
    {
        if (S_ISDIR(sb.st_mode))
            return  isDirectory;
        if (S_ISREG(sb.st_mode))
            return isFile;
    }

    return isNothing;
}

char* getFileContent(char* uri)
{
    char* buffer = NULL;
    long size;

    FILE *fp;
    fp = fopen (uri, "r");
    if (fp != NULL)
    {
        fseek (fp, 0, SEEK_END);
        size = ftell (fp);
        fseek (fp, 0, SEEK_SET);
        buffer = malloc (size);
        if (buffer)
        {
            fread (buffer, 1, size, fp);
        }
        fclose (fp);
    }

    return buffer;
}

char* getFileName(char* uri)
{
    return strrchr(uri, '/');
}

char* getDirectoryContent(char* uri, char* uriPart)
{
    DIR* FD;
    char* listbuffer = NULL;
    struct dirent* in_file;
    FILE    *entry_file;


    if ((FD = opendir (uri)) == NULL)
       return listbuffer;

    const char list_structure[] = "<ul>\r\n" \
                    "%s\r\n" \
                    "<ul>\r\n";

    const char item_structure[] = "<li><a href=\"%s\">%s</a></li>\r\n";

    int total_char = 0;

    while ((in_file = readdir(FD)))
    {
       if (!strcmp (in_file->d_name, "."))
           continue;
       if (!strcmp (in_file->d_name, ".."))
           continue;

       char* linkString = malloc(strlen(in_file->d_name) + strlen(uriPart) + 1);
       strcat(linkString, uriPart);
       strcat(linkString, "/");
       strcat(linkString, in_file->d_name);

       unsigned int n = snprintf(NULL, 0, item_structure, linkString, in_file->d_name);
       total_char += (unsigned int)n;
       char* itemString = malloc(n);

       sprintf(itemString, item_structure, linkString, in_file->d_name);

       listbuffer = realloc(listbuffer, total_char);
       strcat(listbuffer, itemString);

       free(itemString);
    }

    unsigned int n = snprintf(NULL, 0, list_structure, listbuffer);
    char* finalString = malloc(n);
    sprintf(finalString, list_structure, listbuffer);

    free(listbuffer);
    closedir(FD);

    return  finalString;
}
