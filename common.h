//
// Created by fu on 3/2/18.
//

#ifndef SPEECH_C_DEMO_COMMON_H
#define SPEECH_C_DEMO_COMMON_H

#include  "asrmain.h"

/**
 * @see libcurl CURLOPT_WRITEFUNCTION
 *
 * @brief curl回调，http请求的结果在result中，注意需要释放free(*result);
 * @param ptr
 * @param size
 * @param nmemb
 * @param result 传入时必需是NULL， 使用后自行释放
 * @return
 */
size_t writefunc(void *ptr, size_t size, size_t nmemb, char **result);

size_t writefunc_data(void *ptr, size_t size, size_t nmemb, tts_task_t result);

size_t header_callback(char *buffer, size_t size, size_t nitems, tts_task_t result);


#endif //SPEECH_C_DEMO_COMMON_H
