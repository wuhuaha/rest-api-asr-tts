#include <memory.h>
#include <time.h>
#include <sys/time.h>
#include <curl/curl.h>
#include <stdlib.h>

#include "common.h"
#include "asrmain.h"
#include "token.h"

const char ASR_SCOPE[] = "audio_voice_assistant_get";
const char API_ASR_URL[] = "http://vop.baidu.com/server_api"; // 可改为https
const char TTS_SCOPE[] = "audio_tts_post";
const char API_TTS_URL[] = "http://tsn.baidu.com/text2audio"; // 可改为https
const char API_ASR_ALI_URL[] = "http://nls-gateway.cn-shanghai.aliyuncs.com/stream/v1/asr"; //阿里asr的url
const char API_TTS_ALI_URL[] = "https://nls-gateway.cn-shanghai.aliyuncs.com/stream/v1/tts"; //阿里tts的url
const char ALI_PERSON[][8]={
							"Siyue",     //温柔女声	客服场景
							"Sijing",	 //严厉女声	客服场景
							"Sijia",	//标准女声	通用场景
							"Sicheng",	//标准男声	通用场景
							"Xiaomeng",	//标准女声	通用场景
							"Xiaowei"	//标准男声	通用场景
							};

#ifndef SAFE_FREE
#define SAFE_FREE(ptr) \
{ \
    if (NULL != ptr) { \
        free(ptr); \
        ptr = NULL; \
    } \
}
#endif

/*
*@Destription:malloc a string ,and copy src to it
*@param:src;
*@return: success:dst  faild:NULL
*/
char *string_malloc_copy(char *src)
{
    register int i;
    if(src == NULL){
        //printf("src string is NULL\n");
        return NULL;
    }
    int length = strlen(src);
    char *dst = (char *)malloc(length + 1);
    if(dst == NULL){
        printf("MALLOC FAILD ~~ \n");
        return NULL;
    }
    for(i = 0; i < length; i++)
    {
        dst[i] = *src++;
    }
    dst[i] = 0;
    return dst;    
}
/*return  n ms*/
static int timeval_sub(struct timeval t1, struct timeval t2)
{
    return ((t1.tv_sec - t2.tv_sec) * 1000000 + t1.tv_usec - t2.tv_usec) / 1000;
}
// 从fp读取文件内容，malloc出相应的空间进行保存并返回空间的指针，长度保存在content_len
char *read_file_data(FILE *fp, int *content_len) {
    fseek(fp, 0, SEEK_END);
    int len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    *content_len = len;
    char *data = (char *) malloc(len);
    if (data == NULL) {
        fprintf(stderr, "malloc size %d failed", len);
        exit(11);
    }
    fread(data, 1, len, fp);
    return data;
}

