from flask import Flask
import base64
import datetime
import hashlib
import hmac
import json
from urllib.parse import urlparse
import ssl
from datetime import datetime
from time import mktime
from urllib.parse import urlencode
from wsgiref.handlers import format_date_time

# 以下密钥信息从控制台获取
Appid = ""  # 填写控制台中获取的 APPID 信息
APISecret = ""  # 填写控制台中获取的 APISecret 信息
APIKey = ""  # 填写控制台中获取的 APIKey 信息

# 用于配置大模型版本，默认“general/generalv2”
domain = "generalv3"   # v1.5版本
# domain = "generalv2"    # v2.0版本
# 云端环境的服务地址
Spark_url = "ws://spark-api.xf-yun.com/v3.1/chat"  # v1.5环境的地址
# Spark_url = "ws://spark-api.xf-yun.com/v2.1/chat"  # v2.0环境的地址
Spark_url1 = "ws://ws-api.xfyun.cn/v2/iat"  # 语音听写
Spark_url2 = "ws://ws-api.xfyun.cn/v2/tts"  # 语音合成
Spark_url3 = "ws://tts-api.xfyun.cn/v2/tts"


app = Flask(__name__)


@app.route('/whf', methods=['GET'])
def handle_request():
    host = urlparse(Spark_url).netloc
    path = urlparse(Spark_url).path
    now = datetime.now()
    date = format_date_time(mktime(now.timetuple()))

    # 拼接字符串
    signature_origin = "host: " + host + "\n"
    signature_origin += "date: " + date + "\n"
    signature_origin += "GET " + path + " HTTP/1.1"
    # 进行hmac-sha256进行加密
    signature_sha = hmac.new(APISecret.encode('utf-8'), signature_origin.encode('utf-8'),
                             digestmod=hashlib.sha256).digest()
    signature_sha_base64 = base64.b64encode(
        signature_sha).decode(encoding='utf-8')
    authorization_origin = f'api_key="{APIKey}", algorithm="hmac-sha256", headers="host date request-line", signature="{signature_sha_base64}"'
    authorization = base64.b64encode(
        authorization_origin.encode('utf-8')).decode(encoding='utf-8')
    # 将请求的鉴权参数组合为字典
    v = {
        "authorization": authorization,
        "date": date,
        "host": host
    }
    # 拼接鉴权参数，生成url
    url = Spark_url + '?' + urlencode(v)
    return url  # 返回一个简单的消息示例


@app.route('/whf1', methods=['GET'])
def handle_request1():
    host = urlparse(Spark_url1).netloc
    path = urlparse(Spark_url1).path
    now = datetime.now()
    date = format_date_time(mktime(now.timetuple()))

    # 拼接字符串
    signature_origin = "host: " + host + "\n"
    signature_origin += "date: " + date + "\n"
    signature_origin += "GET " + path + " HTTP/1.1"
    # 进行hmac-sha256进行加密
    signature_sha = hmac.new(APISecret.encode('utf-8'), signature_origin.encode('utf-8'),
                             digestmod=hashlib.sha256).digest()
    signature_sha_base64 = base64.b64encode(
        signature_sha).decode(encoding='utf-8')
    authorization_origin = f'api_key="{APIKey}", algorithm="hmac-sha256", headers="host date request-line", signature="{signature_sha_base64}"'
    authorization = base64.b64encode(
        authorization_origin.encode('utf-8')).decode(encoding='utf-8')
    # 将请求的鉴权参数组合为字典
    v = {
        "authorization": authorization,
        "date": date,
        "host": host
    }
    # 拼接鉴权参数，生成url
    url = Spark_url1 + '?' + urlencode(v)
    return url  # 返回一个简单的消息示例


@app.route('/whf2', methods=['GET'])
def handle_request2():
    host = urlparse(Spark_url2).netloc
    path = urlparse(Spark_url2).path
    now = datetime.now()
    date = format_date_time(mktime(now.timetuple()))

    # 拼接字符串
    signature_origin = "host: " + host + "\n"
    signature_origin += "date: " + date + "\n"
    signature_origin += "GET " + path + " HTTP/1.1"
    # 进行hmac-sha256进行加密
    signature_sha = hmac.new(APISecret.encode('utf-8'), signature_origin.encode('utf-8'),
                             digestmod=hashlib.sha256).digest()
    signature_sha_base64 = base64.b64encode(
        signature_sha).decode(encoding='utf-8')
    authorization_origin = f'api_key="{APIKey}", algorithm="hmac-sha256", headers="host date request-line", signature="{signature_sha_base64}"'
    authorization = base64.b64encode(
        authorization_origin.encode('utf-8')).decode(encoding='utf-8')
    # 将请求的鉴权参数组合为字典
    v = {
        "authorization": authorization,
        "date": date,
        "host": host
    }
    # 拼接鉴权参数，生成url
    url = Spark_url3 + '?' + urlencode(v)
    return url  # 返回一个简单的消息示例


if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
