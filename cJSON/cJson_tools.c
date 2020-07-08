#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <cjson/cJSON.h>
char *create_monitor(void)
{
    const unsigned int resolution_numbers[3][2] = {
        {1280, 720},
        {1920, 1080},
        {3840, 2160}
    };
    char *string = NULL;
    cJSON *name = NULL;
    cJSON *resolutions = NULL;
    cJSON *resolution = NULL;
    cJSON *width = NULL;
    cJSON *height = NULL;
    size_t index = 0;

    cJSON *monitor = cJSON_CreateObject();
    if (monitor == NULL)
    {
        goto end;
    }

    name = cJSON_CreateString("Awesome 4K");
    if (name == NULL)
    {
        goto end;
    }
    /* after creation was successful, immediately add it to the monitor,
     * thereby transferring ownership of the pointer to it */
    cJSON_AddItemToObject(monitor, "name", name);

    resolutions = cJSON_CreateArray();
    if (resolutions == NULL)
    {
        goto end;
    }
    cJSON_AddItemToObject(monitor, "resolutions", resolutions);

    for (index = 0; index < (sizeof(resolution_numbers) / (2 * sizeof(int))); ++index)
    {
        resolution = cJSON_CreateObject();
        if (resolution == NULL)
        {
            goto end;
        }
        cJSON_AddItemToArray(resolutions, resolution);

        width = cJSON_CreateNumber(resolution_numbers[index][0]);
        if (width == NULL)
        {
            goto end;
        }
        cJSON_AddItemToObject(resolution, "width", width);

        height = cJSON_CreateNumber(resolution_numbers[index][1]);
        if (height == NULL)
        {
            goto end;
        }
        cJSON_AddItemToObject(resolution, "height", height);
    }
    cJSON *age = cJSON_CreateNumber(29);
    cJSON_AddItemToObject(monitor,"age",age);
    string = cJSON_Print(monitor);
    if (string == NULL)
    {
        fprintf(stderr, "Failed to print monitor.\n");
    }

end:
    cJSON_Delete(monitor);
    return string;
}

void testParseJson()
{
  FILE *fp = NULL;
  fp = fopen("test.txt","r");
  if (fp == NULL) {
      return;
  }
  char buf[256] = {0};
  char jsonInfo[1024] = {0};
  int len = 0;
  while(fgets(buf,256,fp) != NULL) {
      snprintf(jsonInfo+len,sizeof(jsonInfo),"%s",buf);
      len += strlen(buf);
      memset(buf,0,sizeof(buf));
  }

  fclose(fp);
 // printf("%s\n",jsonInfo);
  cJSON *info = cJSON_Parse(jsonInfo);
  if(info == NULL) {
 	return;
  }

  cJSON *name = NULL;
  cJSON *resolutions = NULL;
  int i = 0;
  name = cJSON_GetObjectItemCaseSensitive(info,"name");
  if (cJSON_IsString(name) && name->valuestring != NULL) {
     printf("checking Json %s\n",name->valuestring);
  }
  resolutions = cJSON_GetObjectItem(info,"resolutions");
  if(cJSON_IsArray(resolutions)) {
     printf("is array\n");
     printf("size = %d\n",cJSON_GetArraySize(resolutions));
     for(i = 0; i < cJSON_GetArraySize(resolutions); i++) {
          cJSON *number = cJSON_GetArrayItem(resolutions, i);	
       if(cJSON_IsNumber(number)) {
           printf("resolutions[%d] = %d\n",i, number->valueint);
       } else if(cJSON_IsObject(number)) {
            cJSON *width = cJSON_GetObjectItem(number,"width");    
            cJSON *height = cJSON_GetObjectItem(number,"height");    
            if(cJSON_IsNumber(width) && cJSON_IsNumber(height)) {
		printf("width=%d\n",width->valueint);
		printf("height=%d\n",height->valueint);
            }
       }

    } 
  }
  cJSON *age = cJSON_GetObjectItem(info,"age");
  if(cJSON_IsNumber(age)){
	printf("age = %d\n",age->valueint);
  }
     cJSON_Delete(info);
}

int main(int argc, char **argv)
{

    char *string = NULL;
    string =  create_monitor();
    if(string) {
        //printf("%s\n",string);
	free(string);
    }
    testParseJson();
    return 0;
}

