//
// Created by 78472 on 2021/3/19.
//

#include "myutil.h"

extern "C" {
#include "encrypt_sign.c"
}

namespace lhytemp{

    std::mutex myutil::rwLock;
    std::string myutil::cacheDir;
    std::vector<string> myutil::topicVector;
     bool myutil::tvReady2Register = false;


    b64_size_t myutil::Base64_encode( char *out, b64_size_t out_len, const b64_data_t *in, b64_size_t in_len )
    {
        static const char BASE64_ENCODE_TABLE[] =
                "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                "abcdefghijklmnopqrstuvwxyz"
                "0123456789+/=";
        b64_size_t ret = 0u;
        b64_size_t out_count = 0u;

        while ( in_len > 0u && out_count < out_len )
        {
            int i;
            unsigned char c[] = { 0, 0, 64, 64 }; /* index of '=' char */

            /* first character */
            i = *in;
            c[0] = (i & ~0x3) >> 2;

            /* second character */
            c[1] = (i & 0x3) << 4;
            --in_len;
            if ( in_len > 0u )
            {
                ++in;
                i = *in;
                c[1] |= (i & ~0xF) >> 4;

                /* third character */
                c[2] = (i & 0xF) << 2;
                --in_len;
                if ( in_len > 0u )
                {
                    ++in;
                    i = *in;
                    c[2] |= (i & ~0x3F) >> 6;

                    /* fourth character */
                    c[3] = (i & 0x3F);
                    --in_len;
                    ++in;
                }
            }

            /* encode the characters */
            out_count += 4u;
            for ( i = 0; i < 4 && out_count <= out_len; ++i, ++out )
                *out = BASE64_ENCODE_TABLE[c[i]];
        }

        if ( out_count <= out_len )
        {
            if ( out_count < out_len )
                *out = '\0';
            ret = out_count;
        }
        return ret;
    }

    b64_size_t myutil::Base64_decode(b64_data_t *out, b64_size_t out_len, const char *in, b64_size_t in_len) {

#define NV 64
        static const unsigned char BASE64_DECODE_TABLE[] =
                {
                        NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, /*  0-15 */
                        NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, /* 16-31 */
                        NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, 62, NV, NV, NV, 63, /* 32-47 */
                        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, NV, NV, NV, NV, NV, NV, /* 48-63 */
                        NV,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, /* 64-79 */
                        15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, NV, NV, NV, NV, NV, /* 80-95 */
                        NV, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, /* 96-111 */
                        41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, NV, NV, NV, NV, NV, /* 112-127 */
                        NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, /* 128-143 */
                        NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, /* 144-159 */
                        NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, /* 160-175 */
                        NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, /* 176-191 */
                        NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, /* 192-207 */
                        NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, /* 208-223 */
                        NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, /* 224-239 */
                        NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, NV, NV  /* 240-255 */
                };

        b64_size_t ret = 0u;
        b64_size_t out_count = 0u;

        /* in valid base64, length must be multiple of 4's: 0, 4, 8, 12, etc */
        while ( in_len > 3u && out_count < out_len )
        {
            int i;
            unsigned char c[4];
            for ( i = 0; i < 4; ++i, ++in )
                c[i] = BASE64_DECODE_TABLE[(int)(*in)];
            in_len -= 4u;

            /* first byte */
            *out = c[0] << 2;
            *out |= (c[1] & ~0xF) >> 4;
            ++out;
            ++out_count;

            if ( out_count < out_len )
            {
                /* second byte */
                *out = (c[1] & 0xF) << 4;
                if ( c[2] < NV )
                {
                    *out |= (c[2] & ~0x3) >> 2;
                    ++out;
                    ++out_count;

                    if ( out_count < out_len )
                    {
                        /* third byte */
                        *out = (c[2] & 0x3) << 6;
                        if ( c[3] < NV )
                        {
                            *out |= c[3];
                            ++out;
                            ++out_count;
                        }
                        else
                            in_len = 0u;
                    }
                }
                else
                    in_len = 0u;
            }
        }

        if ( out_count <= out_len )
        {
            ret = out_count;
            if ( out_count < out_len )
                *out = '\0';
        }
        return ret;


    }


