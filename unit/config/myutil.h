//
// Created by 78472 on 2021/3/19.
//

#ifndef GAFDEV_MYUTIL_H
#define GAFDEV_MYUTIL_H

#include <string>
#include <mutex>
#include <ctime>
#include <vector>
#include <atomic>
#include <condition_variable>

#include "aes/aes.hpp"
#include "qlibc/QData.h"
#include "qlibc/FileUtils.h"
#include "configParamUtil.h"

typedef unsigned int b64_size_t;
typedef unsigned char b64_data_t;

using namespace std;
namespace lhytemp{

    class myutil {
    public:
        static std::mutex rwLock;
        static std::vector<string> topicVector;

    public:
        /**
         * Encodes base64 data
         *
         * @param[out]     out                 encode base64 string
         * @param[in]      out_len             length of output buffer
         * @param[in]      in                  raw data to encode
         * @param[in]      in_len              length of input buffer
         *
         * @return the amount of data encoded
         *
         * @see Base64_decode
         * @see Base64_encodeLength
         */
        static b64_size_t Base64_encode( char *out, b64_size_t out_len,
                                         const b64_data_t *in, b64_size_t in_len );

        /**
         * Decodes base64 data
         *
         * @param[out]     out                 decoded data
         * @param[in]      out_len             length of output buffer
         * @param[in]      in                  base64 string to decode
         * @param[in]      in_len              length of input buffer
         *
         * @return the amount of data decoded
         *
         * @see Base64_decodeLength
         * @see Base64_encode
         */
        static b64_size_t Base64_decode( b64_data_t *out, b64_size_t out_len,
                                         const char *in, b64_size_t in_len );

        static string byte2HexString(unsigned char* in, int length);

        /**
         * 1. 如果秘钥文件不存在，则生成秘钥文件
         * 2. 从秘钥文件中提取出pub_key
         * 3. 用事先提供的固定秘钥，加密从秘钥文件中提取出的pub_key
         * @param dirPath
         * @param baseInfoFile
         * @return
         */
        static string getSecretMsg(const string& dirPath);

        /**
         * 用生成的秘钥文件加密<tvDid+second>
         * @param tvDid
         * @param second
         * @param dir
         * @return
         */
        static string getTvSign(string& tvDid, const string& second, const string& dir);


        static void storeTopic(const string& topic){    //如果该主题没有被存储，则存储
            std::lock_guard<std::mutex> lg(rwLock);
            for(auto& elem : topicVector){
                if(topic == elem)
                   return;
            }
            topicVector.emplace_back(topic);
        }

        static vector<string> getTopic(){
            std::lock_guard<std::mutex> lg(rwLock);
            return topicVector;
        }

        static bool ecb_encrypt_withPadding(const string &in , string &out, const uint8_t *key);

        static bool ecb_decrypt_withPadding(const string& in, string& out, const uint8_t *key);

    };

    class concurrency{
    public:

        static std::string area;
    };


}



#endif //GAFDEV_MYUTIL_H