/*
*@Description asr config init function
*@param in tts_config
*@param in api_key
*@param in secret_key
*@param in format  //文件类型，如pcm wav
*@param in rate  //采样率
*@param in dev_pid  //使用输入法模型。1536表示识别普通话，1737	英语, 1936	普通话	远场模型, ....... 更多详见 http://ai.baidu.com/docs#/ASR-Linux-SDK/top
*/
RETURN_CODE init_baidu_asr_config(struct asr_config *config, char *api_key, char *secret_key, char *format, int rate, int dev_pid) {

    if(api_key == NULL)	config->api_key = string_malloc_copy("4E1BG9lTnlSeIf1NQFlrSq6h");
	    else	config->api_key = string_malloc_copy(api_key);

	if(secret_key == NULL) config->secret_key = string_malloc_copy("544ca4657ba8002e3dea3ac2f5fdd241");
		else	config->secret_key = string_malloc_copy(secret_key);

	if(format == NULL)	config->format = string_malloc_copy("pcm");
		else	config->format = string_malloc_copy(format);

	if(rate!= 8000 && rate != 16000)	config->rate = 8000;
		else	config->rate = rate;
	
	if(dev_pid != 0)	config->dev_pid = dev_pid;
		else config->dev_pid = 1537;  //1537 表示识别普通话，使用输入法模型。1536表示识别普通话，使用搜索模型 其它语种参见文档

	*(config->beebot_token) = *(config->token) = *(config->cuid) = 0;

	RETURN_CODE res_beebot_token = beebot_get_token(config->beebot_token, config->cuid);

	if(res_beebot_token != RETURN_OK){
		printf("get token from beebot error!");
	}

	RETURN_CODE res_token = speech_get_token(config->api_key, config->secret_key, ASR_SCOPE, config->token);

	if(*(config->cuid) == 0)
		snprintf(config->cuid, sizeof(config->cuid), "1234567C");

	if((res_beebot_token && res_token) == RETURN_OK)
    	return RETURN_OK;
	else
		return ERROR_ASR_CONFIG_INIT;
}
/*
*@Description tts config init function
*@param in tts_config
*@param in api_key
*@param in secret_key
*@param in person;	// 发音人选择, 0为""，1为普通男生，3为情感合成-度逍遥，4为情感合成-度丫丫，默认为普通女声
*@param in speed;	// 语速，取值0-9，默认为5中语速
*@param in pitch;	// 音调，取值0-9，默认为5中语调
*@param in volume;	// #音量，取值0-9，默认为5中音量
*@param in aue;	 	// 下载的文件格式, 3：mp3(default) 4： pcm-16k 5： pcm-8k 6. wav-16k
*@return true: RETURN_OK
*/
RETURN_CODE init_baidu_tts_config(struct tts_config *config, char *api_key, char *secret_key, int person, int speed, int pitch, int volume, int aue) 
{
 
    // 将上述参数填入config中
    if(api_key == NULL)	config->api_key = string_malloc_copy("4E1BG9lTnlSeIf1NQFlrSq6h");
	    else	config->api_key = string_malloc_copy(api_key);

	if(secret_key == NULL) config->secret_key = string_malloc_copy("544ca4657ba8002e3dea3ac2f5fdd241");
		else	config->secret_key = string_malloc_copy(secret_key);

	*(config->beebot_token) = *(config->token) = *(config->cuid) = 0;

	RETURN_CODE res_beebot_token = beebot_get_token(config->beebot_token, config->cuid);

	if(res_beebot_token != RETURN_OK){
		printf("get token from beebot error!");
	}

	RETURN_CODE res_token = speech_get_token(config->api_key, config->secret_key, TTS_SCOPE, config->token);

	if(*(config->cuid) == 0)
		snprintf(config->cuid, sizeof(config->cuid), "1234567C");
    
    config->per = person;
    config->spd = speed;
    config->pit = pitch;
    config->vol = volume;
	config->aue = aue;
	// 下载的文件格式, 3：mp3-16k 4： pcm-16k 5： pcm-8k 6. wav-16k
	config->sample_rate = 16000;
	if((config->aue == 5))
		config->sample_rate = 8000;
	// aue对应的格式，format
	const char formats[4][4] = {"mp3", "pcm", "pcm", "wav"};
	snprintf(config->format, sizeof(config->format), formats[aue - 3]);
	
    return RETURN_OK;
}

