#include "fileExplorerService.h"

char *workSpacePath = ".";

char *getFullUri(char *uriPart) {
    char *fullPath = malloc(2000);

    if (workSpacePath[strlen(workSpacePath) - 1] == '/')
        strncpy(fullPath, workSpacePath, strlen(workSpacePath) - 1);
    else
        strcpy(fullPath, workSpacePath);

    if (uriPart[strlen(uriPart) - 1] == '/')
        strncat(fullPath, uriPart, strlen(uriPart) - 1);
    else
        strcat(fullPath, uriPart);

    return fullPath;
}

enum pathType getServiceIsAvailable(char *uri) {
    struct stat sb;

    if (strstr(uri, "..") != NULL)
        return isNothing;

    if (stat(uri, &sb) == 0) {
        if (S_ISDIR(sb.st_mode))
            return isDirectory;
        if (S_ISREG(sb.st_mode))
            return isFile;
    }

    return isNothing;
}

char *getFileContent(char *uri) {
    char *buffer = NULL;
    long size;

    FILE *fp;
    fp = fopen(uri, "r");
    if (fp != NULL) {
        fseek(fp, 0, SEEK_END);
        size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        buffer = calloc(size, 1);
        if (buffer) {
            fread(buffer, 1, size, fp);
        }
        fclose(fp);
    }

    return buffer;
}

char *getFileName(char *uri) {
    return strrchr(uri, '/');
}

char *getDirectoryContent(char *local_path, char *uri) {
    DIR *FD;

    char *listbuffer = NULL;
    char *linkString = NULL;
    char *itemString = NULL;
    char *finalString = NULL;
    int n = 0;
    int total_char = 0;

    struct dirent *in_file;

    if ((FD = opendir(local_path)) == NULL)
        return listbuffer;

    const char list_structure[] = "<ul>\r\n" \
                    "%s\r\n" \
                    "<ul>\r\n";

    const char item_structure[] = "<li><a href=\"%s\">%s</a></li>\r\n";

    while ((in_file = readdir(FD))) {
        if (!strcmp(in_file->d_name, "."))
            continue;
        if (!strcmp(in_file->d_name, ".."))
            continue;

        if (strlen(uri) == 1 && uri[0] == '/') {
            linkString = calloc(strlen(in_file->d_name) + 1 + 1, 1);
            sprintf(linkString, "%s%s", "/", in_file->d_name);
        } else {
            linkString = calloc(strlen(in_file->d_name) + strlen(uri) + 1 + 1, 1);
            sprintf(linkString, "%s%s%s", uri, "/", in_file->d_name);
        }

        n = snprintf(NULL, 0, item_structure, linkString, in_file->d_name);
        total_char += (unsigned int) n;

        itemString = calloc(n + 1, 1);
        sprintf(itemString, item_structure, linkString, in_file->d_name);

        if (listbuffer == NULL) {
            listbuffer = calloc(total_char + 1, 1);
            strcpy(listbuffer, itemString);
        } else {
            char *tmp = realloc(listbuffer, total_char + 1);
            if (tmp == NULL) {
                free(listbuffer);
                free(itemString);
                free(linkString);
                return NULL;
            }
            listbuffer = tmp;

            sprintf(listbuffer, "%s%s", listbuffer, itemString);
        }

        free(itemString);

        free(linkString);
    }

    n = snprintf(NULL, 0, list_structure, listbuffer);
    finalString = calloc(n + 1, 1);
    sprintf(finalString, list_structure, listbuffer);

    free(listbuffer);
    closedir(FD);

    return finalString;
}
