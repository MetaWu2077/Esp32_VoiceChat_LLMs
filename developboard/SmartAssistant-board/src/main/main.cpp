#include <Arduino.h>
#include "base64.h"
#include "WiFi.h"
#include <WiFiClientSecure.h>
#include "HTTPClient.h"
#include "Audio1.h"
#include "Audio2.h"
#include <ArduinoJson.h>
#include <ArduinoWebsockets.h>
using namespace websockets;

#define key 0
#define ADC 32
#define led3 2
#define led2 18
#define led1 19
const char *wifiData[][2] = {
    {"111", "11111111"}, // 替换为自己常用的wifi名和密码
    // {"222", "12345678"},
    // 继续添加需要的 Wi-Fi 名称和密码
};

String APPID = " "; // 自己的星火大模型账号参数
String APIKey = " ";
String APISecret = " ";

bool ledstatus = true;
bool startPlay = false;
bool lastsetence = false;
bool isReady = false;
unsigned long urlTime = 0;
unsigned long pushTime = 0;
int mainStatus = 0;
int receiveFrame = 0;
int noise = 50;
HTTPClient https;

hw_timer_t *timer = NULL;

uint8_t adc_start_flag = 0;
uint8_t adc_complete_flag = 0;

Audio1 audio1;
Audio2 audio2(false, 3, I2S_NUM_1);

#define I2S_DOUT 27 // DIN
#define I2S_BCLK 26 // BCLK
#define I2S_LRC 25  // LRC

void gain_token(void);
void getText(String role, String content);
void checkLen(JsonArray textArray);
int getLength(JsonArray textArray);
float calculateRMS(uint8_t *buffer, int bufferSize);
void ConnServer();
void ConnServer1();

DynamicJsonDocument doc(4000);
JsonArray text = doc.to<JsonArray>();

String url = "";
String url1 = "";
String Date = "";
DynamicJsonDocument gen_params(const char *appid, const char *domain);

String askquestion = "";
String Answer = "";

const char *appId1 = ""; // 替换为自己的星火大模型参数
const char *domain1 = "generalv3";
const char *websockets_server = "ws://spark-api.xf-yun.com/v3.5/chat";
const char *websockets_server1 = "ws://ws-api.xfyun.cn/v2/iat";
using namespace websockets;

WebsocketsClient webSocketClient;
WebsocketsClient webSocketClient1;

int loopcount = 0;
void onMessageCallback(WebsocketsMessage message)
{
    StaticJsonDocument<4096> jsonDocument;
    DeserializationError error = deserializeJson(jsonDocument, message.data());

    if (!error)
    {
        int code = jsonDocument["header"]["code"];
        if (code != 0)
        {
            Serial.print("sth is wrong: ");
            Serial.println(code);
            Serial.println(message.data());
            webSocketClient.close();
        }
        else
        {
            receiveFrame++;
            Serial.print("receiveFrame:");
            Serial.println(receiveFrame);
            JsonObject choices = jsonDocument["payload"]["choices"];
            int status = choices["status"];
            const char *content = choices["text"][0]["content"];
            Serial.println(content);
            Answer += content;
            String answer = "";
            if (Answer.length() >= 120 && (audio2.isplaying == 0))
            {
                String subAnswer = Answer.substring(0, 120);
                Serial.print("subAnswer:");
                Serial.println(subAnswer);
                int lastPeriodIndex = subAnswer.lastIndexOf("。");

                if (lastPeriodIndex != -1)
                {
                    answer = Answer.substring(0, lastPeriodIndex + 1);
                    Serial.print("answer: ");
                    Serial.println(answer);
                    Answer = Answer.substring(lastPeriodIndex + 2);
                    Serial.print("Answer: ");
                    Serial.println(Answer);
                    audio2.connecttospeech(answer.c_str(), "zh");
                }
                else
                {
                    const char *chinesePunctuation = "？，：；,.";

                    int lastChineseSentenceIndex = -1;

                    for (int i = 0; i < Answer.length(); ++i)
                    {
                        char currentChar = Answer.charAt(i);

                        if (strchr(chinesePunctuation, currentChar) != NULL)
                        {
                            lastChineseSentenceIndex = i;
                        }
                    }
                    if (lastChineseSentenceIndex != -1)
                    {
                        answer = Answer.substring(0, lastChineseSentenceIndex + 1);
                        audio2.connecttospeech(answer.c_str(), "zh");
                        Answer = Answer.substring(lastChineseSentenceIndex + 2);
                    }
                    else
                    {
                        answer = Answer.substring(0, 120);
                        audio2.connecttospeech(answer.c_str(), "zh");
                        Answer = Answer.substring(120 + 1);
                    }
                }
                startPlay = true;
            }

            if (status == 2)
            {
                getText("assistant", Answer);
                if (Answer.length() <= 80 && (audio2.isplaying == 0))
                {
                    // getText("assistant", Answer);
                    audio2.connecttospeech(Answer.c_str(), "zh");
                }
            }
        }
    }
}