/*
*@Description ali asr config init function
*@param in tts_config
*@param in appkey
*@param in token
*@param in format  //文件类型，如pcm wav
*@param in rate  //采样率
*/
RETURN_CODE init_ali_asr_config(struct asr_config *config, char *appkey, char *token, char *format, int rate) {


	if((token==NULL)||(*token==0)){
		printf("error token!");
		return ERROR_ASR_CONFIG_INIT;
	}

	config->api_key = config->secret_key = config->format =NULL;
	*(config->beebot_token) = *(config->token) = *(config->cuid) = *(config->appkey) = 0;

	if(appkey != NULL)	sprintf(config->appkey, "%s", appkey);

	if(format == NULL)	config->format = string_malloc_copy("pcm");
		else	config->format = string_malloc_copy(format);

	if(rate!= 8000 && rate != 16000)	config->rate = 8000;
		else	config->rate = rate;

	sprintf(config->token, "%s", token);

    return RETURN_OK;
}
/*
*@Description tts config init function
*@param in tts_config
*@param in appkey
*@param in token
*@param in person;	// 发音人选择： 0 —— 温柔女声	客服场景
*@									1 —— 严厉女声	客服场景
*@									2 —— 标准女声	通用场景
*@									3 —— 标准男声	通用场景
*@									4 —— 标准女声	通用场景
*@									5 —— 标准男声	通用场景"
*@param in speed;	// 语速，取值0-9，默认为5中语速
*@param in pitch;	// 音调，取值0-9，默认为5中语调
*@param in volume;	// #音量，取值0-9，默认为5中音量
*@param in aue;	 	// 下载的文件格式, 3：mp3-16k 4： pcm-16k 5： pcm-8k 6. wav-16k  7. wav-8k
*@return true: RETURN_OK
*/
RETURN_CODE init_ali_tts_config(struct tts_config *config, char *appkey, char *token, int person, int speed, int pitch, int volume, int aue) 
{
 
    // 将上述参数填入config中
	if((token==NULL)||(*token==0)){
		printf("error token!");
		return ERROR_ASR_CONFIG_INIT;
	}

	config->api_key = config->secret_key = NULL;
	*(config->beebot_token) = *(config->token) = *(config->cuid) = *(config->appkey) = 0;

	if(appkey != NULL)	sprintf(config->appkey, "%s", appkey);
	
	sprintf(config->token, "%s", token);
    
    config->per = person;
    config->spd = speed*10;
    config->pit = pitch*100-500;
    config->vol = volume*10;
	config->aue = aue;
	// 下载的文件格式, 3：mp3-16k 4： pcm-16k 5： pcm-8k 6. wav-16k  7. wav-8k
	config->sample_rate = 8000;
	if((config->aue == 3)||(config->aue == 4)||(config->aue == 6))
		config->sample_rate = 16000;	
	
	// aue对应的格式，format
	const char formats[5][4] = {"mp3", "pcm", "pcm", "wav", "wav"};
	snprintf(config->format, sizeof(config->format), formats[aue - 3]);
	
    return RETURN_OK;
}

//asr_config  反初始化  一定要调用 否则会内存泄露
void uninit_asr_config(struct asr_config *config)
{
	SAFE_FREE(config->api_key);
	SAFE_FREE(config->secret_key);
	SAFE_FREE(config->format);
}

//tts_config  反初始化  一定要调用 否则会内存泄露
void uninit_tts_config(struct tts_config *config)
{
	SAFE_FREE(config->api_key);
	SAFE_FREE(config->secret_key);
}

/*
*@Description:初始化asr任务
*@param: in asr_task_t
*@param: in file_name : 要识别的文件名
*@return: true:RETURN_OK  fail: ERROR_ASR_TASK_INIT
*/
RETURN_CODE init_asr_task(asr_task_t p_task, char* file_name)
{
	if(p_task == NULL){
		printf("asr_task is NULL! you must new a object to init ~");
		return ERROR_ASR_TASK_INIT;
	}
	if(file_name == NULL){
		printf("file name is NULL!");
		return ERROR_ASR_TASK_INIT;
	}
	if(*file_name == 0){
		printf("length of file name is 0!");
		return ERROR_ASR_TASK_INIT;
	}
	snprintf(p_task->file_name , sizeof(p_task->file_name), "%s", file_name);

	return RETURN_OK; 
	
}

/*
*@Description:初始化tts任务
*@param: in tts_task_t
*@param: in text : 要合成语音的内容字符串
*@param: in result_audio_name : 要保存到的文件名
*@return: true:RETURN_OK  fail: ERROR_TTS_TASK_INIT
*/

RETURN_CODE init_tts_task(tts_task_t p_task, char* text, char* result_audio_name)
{
	if(p_task == NULL){
		printf("tts_task is NULL! you must new a object to init ~");
		return ERROR_TTS_TASK_INIT;
	}
	if(result_audio_name == NULL){
		printf("result_audio_name is NULL!");
		return ERROR_TTS_TASK_INIT;
	}
	if(text == NULL){
		printf("text is NULL!");
		return ERROR_TTS_TASK_INIT;
	}
	if(*result_audio_name == 0){
		printf("length of result_audio_name is 0!");
		return ERROR_TTS_TASK_INIT;
	}
	if(*text == 0){
		printf("length of text is 0!");
		return ERROR_TTS_TASK_INIT;
	}
	snprintf(p_task->file_name , sizeof(p_task->file_name), "%s", result_audio_name);

	snprintf(p_task->text , sizeof(p_task->text), "%s", text);

	return RETURN_OK; 
	
}

