##修改自百度开源项目，添加阿里asr及tts支持

## 简介

使用C 代码测试本地音频文件的识别流程。

项目仅依赖libcurl库，编译命令为gcc

## 环境准备

目前支持Linux和windows的cygwin环境

### libcurl 库

```bash
#linux centos
yum install libcurl-devel

# linux ubuntu 一般已经安装
apt-get install libcurl3

#cygwin
#运行setup-x86_64.exe，输入libcurl-devel搜索
```



gcc

```bash
gcc -v #验证版本

#linux centos
yum groupinfo Development tools

# linux ubuntu
apt-get  install  build-essential

#cygwin
#运行setup-x86_64.exe，输入gcc-core搜索 
```






## 测试流程

cd  项目目录

sh build_and_asr.sh  编译及运行


   ​