    string myutil::byte2HexString(unsigned char* in, int length){

        string str;
        for(int i = 0; i < length; i++){
            char hex1;
            char hex2;
            int value = in[i];
            int v1 = value/16;
            int v2 = value%16;

            if(v1 >=0 && v1 <=9){
                hex1 = (char)(48 + v1);
            }else{
                hex1 = (char)(87 + v1);
            }

            if(v2 >=0 && v2 <=9){
                hex2 = (char)(48 + v2);
            }else{
                hex2 = (char)(87 + v2);
            }
            str = str + hex1 +hex2;
        }
        return str;
    }

     string myutil::getSecretMsg(const string& dirPath, const string& baseInfoFilePath){

        qlibc::QData info;
        info.loadFromFile(baseInfoFilePath);

        //  生成秘钥的时机：generateFile.json文件不存在 || 文件存储的加密文件名为空 || 加密文件名不存在
        string secret_file_name_fullPath;
        string secret_file_name;
        bool secretReadyFlag = false;
        const string secretDir = FileUtils::contactFileName(dirPath, "secret");
        string generateFilePath = FileUtils::contactFileName(secretDir, "generateFile.json");
        bool generateFileIsExist = FileUtils::fileExists(generateFilePath);

        if(generateFileIsExist){    //generateFile.json存在
            qlibc::QData generateFileInfo;
            generateFileInfo.loadFromFile(generateFilePath);
            secret_file_name = generateFileInfo.getString("filename");

            if(!secret_file_name.empty()){  //generateFile.json里存储的文件名不为空
                secret_file_name_fullPath = FileUtils::contactFileName(secretDir, secret_file_name);
                if(FileUtils::fileExists(secret_file_name_fullPath)){   //文件名对应的加密文件存在
                    secretReadyFlag = true;
//                    QLog("==>secret file already exists....\n");
                }
            }
        }

        if(!secretReadyFlag){
            uint8_t file_name[256];
            uint32_t file_name_len = 256;
            generate_save_secp256k1_key(const_cast<char *>(secretDir.c_str()), file_name, &file_name_len);
            secret_file_name = string(reinterpret_cast<const char *>(file_name), 0, file_name_len);
//            QLog("===> generate secret file: %s\n", secret_file_name.c_str());

            qlibc::QData qd;
            qd.setString("filename", secret_file_name);
            qd.saveToFile(generateFilePath);
        }

        //获取生成的签名私钥
        uint8_t file_name_publickey[64]{};
        uint32_t file_name_len_publickey = 32;
        unsigned char pub_key[66]{};
        memcpy(file_name_publickey, secret_file_name.c_str(), 32);

        get_secp256k1_pub_key_from_file(const_cast<char *>(secretDir.c_str()), file_name_publickey, file_name_len_publickey, pub_key);
        string publickey_hexString = lhytemp::myutil::byte2HexString(pub_key, 65);

        //构造待加密的加密体
        Json::Value secretMessage;
        secretMessage["deviceSn"] = info.getString("deviceSn");
        secretMessage["deviceMac"] = info.getString("deviceMac");
        secretMessage["pubKey"] = publickey_hexString;
        Json::StreamWriterBuilder swbuilder;
        std::string jsString = Json::writeString(swbuilder, secretMessage);

        //用固定秘钥对加密体进行加密
        char* message = const_cast<char*>(jsString.c_str());
//        QLog("====>origin message to upload--size: %d---message: %s\n", jsString.size(), message);
        uint8_t file_name[256] = "620EE4666F39AAA0CCA8AE95F20FBA0D";    //固定秘钥文件名
        uint32_t file_name_len = 32;    //固定秘钥长度

        uint8_t encrypt_buf[256]{};   //加密后报文
        uint32_t encrypt_len = 256; //加密报文长度
        rsa_encrypt(const_cast<char *>(secretDir.c_str()), file_name, file_name_len, message, encrypt_buf, &encrypt_len);

        //base64编码
        char out[1024]{};
        b64_size_t out_len = 1024;
        lhytemp::myutil::Base64_encode(out, out_len, encrypt_buf, encrypt_len);
        string outstr(out, 0, strlen(out));
//        QLog("======secretMsg: %s\n", outstr.c_str());

        return outstr;
    }