void onEventsCallback(WebsocketsEvent event, String data)
{
    if (event == WebsocketsEvent::ConnectionOpened)
    {
        Serial.println("Send message to server0!");
        DynamicJsonDocument jsonData = gen_params(appId1, domain1);
        String jsonString;
        serializeJson(jsonData, jsonString);
        Serial.println(jsonString);
        webSocketClient.send(jsonString);
    }
    else if (event == WebsocketsEvent::ConnectionClosed)
    {
        Serial.println("Connnection0 Closed");
    }
    else if (event == WebsocketsEvent::GotPing)
    {
        Serial.println("Got a Ping!");
    }
    else if (event == WebsocketsEvent::GotPong)
    {
        Serial.println("Got a Pong!");
    }
}

void onMessageCallback1(WebsocketsMessage message)
{
    StaticJsonDocument<4096> jsonDocument;
    DeserializationError error = deserializeJson(jsonDocument, message.data());

    if (!error)
    {
        int code = jsonDocument["code"];
        if (code != 0)
        {
            Serial.println(code);
            Serial.println(message.data());
            webSocketClient1.close();
        }
        else
        {
            Serial.println("xunfeiyun return message:");
            Serial.println(message.data());
            JsonArray ws = jsonDocument["data"]["result"]["ws"].as<JsonArray>();

            for (JsonVariant i : ws)
            {
                for (JsonVariant w : i["cw"].as<JsonArray>())
                {
                    askquestion += w["w"].as<String>();
                }
            }
            Serial.println(askquestion);
            int status = jsonDocument["data"]["status"];
            if (status == 2)
            {
                Serial.println("status == 2");
                webSocketClient1.close();
                if (askquestion == "")
                {
                    askquestion = "sorry, i can't hear you";
                    audio2.connecttospeech(askquestion.c_str(), "zh");
                }
                else if (askquestion.substring(0, 9) == "唱歌了" or askquestion.substring(0, 9) == "唱歌啦")
                {

                    if (askquestion.substring(0, 12) == "唱歌了，" or askquestion.substring(0, 12) == "唱歌啦，")
                    { // 自建音乐服务器，按照文件名查找对应歌曲
                        String audioStreamURL = "http://192.168.0.1/mp3/" + askquestion.substring(12, askquestion.length() - 3) + ".mp3";
                        Serial.println(audioStreamURL.c_str());
                        audio2.connecttohost(audioStreamURL.c_str());
                    }
                    else if (askquestion.substring(9) == "。")
                    {
                        askquestion = "好啊, 你想听什么歌？";
                        mainStatus = 1;
                        audio2.connecttospeech(askquestion.c_str(), "zh");
                    }
                    else
                    {
                        String audioStreamURL = "http://192.168.0.1/mp3/" + askquestion.substring(9, askquestion.length() - 3) + ".mp3";
                        Serial.println(audioStreamURL.c_str());
                        audio2.connecttohost(audioStreamURL.c_str());
                    }
                }
                else if (mainStatus == 1)
                {
                    askquestion.trim();
                    if (askquestion.endsWith("。"))
                    {
                        askquestion = askquestion.substring(0, askquestion.length() - 3);
                    }
                    else if (askquestion.endsWith(".") or askquestion.endsWith("?"))
                    {
                        askquestion = askquestion.substring(0, askquestion.length() - 1);
                    }
                    String audioStreamURL = "http://192.168.0.1/mp3/" + askquestion + ".mp3";
                    Serial.println(audioStreamURL.c_str());
                    audio2.connecttohost(audioStreamURL.c_str());
                    mainStatus = 0;
                }
                else
                {
                    getText("user", askquestion);
                    Serial.print("text:");
                    Serial.println(text);
                    Answer = "";
                    lastsetence = false;
                    isReady = true;
                    ConnServer();
                }
            }
        }
    }
    else
    {
        Serial.println("error:");
        Serial.println(error.c_str());
        Serial.println(message.data());
    }
}

