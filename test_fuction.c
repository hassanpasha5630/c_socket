#include <dirent.h> 
#include <stdio.h> 
int main(void) {
  DIR *d;
  struct dirent *dir;
  d = opendir(".");
  char *file_list[64];
  if (d) {
    int counter = 0 ;
    while ((dir = readdir(d)) != NULL) {
     //file_list[counter] = dir->d_name ;
     //counter ++;
     printf("%s\n", dir->d_name);
    }
    closedir(d);
  }
  for(int i = 0;i<11;i++){
   printf("%s\n", file_list[i]);
  }

  return(0);
}