    string myutil::getTvSign(string& tvDid, const string& second, const string& dir) {

        string tvSignMessage = tvDid + second;
        const string secretDir = FileUtils::contactFileName(dir, "secret");
        const string generateFilePath = FileUtils::contactFileName(secretDir, "generateFile.json");
//        QLog("=====>generateFilePath: %s\n", generateFilePath.c_str());

        qlibc::QData secretInfo;
        secretInfo.loadFromFile(generateFilePath);
        string filename = secretInfo.getString("filename");

        uint8_t file_name[64]{};  //生成秘钥文件名
        uint32_t file_name_len = 32;    //生成秘钥长度
        unsigned char sign_buf[66]{};
        memcpy(file_name, filename.c_str(), filename.size());

//        QLog("===>secretDir: %s\n", secretDir.c_str());
//        QLog("====>file_name: %s\n", reinterpret_cast<char*>(file_name));
//        QLog("====>tvSignMessage: %s\n", tvSignMessage.c_str());

        string secretDir1 = secretDir + "/";
        sign_secp256K1_recoverable(const_cast<char*>(secretDir1.c_str()), file_name, file_name_len, const_cast<char*>(tvSignMessage.c_str()), sign_buf);
//        QLog("===============>getTvSign3---\n");

        print_hex("sign_buf", sign_buf, 65);
        string str = lhytemp::myutil::byte2HexString(sign_buf, 65);
        return str;
    }

    bool myutil::ecb_encrypt_withPadding(const string &in, string &out, const uint8_t *key) {

        struct AES_ctx ctx{};
        AES_init_ctx(&ctx, key);

        //填充padding
        unsigned int encryptLength = (in.size() / 16 + 1) * 16;
        unsigned int paddingLength = encryptLength - in.size();
        uint8_t *dataWithPadding = new uint8_t[encryptLength];

        memcpy(dataWithPadding, in.data(), in.size());
        memset(dataWithPadding + in.size(), paddingLength, paddingLength);

        //ECB加密
        for(int index = 0; index < in.size() / 16 + 1; index++){
            AES_ECB_encrypt(&ctx, dataWithPadding + index * 16);
        }

        //BASE64加密
        char encodeOut[10240*10]{};
        int encodeOutLength = 1024*10;
        int base64EncodedLen = myutil::Base64_encode(encodeOut, encodeOutLength, dataWithPadding, encryptLength);

        out = string(encodeOut, 0, base64EncodedLen);

        return true;
    }

    bool myutil::ecb_decrypt_withPadding(const string &in, string &out, const uint8_t *key) {
        //base64解密
        b64_data_t decodeOut[1024*64]{};
        b64_size_t decodeOutLength = 1024*64;
        int base64DecodedLen = myutil::Base64_decode(decodeOut, decodeOutLength, reinterpret_cast<const char *>(in.c_str()), in.size());

        //ECB解密
        struct AES_ctx ctx{};
        AES_init_ctx(&ctx, key);

        char *messageBuffer = new char[base64DecodedLen + 1];   //base64解密后的长度，就是真实数据长度（包含填充字节）
        memset(messageBuffer, 0, base64DecodedLen + 1);

        b64_data_t decryptCode[16]{};
        memset(decryptCode, 0, 16);

        for(int i = 0; i < (base64DecodedLen / 16); i++){
            memcpy(decryptCode, decodeOut + i * 16, 16);
            AES_ECB_decrypt(&ctx, decryptCode);
            memcpy(messageBuffer + i*16, decryptCode, 16);
            memset(decryptCode, 0, 16);
        }

        int paddingLength = messageBuffer[base64DecodedLen-1];
        out = string(messageBuffer, 0, base64DecodedLen - paddingLength);
        delete[] messageBuffer;

        return true;
    }


    mutex concurrency::connectMutex;
    bool  concurrency::connectFlag = false;

    mutex concurrency::connectMutex2;
    bool  concurrency::connectFlag2 = false;

     mutex concurrency::registerMutex;
     condition_variable concurrency::registerConVar;
     bool concurrency::registerFlag = false;

     std::mutex concurrency::areaMutex;
     std::string concurrency::area = "1";


}