void onEventsCallback1(WebsocketsEvent event, String data)
{
    if (event == WebsocketsEvent::ConnectionOpened)
    {
        Serial.println("Send message to xunfeiyun");
        digitalWrite(led2, HIGH);
        int silence = 0;
        int firstframe = 1;
        int j = 0;
        int voicebegin = 0;
        int voice = 0;
        DynamicJsonDocument doc(2500);
        while (1)
        {
            doc.clear();
            JsonObject data = doc.createNestedObject("data");
            audio1.Record();
            float rms = calculateRMS((uint8_t *)audio1.wavData[0], 1280);
            printf("%d %f\n", 0, rms);
            if (rms < noise)
            {
                if (voicebegin == 1)
                {
                    silence++;
                    // Serial.print("noise:");
                    // Serial.println(noise);
                }
            }
            else
            {
                voice++;
                if (voice >= 5)
                {
                    voicebegin = 1;
                }
                else
                {
                    voicebegin = 0;
                }
                silence = 0;
            }
            if (silence == 6)
            {
                data["status"] = 2;
                data["format"] = "audio/L16;rate=8000";
                data["audio"] = base64::encode((byte *)audio1.wavData[0], 1280);
                data["encoding"] = "raw";
                j++;

                String jsonString;
                serializeJson(doc, jsonString);

                webSocketClient1.send(jsonString);
                digitalWrite(led2, LOW);
                delay(40);
                break;
            }
            if (firstframe == 1)
            {
                data["status"] = 0;
                data["format"] = "audio/L16;rate=8000";
                data["audio"] = base64::encode((byte *)audio1.wavData[0], 1280);
                data["encoding"] = "raw";
                j++;

                JsonObject common = doc.createNestedObject("common");
                common["app_id"] = appId1;

                JsonObject business = doc.createNestedObject("business");
                business["domain"] = "iat";
                business["language"] = "zh_cn";
                business["accent"] = "mandarin";
                business["vinfo"] = 1;
                business["vad_eos"] = 1000;

                String jsonString;
                serializeJson(doc, jsonString);

                webSocketClient1.send(jsonString);
                firstframe = 0;
                delay(40);
            }
            else
            {
                data["status"] = 1;
                data["format"] = "audio/L16;rate=8000";
                data["audio"] = base64::encode((byte *)audio1.wavData[0], 1280);
                data["encoding"] = "raw";

                String jsonString;
                serializeJson(doc, jsonString);

                webSocketClient1.send(jsonString);
                delay(40);
            }
        }
    }
    else if (event == WebsocketsEvent::ConnectionClosed)
    {
        Serial.println("Connnection1 Closed");
    }
    else if (event == WebsocketsEvent::GotPing)
    {
        Serial.println("Got a Ping!");
    }
    else if (event == WebsocketsEvent::GotPong)
    {
        Serial.println("Got a Pong!");
    }
}
void ConnServer()
{
    Serial.println("url:" + url);

    webSocketClient.onMessage(onMessageCallback);
    webSocketClient.onEvent(onEventsCallback);
    // Connect to WebSocket
    Serial.println("Begin connect to server0......");
    if (webSocketClient.connect(url.c_str()))
    {
        Serial.println("Connected to server0!");
    }
    else
    {
        Serial.println("Failed to connect to server0!");
    }
}

void ConnServer1()
{
    // Serial.println("url1:" + url1);
    webSocketClient1.onMessage(onMessageCallback1);
    webSocketClient1.onEvent(onEventsCallback1);
    // Connect to WebSocket
    Serial.println("Begin connect to server1......");
    if (webSocketClient1.connect(url1.c_str()))
    {
        Serial.println("Connected to server1!");
    }
    else
    {
        Serial.println("Failed to connect to server1!");
    }
}