// asr_task 反初始化  暂时没用
RETURN_CODE unint_asr_task(asr_task_t p_task)
{
	return RETURN_OK;
}

//tts_task 反初始化  暂时没用
RETURN_CODE unint_tts_task(tts_task_t p_task)
{
	return RETURN_OK;
}

// 调用百度语音识别接口
RETURN_CODE baidu_asr(struct asr_config *config, asr_task_t p_task) {
    char url[300];

    gettimeofday(&(p_task->asr_start_time), NULL);

	FILE *fp = fopen(p_task->file_name, "r");
    if (fp == NULL) {
        //文件不存在
        snprintf(g_demo_error_msg, BUFFER_ERROR_SIZE,
                 "current running directory does not contain file %s", p_task->file_name);
        return ERROR_ASR_FILE_NOT_EXIST;
    }

    CURL *curl = curl_easy_init(); // 需要释放
    char *cuid = curl_easy_escape(curl, config->cuid, strlen(config->cuid)); // 需要释放

	if(*(config->beebot_token) != 0){
		snprintf(url, sizeof(url), "%s?cuid=%s&token=%s&dev_pid=%d",
             API_ASR_URL, cuid, config->beebot_token, config->dev_pid);
	}else{
    	snprintf(url, sizeof(url), "%s?cuid=%s&token=%s&dev_pid=%d",
             API_ASR_URL, cuid, config->token, config->dev_pid);
	}
    SAFE_FREE(cuid);
    printf("request url :%s\n", url);

    struct curl_slist *headerlist = NULL;
    char header[50];
    snprintf(header, sizeof(header), "Content-Type: audio/%s; rate=%d", config->format,
             config->rate);
    headerlist = curl_slist_append(headerlist, header); // 需要释放

    int content_len = 0;
    char *result = NULL;
    char *audio_data = read_file_data(fp, &content_len); // 读取文件， 需要释放
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10); // 连接5s超时
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 100); // 整体请求60s超时
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist); // 添加http header Content-Type
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, audio_data); // 音频数据
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, content_len); // 音频数据长度
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);  // 需要释放

    CURLcode res_curl = curl_easy_perform(curl);
    RETURN_CODE res = RETURN_OK;
    if (res_curl != CURLE_OK) {
        // curl 失败
        snprintf(g_demo_error_msg, BUFFER_ERROR_SIZE, "perform curl error:%d, %s.\n", res,
                 curl_easy_strerror(res_curl));
        res = ERROR_ASR_CURL;
    } else {
        printf("YOUR FINAL RESULT: %s\n", result);
		snprintf(p_task->result_string, sizeof(p_task->result_string), "%s", result);
    }
    
    gettimeofday(&(p_task->asr_end_time), NULL);
    printf("asr used time:%d\n", timeval_sub(p_task->asr_end_time, p_task->asr_start_time));

    curl_slist_free_all(headerlist);
    free(audio_data);
    free(result);
    curl_easy_cleanup(curl);
    return res;
}

