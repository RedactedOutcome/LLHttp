#include "LLHttp/pch.h"
#include "ParsedURL.h"

    namespace LLHttp{
    int ParsedURL::ParseURL(const HBuffer& url)
    {
        //TODO: introduce url error codes
        m_Protocol = URLProtocol::Unsupported;
        size_t at = 0;

        //Imply ports
        if(url.StartsWith("http://", 7))
        {
            m_Protocol = URLProtocol::Http;
            m_Port = 80;
            at += 7;
        }
        else if (url.StartsWith("https://", 8) == 0)
        {
            m_Protocol = URLProtocol::Https;
            m_Port = 443;
            at += 8;
        }else{
            m_Protocol = URLProtocol::Http;
            m_Port = 80;
        }

        size_t hostStart = at;
        char c = url.Get(at);
        while (c != '\00')
        {
            if(c == ' '){
                return -2;
            }
            if (c == '/')break;
            if(c == ':')break;
            at++;
            c = url.Get(at);
        }
        
        //Check if has . for example google.com
        bool isValidHost = false;
        for(size_t i = hostStart; i < at; i++){
            if(url.At(i) == '.'){
                isValidHost=true;
                break;
            }
        }

        size_t hostLength = at - hostStart;
        if(!isValidHost && hostLength > 0){
            return (int)HttpEncodingErrorCode::InvalidHostname;
        }

        if(hostLength < 1){
            //Just assume localhost
            m_Host = "127.0.0.1";//FOR SERVER LOCALHOST
        }else{
            char* host = new char[hostLength + 1];
            memcpy(host, url.GetData() + hostStart, hostLength);
            memset(host + hostLength, '\0', 1);
            m_Host = std::move(HBuffer(host, hostLength, hostLength + 1, true, true));
        }

        //Check if they dont want it implied
        c = url.Get(at++);
        if(c == '\0')return (int)HttpEncodingErrorCode::NeedsMoreData;

        if(c == ':'){
            size_t portStart = at;
            //if(!std::isdigit(url.Get(at)))return (int)HttpEncodingErrorCode::InvalidPort;
            while(std::isdigit(url.Get(at))){
                at++;
            }

            size_t portLength = at - portStart;
            if(portLength > 5 || portLength < 1){
                //PORT TOO LONG
                return (int)HttpEncodingErrorCode::InvalidPort;
            }
            ///TODO: replace with status = url.ToString(portStart, portLength, result);
            char* port = new char[portLength + 1];
            memcpy(port, url.GetData() + portStart, portLength);
            memset(port + portLength, '\0', 1);
            m_Port = std::atoi(port);
            delete port;
        }

        c = url.At(at);

        std::cout << "T1"<<std::endl;
        if(c == '/'){
            std::cout << "Getting Path";
            size_t pathStart = at;
            char j = url.Get(at);
            while(j != '\0'){
                if(j == ' '){
                    //NO SPACES ALLOWED
                    return (int)HttpEncodingErrorCode::InvalidPath;
                }
                j = url.Get(++at);
            }

            size_t pathLength = at - pathStart;
            char* path = new char[pathLength + 1];
            memcpy(path, url.GetData() + pathStart, pathLength);
            memset(path + pathLength, '\0', 1);
            m_Path.Assign(path, pathLength, pathLength + 1, true, true);
        }else{
            std::cout << "char is " << c << " " << (size_t)c <<std::endl;
        }
        return 0;
    }
}