void voicePlay()
{
    if ((audio2.isplaying == 0) && Answer != "")
    {
        // String subAnswer = "";
        // String answer = "";
        // if (Answer.length() >= 100)
        //     subAnswer = Answer.substring(0, 100);
        // else
        // {
        //     subAnswer = Answer.substring(0);
        //     lastsetence = true;
        //     // startPlay = false;
        // }

        // Serial.print("subAnswer:");
        // Serial.println(subAnswer);
        int firstPeriodIndex = Answer.indexOf("。");
        int secondPeriodIndex = 0;

        if (firstPeriodIndex != -1)
        {
            secondPeriodIndex = Answer.indexOf("。", firstPeriodIndex + 1);
            if (secondPeriodIndex == -1)
                secondPeriodIndex = firstPeriodIndex;
        }
        else
        {
            secondPeriodIndex = firstPeriodIndex;
        }

        if (secondPeriodIndex != -1)
        {
            String answer = Answer.substring(0, secondPeriodIndex + 1);
            Serial.print("answer: ");
            Serial.println(answer);
            Answer = Answer.substring(secondPeriodIndex + 2);
            audio2.connecttospeech(answer.c_str(), "zh");
        }
        else
        {
            const char *chinesePunctuation = "？，：；,.";

            int lastChineseSentenceIndex = -1;

            for (int i = 0; i < Answer.length(); ++i)
            {
                char currentChar = Answer.charAt(i);

                if (strchr(chinesePunctuation, currentChar) != NULL)
                {
                    lastChineseSentenceIndex = i;
                }
            }

            if (lastChineseSentenceIndex != -1)
            {
                String answer = Answer.substring(0, lastChineseSentenceIndex + 1);
                audio2.connecttospeech(answer.c_str(), "zh");
                Answer = Answer.substring(lastChineseSentenceIndex + 2);
            }
        }
        startPlay = true;
    }
    else
    {
        // digitalWrite(led3, LOW);
    }
}

void wifiConnect(const char *wifiData[][2], int numNetworks)
{
    WiFi.disconnect(true);
    for (int i = 0; i < numNetworks; ++i)
    {
        const char *ssid = wifiData[i][0];
        const char *password = wifiData[i][1];

        Serial.print("Connecting to ");
        Serial.println(ssid);

        WiFi.begin(ssid, password);
        uint8_t count = 0;
        while (WiFi.status() != WL_CONNECTED)
        {
            digitalWrite(led1, ledstatus);
            ledstatus = !ledstatus;
            Serial.print(".");
            count++;
            if (count >= 30)
            {
                Serial.printf("\r\n-- wifi connect fail! --");
                break;
            }
            vTaskDelay(100);
        }

        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.printf("\r\n-- wifi connect success! --\r\n");
            Serial.print("IP address: ");
            Serial.println(WiFi.localIP());
            Serial.println("Free Heap: " + String(ESP.getFreeHeap()));
            return; // 如果连接成功，退出函数
        }
    }
}

String getUrl(String Spark_url, String host, String path, String Date)
{

    // 拼接字符串
    String signature_origin = "host: " + host + "\n";
    signature_origin += "date: " + Date + "\n";
    signature_origin += "GET " + path + " HTTP/1.1";
    // signature_origin="host: spark-api.xf-yun.com\ndate: Mon, 04 Mar 2024 19:23:20 GMT\nGET /v3.5/chat HTTP/1.1";

    // hmac-sha256 加密
    unsigned char hmac[32];
    mbedtls_md_context_t ctx;
    mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;
    const size_t messageLength = signature_origin.length();
    const size_t keyLength = APISecret.length();
    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 1);
    mbedtls_md_hmac_starts(&ctx, (const unsigned char *)APISecret.c_str(), keyLength);
    mbedtls_md_hmac_update(&ctx, (const unsigned char *)signature_origin.c_str(), messageLength);
    mbedtls_md_hmac_finish(&ctx, hmac);
    mbedtls_md_free(&ctx);

    // base64 编码
    String signature_sha_base64 = base64::encode(hmac, sizeof(hmac) / sizeof(hmac[0]));

    // 替换Date
    Date.replace(",", "%2C");
    Date.replace(" ", "+");
    Date.replace(":", "%3A");
    String authorization_origin = "api_key=\"" + APIKey + "\", algorithm=\"hmac-sha256\", headers=\"host date request-line\", signature=\"" + signature_sha_base64 + "\"";
    String authorization = base64::encode(authorization_origin);
    String url = Spark_url + '?' + "authorization=" + authorization + "&date=" + Date + "&host=" + host;
    Serial.println(url);
    return url;
}

void getTimeFromServer()
{
    String timeurl = "https://www.baidu.com";
    HTTPClient http;
    http.begin(timeurl);
    const char *headerKeys[] = {"Date"};
    http.collectHeaders(headerKeys, sizeof(headerKeys) / sizeof(headerKeys[0]));
    int httpCode = http.GET();
    Date = http.header("Date");
    Serial.println(Date);
    http.end();
    // delay(50); // 可以根据实际情况调整延时时间
}

