#ifndef SPEECH_C_DEMO_ASRMAIN_H
#define SPEECH_C_DEMO_ASRMAIN_H

#include <stdlib.h>
#include <stdio.h>

/**
 * @brief 函数返回值定义
 */
typedef enum RETURN_CODE {
    RETURN_OK = 0, // 返回正常
    RETURN_ERROR = 1, // 返回错误
    ERROR_ASR_CONFIG_INIT = 10, // asr config init error
    ERROR_TTS_CONFIG_INIT = 11, // tts config init error
    ERROR_ASR_TASK_INIT = 12, // asr task init error
    ERROR_TTS_TASK_INIT = 13, // tts task init error
    ERROR_TOKEN_CURL = 23, // TOKEN CURL 调用错误
    ERROR_TTS_CURL = 24, // TTS CURL 调用错误
    ERROR_TOKEN_PARSE_ACCESS_TOKEN = 25,  // access_token字段在返回结果中不存在
    ERROR_TOKEN_PARSE_SCOPE = 26, // 解析scope字段，或者scope不存在
    ERROR_ASR_FILE_NOT_EXIST = 101, // 本地文件不存在
    ERROR_ASR_CURL = 102 // 识别 curl 错误
} RETURN_CODE;

/**
 * @brief 全局的报错信息，遇见报错，代码应该停止
 */
extern const int BUFFER_ERROR_SIZE;
extern const int ENABLE_CURL_VERBOSE;
extern char g_demo_error_msg[];

struct asr_config {
    char *api_key; // 填写网页上申请的appkey 如 $apiKey="g8eBUMSokVB1BHGmgxxxxxx"
    char *secret_key; // 填写网页上申请的APP SECRET 如 $secretKey="94dc99566550d87f8fa8ece112xxxxx"
    char appkey[32];//阿里的appkey
    char cuid[32];
	char token[128];
	char beebot_token[128];
    char *format;
    int rate;
    int dev_pid;
};

struct tts_config {
    char *api_key; // 填写网页上申请的appkey 如 $apiKey="g8eBUMSokVB1BHGmgxxxxxx"
    char *secret_key; // 填写网页上申请的APP SECRET 如 $secretKey="94dc99566550d87f8fa8ece112xxxxx"
    char appkey[32];//阿里的appkey
    char cuid[32];
	char token[128];
	char beebot_token[128];
    int spd;	// 语速，取值0-9，默认为5中语速
    int pit;	// 音调，取值0-9，默认为5中语调
    int vol;	// #音量，取值0-9，默认为5中音量
    int sample_rate; //采样率
    int per;	// 发音人选择, 如果为百度则：0为普通女声，1为普通男生，3为情感合成-度逍遥，4为情感合成-度丫丫，默认为普通女声
    			//如果为阿里则： 0 —— 温柔女声	客服场景，1 —— 严厉女声	客服场景，2 —— 标准女声	通用场景，3 —— 标准男声	通用场景，4 —— 标准女声	通用场景，5 —— 标准男声	通用场景"
	int aue;	// 下载的文件格式, 下载的文件格式, 通用的——3：mp3-16k 4： pcm-16k 5： pcm-8k 6. wav-16k 阿里另外的：7. wav-8k
	char format[4];
};

typedef struct {
	char file_name[2048];
	struct timeval asr_start_time;
	struct timeval asr_end_time;
	RETURN_CODE result;
	char result_string[2048];
	char result_word[2048];
} asr_task;

typedef asr_task * asr_task_t;

typedef struct {
	char file_name[512 * 3 + 4];
	char text[512 * 3 + 1];
	struct timeval tts_start_time;
	struct timeval tts_end_time;
	RETURN_CODE result;
	FILE *fp; // 保存结果的文件
} tts_task;

typedef tts_task * tts_task_t;

extern const char ASR_SCOPE[];

/*
*@Description asr config init function
*@param in tts_config
*@param in api_key
*@param in secret_key
*@param in format  //文件类型，如pcm wav
*@param in rate  //采样率
*@param in dev_pid  //使用输入法模型。1536表示识别普通话，1737	英语, 1936	普通话	远场模型, ....... 更多详见 http://ai.baidu.com/docs#/ASR-Linux-SDK/top
*/
RETURN_CODE init_baidu_asr_config(struct asr_config *config, char *api_key, char *secret_key, char *format, int rate, int dev_pid);
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
RETURN_CODE init_baidu_tts_config(struct tts_config *config, char *api_key, char *secret_key, int person, int speed, int pitch, int volume, int aue);
/*
*@Description ali asr config init function
*@param in tts_config
*@param in appkey
*@param in token
*@param in format  //文件类型，如pcm wav
*@param in rate  //采样率
*/
RETURN_CODE init_ali_asr_config(struct asr_config *config, char *appkey, char *token, char *format, int rate);
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
RETURN_CODE init_ali_tts_config(struct tts_config *config, char *appkey, char *token, int person, int speed, int pitch, int volume, int aue);
//asr_config  反初始化  一定要调用 否则会内存泄露
void uninit_asr_config(struct asr_config *config);
//tts_config  反初始化  一定要调用 否则会内存泄露
void uninit_tts_config(struct tts_config *config);
/*
*@Description:初始化asr任务
*@param: in asr_task_t
*@param: in file_name : 要识别的文件名
*@return: true:RETURN_OK  fail: ERROR_ASR_TASK_INIT
*/
RETURN_CODE init_asr_task(asr_task_t p_task, char* file_name);
/*
*@Description:初始化tts任务
*@param: in tts_task_t
*@param: in text : 要合成语音的内容字符串
*@param: in result_audio_name : 要保存到的文件名
*@return: true:RETURN_OK  fail: ERROR_TTS_TASK_INIT
*/
RETURN_CODE init_tts_task(tts_task_t p_task, char* text, char* result_audio_name);
// asr_task 反初始化  暂时没用
RETURN_CODE unint_asr_task(asr_task_t p_task);
//tts_task 反初始化  暂时没用
RETURN_CODE unint_tts_task(tts_task_t p_task);


/**
 * @brief 实际运行
 * @param config 设置
 * @param token appkey appsecret换取的token
 * @return
 */
RETURN_CODE baidu_asr(struct asr_config *config, asr_task_t p_task);

RETURN_CODE baidu_tts(struct tts_config *config, tts_task_t p_task);

// 调用阿里语音识别接口
RETURN_CODE ali_asr_v2(struct asr_config *config, asr_task_t p_task);
//调用阿里tts接口
RETURN_CODE ali_tts(struct tts_config *config, tts_task_t p_task);


/**
 * @brief 将文件读出，返回
 * @param fp in 文件
 * @param content_len out 文件大小
 * @return 结果需要自行释放 free
 */
char *read_file_data(FILE *fp, int *content_len);

#endif //SPEECH_C_DEMO_ASRMAIN_H
