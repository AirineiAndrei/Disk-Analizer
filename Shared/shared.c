#include "shared.h"

void add_to_path(const char* current_path,const char* add_path,char *ans)
{
    strcpy(ans,current_path);
    if (ans[strlen(ans) - 1] != '/') {
        strcat(ans, "/");
    }
    strcat(ans,add_path);
}