void setup()
{
    // String Date = "Fri, 22 Mar 2024 03:35:56 GMT";
    Serial.begin(115200);
    // pinMode(ADC,ANALOG);
    pinMode(key, INPUT_PULLUP);
    pinMode(34, INPUT_PULLUP);
    pinMode(35, INPUT_PULLUP);
    pinMode(led1, OUTPUT);
    pinMode(led2, OUTPUT);
    pinMode(led3, OUTPUT);
    audio1.init();

    int numNetworks = sizeof(wifiData) / sizeof(wifiData[0]);
    wifiConnect(wifiData, numNetworks);

    getTimeFromServer();

    audio2.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio2.setVolume(50);

    // String Date = "Fri, 22 Mar 2024 03:35:56 GMT";
    url = getUrl("ws://spark-api.xf-yun.com/v3.5/chat", "spark-api.xf-yun.com", "/v3.5/chat", Date);
    url1 = getUrl("ws://ws-api.xfyun.cn/v2/iat", "ws-api.xfyun.cn", "/v2/iat", Date);
    urlTime = millis();

    ///////////////////////////////////
}

void loop()
{

    webSocketClient.poll();
    webSocketClient1.poll();
    // delay(10);
    if (startPlay)
    {
        voicePlay();
    }

    audio2.loop();

    if (audio2.isplaying == 1)
    {
        digitalWrite(led3, HIGH);
    }
    else
    {
        digitalWrite(led3, LOW);
        if ((urlTime + 240000 < millis()) && (audio2.isplaying == 0))
        {
            urlTime = millis();
            getTimeFromServer();
            url = getUrl("ws://spark-api.xf-yun.com/v3.5/chat", "spark-api.xf-yun.com", "/v3.5/chat", Date);
            url1 = getUrl("ws://ws-api.xfyun.cn/v2/iat", "ws-api.xfyun.cn", "/v2/iat", Date);
        }
    }

    if (digitalRead(key) == 0)
    {
        audio2.isplaying = 0;
        startPlay = false;
        isReady = false;
        Answer = "";
        Serial.printf("Start recognition\r\n\r\n");

        adc_start_flag = 1;
        // Serial.println(esp_get_free_heap_size());

        if (urlTime + 240000 < millis()) // 超过4分钟，重新做一次鉴权
        {
            urlTime = millis();
            getTimeFromServer();
            url = getUrl("ws://spark-api.xf-yun.com/v3.5/chat", "spark-api.xf-yun.com", "/v3.5/chat", Date);
            url1 = getUrl("ws://ws-api.xfyun.cn/v2/iat", "ws-api.xfyun.cn", "/v2/iat", Date);
        }
        askquestion = "";
        // audio2.connecttospeech(askquestion.c_str(), "zh");
        ConnServer1();
        // ConnServer();
        // delay(6000);
        // audio1.Record();
        adc_complete_flag = 0;

        // Serial.println(text);
        // checkLen(text);
    }
}

void getText(String role, String content)
{
    checkLen(text);
    DynamicJsonDocument jsoncon(1024);
    jsoncon["role"] = role;
    jsoncon["content"] = content;
    text.add(jsoncon);
    jsoncon.clear();
    String serialized;
    serializeJson(text, serialized);
    Serial.print("text: ");
    Serial.println(serialized);
    // serializeJsonPretty(text, Serial);
}

int getLength(JsonArray textArray)
{
    int length = 0;
    for (JsonObject content : textArray)
    {
        const char *temp = content["content"];
        int leng = strlen(temp);
        length += leng;
    }
    return length;
}

void checkLen(JsonArray textArray)
{
    while (getLength(textArray) > 3000)
    {
        textArray.remove(0);
    }
    // return textArray;
}

DynamicJsonDocument gen_params(const char *appid, const char *domain)
{
    DynamicJsonDocument data(2048);

    JsonObject header = data.createNestedObject("header");
    header["app_id"] = appid;
    header["uid"] = "1234";

    JsonObject parameter = data.createNestedObject("parameter");
    JsonObject chat = parameter.createNestedObject("chat");
    chat["domain"] = domain;
    chat["temperature"] = 0.5;
    chat["max_tokens"] = 1024;

    JsonObject payload = data.createNestedObject("payload");
    JsonObject message = payload.createNestedObject("message");

    JsonArray textArray = message.createNestedArray("text");
    for (const auto &item : text)
    {
        textArray.add(item);
    }
    return data;
}

float calculateRMS(uint8_t *buffer, int bufferSize)
{
    float sum = 0;
    int16_t sample;

    for (int i = 0; i < bufferSize; i += 2)
    {

        sample = (buffer[i + 1] << 8) | buffer[i];
        sum += sample * sample;
    }

    sum /= (bufferSize / 2);

    return sqrt(sum);
}