// 调用阿里语音识别接口
RETURN_CODE ali_asr_v2(struct asr_config *config, asr_task_t p_task) {
    char url[300];

    gettimeofday(&(p_task->asr_start_time), NULL);

	FILE *fp = fopen(p_task->file_name, "r");
    if (fp == NULL) {
        //文件不存在
        snprintf(g_demo_error_msg, BUFFER_ERROR_SIZE,
                 "current running directory does not contain file %s", p_task->file_name);
        return ERROR_ASR_FILE_NOT_EXIST;
    }

    CURL *curl = curl_easy_init(); // 需要释放
    
    snprintf(url, sizeof(url), "%s?appkey=%s&format=%s&sample_rate=%d",
             API_ASR_ALI_URL, config->appkey, config->format, config->rate);
	
    printf("request url :%s\n", url);

    struct curl_slist *headerlist = NULL;
    char header[50];
    snprintf(header, sizeof(header), "X-NLS-Token:%s", config->token);
    headerlist = curl_slist_append(headerlist, header); // 需要释放

    int content_len = 0;
    char *result = NULL;
    char *audio_data = read_file_data(fp, &content_len); // 读取文件， 需要释放
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10); // 连接5s超时
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 100); // 整体请求60s超时
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist); // 添加http header Content-Type
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, audio_data); // 音频数据
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, content_len); // 音频数据长度
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);  // 需要释放

    CURLcode res_curl = curl_easy_perform(curl);
    RETURN_CODE res = RETURN_OK;
    if (res_curl != CURLE_OK) {
        // curl 失败
        snprintf(g_demo_error_msg, BUFFER_ERROR_SIZE, "perform curl error:%d, %s.\n", res,
                 curl_easy_strerror(res_curl));
        res = ERROR_ASR_CURL;
    } else {
        printf("YOUR FINAL RESULT: %s\n", result);
		snprintf(p_task->result_string, sizeof(p_task->result_string), "%s", result);
    }
    
    gettimeofday(&(p_task->asr_end_time), NULL);
    printf("asr used time:%d\n", timeval_sub(p_task->asr_end_time, p_task->asr_start_time));

    curl_slist_free_all(headerlist);
    free(audio_data);
    free(result);
    curl_easy_cleanup(curl);
    return res;
}


//调用百度tts接口
RETURN_CODE baidu_tts(struct tts_config *config, tts_task_t p_task) 
{
	gettimeofday(&(p_task->tts_start_time), NULL);

    char params[200 + strlen(p_task->text) * 9];
    CURL *curl = curl_easy_init(); // 需要释放
    char *cuid = curl_easy_escape(curl, config->cuid, strlen(config->cuid)); // 需要释放
    char *textemp = curl_easy_escape(curl, p_task->text, strlen(p_task->text)); // 需要释放
	char *tex = curl_easy_escape(curl, textemp, strlen(textemp)); // 需要释放
	curl_free(textemp);
	char params_pattern[] = "ctp=1&lan=zh&cuid=%s&tok=%s&tex=%s&per=%d&spd=%d&pit=%d&vol=%d&aue=%d";
    snprintf(params, sizeof(params), params_pattern , cuid, config->beebot_token, tex,
             config->per, config->spd, config->pit, config->vol, config->aue);
			 
	char url[sizeof(params) + 200];
	snprintf(url, sizeof(url), "%s?%s", API_TTS_URL, params);
    printf("test in browser: %s\n", url);
    curl_free(cuid);
  	curl_free(tex);
	
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, params);
    curl_easy_setopt(curl, CURLOPT_URL, API_TTS_URL);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10); // 连接5s超时
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 100); // 整体请求60s超时
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback); // 检查头部
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, p_task);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, p_task);  // 需要释放
    curl_easy_setopt(curl, CURLOPT_VERBOSE, ENABLE_CURL_VERBOSE);
    CURLcode res_curl = curl_easy_perform(curl);

    RETURN_CODE res = RETURN_OK;
    if (res_curl != CURLE_OK) {
        // curl 失败
        snprintf(g_demo_error_msg, BUFFER_ERROR_SIZE, "perform curl error:%d, %s.\n", res,
                 curl_easy_strerror(res_curl));
        res = ERROR_TTS_CURL;
    }
	gettimeofday(&(p_task->tts_end_time), NULL);
	if (p_task->fp != NULL) {
		fclose(p_task->fp);
	}
    curl_easy_cleanup(curl);
    return res;
}
//调用阿里tts接口
RETURN_CODE ali_tts(struct tts_config *config, tts_task_t p_task) 
{
	gettimeofday(&(p_task->tts_start_time), NULL);

	//person:

    char params[200 + strlen(p_task->text) * 9];
    CURL *curl = curl_easy_init(); // 需要释放
	char *textemp = curl_easy_escape(curl, p_task->text, strlen(p_task->text)); // 需要释放
    char params_pattern[] = "appkey=%s&token=%s&text=%s&format=%s&sample_rate=%d&voice=%s&volume=%d&speech_rate=%d&pitch_rate=%d";
    snprintf(params, sizeof(params), params_pattern , config->appkey, config->token, textemp, config->format, config->sample_rate,
    				ALI_PERSON[config->per], config->vol, config->spd, config->pit);
    //snprintf(params, sizeof(params), "appkey=%s&token=%s&text=%s&format=%s&sample_rate=%d", config->appkey, config->token, tex, config->format, sample_rate);
    
    curl_free(textemp); 
    char url[sizeof(params) + 200];
    snprintf(url, sizeof(url), "%s?%s", API_TTS_ALI_URL, params);

    	
    printf("test in browser: %s\n", url);
	
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10); // 连接5s超时
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 100); // 整体请求60s超时
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback); // 检查头部
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, p_task);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, p_task);  // 需要释放
    CURLcode res_curl = curl_easy_perform(curl);

    RETURN_CODE res = RETURN_OK;
    if (res_curl != CURLE_OK) {
        // curl 失败
        snprintf(g_demo_error_msg, BUFFER_ERROR_SIZE, "perform curl error:%d, %s.\n", res,
                 curl_easy_strerror(res_curl));
        res = ERROR_TTS_CURL;
    }
	gettimeofday(&(p_task->tts_end_time), NULL);
	if (p_task->fp != NULL) {
		fclose(p_task->fp);
	}
    curl_easy_cleanup(curl);
    return res;
}


