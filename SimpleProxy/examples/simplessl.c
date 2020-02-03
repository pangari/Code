#include <stdio.h>

#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>

static size_t write_data(char *buffer, size_t size, size_t nitems, void *userp)
{
    return fwrite(buffer, size, nitems, (FILE *)userp);
}
static size_t write_header(char *buffer, size_t size, size_t nitems, void *userp)
{
    return fwrite(buffer, size, nitems, (FILE *)userp);
}

//int main(int argc, char **argv)
int main_ssl(int argc, char **argv)
{
    CURL *curl;
    CURLcode res;

    static const char *pCertFile = "cert.pem";

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if(curl) 
    {
        curl_easy_setopt(curl, CURLOPT_URL, "https://202.6.168.156/webapi?SubSystem=User&Action=Login&User=SLE_DEV&Password=SGT12345");

        curl_easy_setopt(curl, CURLOPT_HEADERDATA, stderr);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, write_header);

        curl_easy_setopt(curl, CURLOPT_WRITEDATA, stdout);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);

        while(1)
        {
            curl_easy_setopt(curl,CURLOPT_SSLCERTTYPE,"PEM");
            curl_easy_setopt(curl,CURLOPT_SSLCERT,pCertFile);
            curl_easy_setopt(curl,CURLOPT_SSL_VERIFYPEER,FALSE);
            curl_easy_setopt(curl,CURLOPT_SSL_VERIFYHOST,1);
            res = curl_easy_perform(curl);
            break;
        }

        /* always cleanup */
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();

    return 0;
}
