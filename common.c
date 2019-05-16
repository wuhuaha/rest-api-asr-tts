#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include "asrmain.h"

const int BUFFER_ERROR_SIZE = 1024;
char g_demo_error_msg[1024] = {0};
const int ENABLE_CURL_VERBOSE = 0;
const int MAX_HEADER_VALUE_LEN = 100;


//asr的回调函数，将接收到的内容写入result中
size_t writefunc(void *ptr, size_t size, size_t nmemb, char **result) {
    size_t result_len = size * nmemb;
    if (*result == NULL) {
        *result = (char *) malloc(result_len + 1);
    } else {
        *result = (char *) realloc(*result, result_len + 1);
    }
    if (*result == NULL) {
        printf("realloc failure!\n");
        return 1;
    }
    memcpy(*result, ptr, result_len);
    (*result)[result_len] = '\0';
    // printf("buffer: %s\n", *result);
    return result_len;
}

//忽略大小写的检查buffer中的前几个字符是否与key相同；检查buffer中剩余的文件长度是否超过限制；两者都通过检查则返回RETURN_OK， 否则返回RETURN_ERROR
static RETURN_CODE search_header(const char *buffer, size_t len, const char *key, char *value) {
    size_t len_key = strlen(key);
    char header_key[len_key + 1];
    header_key[len_key] = '\0';
    memcpy(header_key, buffer, len_key);
    if (strcasecmp(key, header_key) == 0 && (len - len_key) < MAX_HEADER_VALUE_LEN) {
        int len_value = len - len_key;
        value[len_value - 1] = '\0';
        memcpy(value, buffer + len_key + 1, len_value);
        return RETURN_OK;
    }
    return RETURN_ERROR;

}

size_t header_callback(char *buffer, size_t size, size_t nitems, tts_task_t result) {
    size_t len = size * nitems;
    char key[] = "Content-Type";
    char value[MAX_HEADER_VALUE_LEN];
    if (search_header(buffer, len, key, value) == RETURN_OK) {
        if (strstr(value, "audio/") != NULL) {
            result->result = RETURN_OK;
        } else {
            fprintf(stderr, "Server return ERROR, %s : %s\n%s\n", key, value, buffer);

        }
    }
    return len;
}

size_t writefunc_data(void *ptr, size_t size, size_t nmemb, tts_task_t result) {
    //printf("file_name:%s\n",result->file_name);
    if (result->fp == NULL) {
        result->fp = fopen(result->file_name, "w+");
        if (result->fp == NULL) {
            snprintf(g_demo_error_msg, BUFFER_ERROR_SIZE, "%s cannot be opened\n", result->file_name);
            return 0;
        }
        printf("Data will be write into %s in the current directory\n", result->file_name);
    }
    return fwrite(ptr, size, nmemb, result->fp);
}

