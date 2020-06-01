#include "ImageWorker.h"
namespace NodeBinding
{

    void ImageWorker::setOnErrorCallback(Napi::Function func)
    {
        this->onErrorCallback = Napi::Persistent(func);
    }

    void ImageWorker::setOnLoadCallback(Napi::Function func)
    {
        this->onLoadCallback = Napi::Persistent(func);
    }

    void ImageWorker::OnOK()
    {
        cachedImage(this->url,this->mImage);
        if (this->onLoadCallback)
        {
            this->onLoadCallback.Call({Env().Undefined()});
        }
    }

    void ImageWorker::OnError(const Napi::Error &e)
    {
        if (this->onErrorCallback)
        {
            this->onErrorCallback.Call({Napi::String::New(Env(), e.Message())});
        }
    }

    void ImageWorker::Execute()
    {
        if (url.rfind("http", 0) == 0 || url.rfind("https", 0) == 0)
        {
            content.size = downloadImage(url, &content);
            if ((int)content.size <= 0)
            {
                free(content.memory);
                content.memory = nullptr;
                this->SetError(std::move("Image Download Fail"));
                return;
            }
        }
        else
        { //本地文件
            content.size = readLocalImage(url, &content);
            if ((int)content.size <= 0)
            {
                free(content.memory);
                content.memory = nullptr;
                this->SetError(std::move("Image Read Fail"));
                return;
            }
        }

        PIC_FORMAT format = getPicFormatFromContent(content.memory, content.size);
        if (format == PNG_FORAMT)
        {
            decodeFromPNGImage(this->mImage->getPixels(), _width, _height, (const unsigned char *)content.memory, content.size);
        }
        else if (format == JPEG_FORMAT)
        {
            decodeFromJEPGImage(this->mImage->getPixels(), _width, _height, (const unsigned char *)content.memory, (unsigned int)content.size);
        }
        else if (format == UNKOWN_PIC_FORMAT)
        {
            this->SetError(std::move("Image Format Unspported"));
        }
        free(content.memory);
        content.memory = nullptr;
    }
} // namespace NodeBinding