int main() {

    curl_global_init(CURL_GLOBAL_ALL);//注意：这个函数非线程安全，应该在主线程中调用
	RETURN_CODE rescode ;
	
	//test_asr
	struct asr_config config_asr = {0};
	asr_task task_asr = {0};
	init_baidu_asr_config(&config_asr, NULL, NULL, NULL, 16000, 1537);
	init_asr_task(&task_asr, "16k_test.pcm");
	rescode = baidu_asr(&config_asr, &task_asr);
	printf("task_asr.result_string:%s\n",task_asr.result_string);
	unint_asr_task(&task_asr);
	uninit_asr_config(&config_asr);
	//test_asr_ali
	struct asr_config config_asr_ali = {0};
	asr_task task_asr_ali = {0};
	init_ali_asr_config(&config_asr_ali, "OaAeAZAaNMidbjEs", "e973703b0dad4fb792df488061d6135b", "pcm", 16000);
	init_asr_task(&task_asr_ali, "16k_test_2.pcm");
	rescode = ali_asr_v2(&config_asr_ali, &task_asr_ali);
	printf("task_asr.result_string:%s\n",task_asr_ali.result_string);
	unint_asr_task(&task_asr_ali);
	uninit_asr_config(&config_asr_ali);
	//test tts
	struct tts_config config_tts = {0};
	tts_task task_tts = {0};
	init_baidu_tts_config(&config_tts, NULL, NULL, 4, 5, 5, 5, 6);
	init_tts_task(&task_tts, "测试一下看看好不好用", "tts_test.wav");
	rescode = baidu_tts(&config_tts, &task_tts);
	unint_tts_task(&task_tts);
	uninit_tts_config(&config_tts);
	//test ali tts
	struct tts_config config_ali_tts = {0};
	tts_task task_ali_tts = {0};
	init_ali_tts_config(&config_ali_tts, "OaAeAZAaNMidbjEs", "e973703b0dad4fb792df488061d6135b", 0, 5, 5, 5, 7);
	init_tts_task(&task_ali_tts, "测试一下看看好不好用", "tts_ali_test.wav");
	rescode = ali_tts(&config_ali_tts, &task_ali_tts);
	unint_tts_task(&task_ali_tts);
	uninit_tts_config(&config_ali_tts);
	
    curl_global_cleanup();	//注意：这个函数非线程安全，应该在主线程中调用
    
    if (rescode != RETURN_OK) {
        fprintf(stderr, "ERROR: %s, %d", g_demo_error_msg, rescode);
    }
    return rescode;
}
