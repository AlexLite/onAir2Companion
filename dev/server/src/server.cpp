#include "stdafx.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mstcpip.h>
#include <winhttp.h>
#include <comutil.h>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include <nlohmann/json.hpp>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Winhttp.lib")
#pragma comment(lib, "comsuppw.lib")

using json = nlohmann::json;
using namespace std::chrono_literals;

namespace {

struct Config {
  std::string listenIp = "0.0.0.0";
  int listenPort = 27015;
  std::string localQueue = "BridgeQueue";
  std::string onAirMachine = "localhost";
  std::string onAirQueue = "FDOnAir1";
  std::string onAirSubject = "OnAir1.Mirror";
  int pollIntervalMs = 10;
  int sendTimeoutMs = 3000;
  bool tcpNoDelay = true;
  bool tcpKeepAlive = true;
  int socketSendBuf = 16384;
  int socketRecvBuf = 16384;
  std::string companionTransport = "tcp";
  std::string companionWsHost = "127.0.0.1";
  int companionWsPort = 16622;
  std::string companionWsPath = "/";
  bool companionWsSecure = false;
  int companionWsTimeoutMs = 2000;
  bool onAirToCompanionEnabled = false;
  std::string onAirToCompanionPrefix = "COMPANION:";
  std::string companionCommandTransport = "http";
  std::string companionHttpBase = "http://127.0.0.1:8000";
};

struct SendRequest {
  long id = 0;
  std::string command;
  std::string toMachine;
  std::string toQueue;
  std::string subject;
  long notify = 0;
  bool done = false;
  bool ok = false;
  long slErrorCode = 0;
  long slResultCode = 0;
  std::string error;
  std::mutex m;
  std::condition_variable cv;
};

std::atomic_bool g_running{ true };
std::atomic_bool g_bridgeConnected{ false };
std::atomic_long g_messageId{ 1 };

std::mutex g_clientsMutex;
std::vector<SOCKET> g_clients;
std::mutex g_sendMutex;
std::mutex g_logMutex;

std::mutex g_sendQueueMutex;
std::condition_variable g_sendQueueCv;
std::deque<std::shared_ptr<SendRequest>> g_sendQueue;

std::mutex g_stateMutex;
std::string g_lastBridgeError;

Config g_cfg;
std::wstring utf8ToWide(const std::string& s);
std::string wideToUtf8(const wchar_t* ws);

class CompanionWsClient {
public:
  explicit CompanionWsClient(const Config& cfg)
    : host_(cfg.companionWsHost),
      port_(cfg.companionWsPort),
      path_(cfg.companionWsPath.empty() ? "/" : cfg.companionWsPath),
      secure_(cfg.companionWsSecure),
      timeoutMs_(cfg.companionWsTimeoutMs > 0 ? cfg.companionWsTimeoutMs : 2000) {}

  ~CompanionWsClient() {
    std::lock_guard<std::mutex> lock(m_);
    closeLocked();
  }

  bool sendText(const std::string& text, std::string& err) {
    std::lock_guard<std::mutex> lock(m_);
    if (!connectLocked(err)) return false;

    const DWORD size = static_cast<DWORD>(text.size());
    DWORD rc = WinHttpWebSocketSend(
      websocket_,
      WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE,
      reinterpret_cast<PVOID>(const_cast<char*>(text.data())),
      size
    );
    if (rc != ERROR_SUCCESS) {
      err = "WinHttpWebSocketSend failed: " + std::to_string(static_cast<long>(rc));
      closeLocked();
      return false;
    }
    return true;
  }

  bool sendJson(const json& obj, std::string& err) {
    return sendText(obj.dump(), err);
  }

  bool isConnected() const {
    std::lock_guard<std::mutex> lock(m_);
    return websocket_ != nullptr;
  }

private:
  bool connectLocked(std::string& err) {
    if (websocket_) return true;

    const std::wstring agent = L"OnAirBridge/1.0";
    session_ = WinHttpOpen(agent.c_str(), WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!session_) {
      err = "WinHttpOpen failed: " + std::to_string(static_cast<long>(GetLastError()));
      closeLocked();
      return false;
    }

    WinHttpSetTimeouts(session_, timeoutMs_, timeoutMs_, timeoutMs_, timeoutMs_);

    std::wstring whost = utf8ToWide(host_);
    connection_ = WinHttpConnect(session_, whost.c_str(), static_cast<INTERNET_PORT>(port_), 0);
    if (!connection_) {
      err = "WinHttpConnect failed: " + std::to_string(static_cast<long>(GetLastError()));
      closeLocked();
      return false;
    }

    std::wstring wpath = utf8ToWide(path_);
    DWORD flags = secure_ ? WINHTTP_FLAG_SECURE : 0;
    request_ = WinHttpOpenRequest(connection_, L"GET", wpath.c_str(), nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
    if (!request_) {
      err = "WinHttpOpenRequest failed: " + std::to_string(static_cast<long>(GetLastError()));
      closeLocked();
      return false;
    }

    if (!WinHttpSetOption(request_, WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET, nullptr, 0)) {
      err = "WinHttpSetOption(UPGRADE_TO_WEB_SOCKET) failed: " + std::to_string(static_cast<long>(GetLastError()));
      closeLocked();
      return false;
    }

    if (!WinHttpSendRequest(request_, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
      err = "WinHttpSendRequest failed: " + std::to_string(static_cast<long>(GetLastError()));
      closeLocked();
      return false;
    }

    if (!WinHttpReceiveResponse(request_, nullptr)) {
      err = "WinHttpReceiveResponse failed: " + std::to_string(static_cast<long>(GetLastError()));
      closeLocked();
      return false;
    }

    websocket_ = WinHttpWebSocketCompleteUpgrade(request_, 0);
    if (!websocket_) {
      err = "WinHttpWebSocketCompleteUpgrade failed: " + std::to_string(static_cast<long>(GetLastError()));
      closeLocked();
      return false;
    }

    WinHttpCloseHandle(request_);
    request_ = nullptr;
    return true;
  }

  void closeLocked() {
    if (websocket_) {
      WinHttpWebSocketClose(websocket_, WINHTTP_WEB_SOCKET_SUCCESS_CLOSE_STATUS, nullptr, 0);
      WinHttpCloseHandle(websocket_);
      websocket_ = nullptr;
    }
    if (request_) {
      WinHttpCloseHandle(request_);
      request_ = nullptr;
    }
    if (connection_) {
      WinHttpCloseHandle(connection_);
      connection_ = nullptr;
    }
    if (session_) {
      WinHttpCloseHandle(session_);
      session_ = nullptr;
    }
  }

  std::string host_;
  int port_ = 16622;
  std::string path_ = "/";
  bool secure_ = false;
  int timeoutMs_ = 2000;
  mutable std::mutex m_;
  HINTERNET session_ = nullptr;
  HINTERNET connection_ = nullptr;
  HINTERNET request_ = nullptr;
  HINTERNET websocket_ = nullptr;
};

std::unique_ptr<CompanionWsClient> g_companionWs;

std::string nowIso() {
  SYSTEMTIME st{};
  GetLocalTime(&st);
  std::ostringstream oss;
  oss << std::setfill('0')
      << std::setw(4) << st.wYear << "-"
      << std::setw(2) << st.wMonth << "-"
      << std::setw(2) << st.wDay << " "
      << std::setw(2) << st.wHour << ":"
      << std::setw(2) << st.wMinute << ":"
      << std::setw(2) << st.wSecond << "."
      << std::setw(3) << st.wMilliseconds;
  return oss.str();
}

std::string sanitizeForLog(std::string s, size_t maxLen = 220) {
  for (char& ch : s) {
    if (ch == '\r' || ch == '\n' || ch == '\t') ch = ' ';
  }
  if (s.size() > maxLen) s = s.substr(0, maxLen) + "...";
  return s;
}

void logInfo(const std::string& msg) {
  std::lock_guard<std::mutex> lock(g_logMutex);
  std::cout << "[" << nowIso() << "] " << msg << std::endl;
}

void logError(const std::string& msg) {
  std::lock_guard<std::mutex> lock(g_logMutex);
  std::cerr << "[" << nowIso() << "] ERROR: " << msg << std::endl;
}

std::wstring utf8ToWide(const std::string& s) {
  if (s.empty()) return std::wstring();
  const int needed = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
  if (needed <= 0) return std::wstring();
  std::wstring out(static_cast<size_t>(needed), L'\0');
  MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &out[0], needed);
  out.resize(static_cast<size_t>(needed - 1));
  return out;
}

std::string wideToUtf8(const wchar_t* ws) {
  if (!ws) return {};
  const int needed = WideCharToMultiByte(CP_UTF8, 0, ws, -1, nullptr, 0, nullptr, nullptr);
  if (needed <= 0) return {};
  std::string out(static_cast<size_t>(needed), '\0');
  WideCharToMultiByte(CP_UTF8, 0, ws, -1, &out[0], needed, nullptr, nullptr);
  out.resize(static_cast<size_t>(needed - 1));
  return out;
}

std::string trimLine(std::string s) {
  while (!s.empty() && (s.back() == '\r' || s.back() == '\n' || s.back() == ' ' || s.back() == '\t')) s.pop_back();
  size_t i = 0;
  while (i < s.size() && (s[i] == ' ' || s[i] == '\t')) i++;
  return s.substr(i);
}

std::string getExecutableDir() {
  char path[MAX_PATH] = { 0 };
  DWORD len = GetModuleFileNameA(nullptr, path, MAX_PATH);
  if (len == 0 || len >= MAX_PATH) return ".";
  std::string full(path, len);
  size_t pos = full.find_last_of("\\/");
  if (pos == std::string::npos) return ".";
  return full.substr(0, pos);
}

bool sendRawLine(SOCKET sock, const std::string& line) {
  const std::string payload = line + "\n";
  size_t sent = 0;
  while (sent < payload.size()) {
    const int n = send(sock, payload.data() + sent, static_cast<int>(payload.size() - sent), 0);
    if (n == SOCKET_ERROR || n == 0) return false;
    sent += static_cast<size_t>(n);
  }
  return true;
}

bool sendJson(SOCKET sock, const json& obj) {
  std::lock_guard<std::mutex> lock(g_sendMutex);
  return sendRawLine(sock, obj.dump());
}

void removeClient(SOCKET sock) {
  std::lock_guard<std::mutex> lock(g_clientsMutex);
  auto it = std::remove(g_clients.begin(), g_clients.end(), sock);
  if (it != g_clients.end()) g_clients.erase(it, g_clients.end());
}

void broadcastJson(const json& obj) {
  std::vector<SOCKET> snapshot;
  {
    std::lock_guard<std::mutex> lock(g_clientsMutex);
    snapshot = g_clients;
  }

  std::vector<SOCKET> dead;
  for (SOCKET s : snapshot) {
    if (!sendJson(s, obj)) dead.push_back(s);
  }
  for (SOCKET s : dead) removeClient(s);
}

bool sendToCompanionWs(const json& obj, std::string& err) {
  if (!g_companionWs) {
    err = "Companion WS is not configured";
    return false;
  }
  return g_companionWs->sendJson(obj, err);
}

bool sendTextToCompanionWs(const std::string& text, std::string& err) {
  if (!g_companionWs) {
    err = "Companion WS is not configured";
    return false;
  }
  return g_companionWs->sendText(text, err);
}

bool isHttpVerb(const std::string& token) {
  return token == "GET" || token == "POST" || token == "PUT" || token == "PATCH" || token == "DELETE";
}

bool sendToCompanionHttp(const std::string& rawCommand, std::string& err) {
  std::string method = "GET";
  std::string pathOrUrl = rawCommand;
  std::string body;

  auto sp = rawCommand.find(' ');
  if (sp != std::string::npos) {
    std::string first = rawCommand.substr(0, sp);
    if (isHttpVerb(first)) {
      method = first;
      std::string rest = trimLine(rawCommand.substr(sp + 1));
      auto sp2 = rest.find(' ');
      if (sp2 == std::string::npos) {
        pathOrUrl = rest;
      } else {
        pathOrUrl = trimLine(rest.substr(0, sp2));
        body = trimLine(rest.substr(sp2 + 1));
      }
    }
  }

  if (pathOrUrl.empty()) {
    err = "empty companion HTTP path";
    return false;
  }

  std::string url = pathOrUrl;
  if (pathOrUrl.rfind("http://", 0) != 0 && pathOrUrl.rfind("https://", 0) != 0) {
    if (!pathOrUrl.empty() && pathOrUrl[0] != '/') pathOrUrl = "/" + pathOrUrl;
    url = g_cfg.companionHttpBase + pathOrUrl;
  }

  std::wstring wurl = utf8ToWide(url);
  URL_COMPONENTS uc{};
  uc.dwStructSize = sizeof(uc);
  uc.dwSchemeLength = static_cast<DWORD>(-1);
  uc.dwHostNameLength = static_cast<DWORD>(-1);
  uc.dwUrlPathLength = static_cast<DWORD>(-1);
  uc.dwExtraInfoLength = static_cast<DWORD>(-1);

  if (!WinHttpCrackUrl(wurl.c_str(), 0, 0, &uc)) {
    err = "WinHttpCrackUrl failed: " + std::to_string(static_cast<long>(GetLastError()));
    return false;
  }

  std::wstring host(uc.lpszHostName, uc.dwHostNameLength);
  std::wstring path(uc.lpszUrlPath ? uc.lpszUrlPath : L"/", uc.dwUrlPathLength);
  if (uc.lpszExtraInfo && uc.dwExtraInfoLength > 0) {
    path.append(uc.lpszExtraInfo, uc.dwExtraInfoLength);
  }
  if (path.empty()) path = L"/";

  HINTERNET session = WinHttpOpen(L"OnAirBridge/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
  if (!session) {
    err = "WinHttpOpen failed: " + std::to_string(static_cast<long>(GetLastError()));
    return false;
  }
  WinHttpSetTimeouts(session, g_cfg.companionWsTimeoutMs, g_cfg.companionWsTimeoutMs, g_cfg.companionWsTimeoutMs, g_cfg.companionWsTimeoutMs);

  HINTERNET connect = WinHttpConnect(session, host.c_str(), uc.nPort, 0);
  if (!connect) {
    err = "WinHttpConnect failed: " + std::to_string(static_cast<long>(GetLastError()));
    WinHttpCloseHandle(session);
    return false;
  }

  std::wstring wmethod = utf8ToWide(method);
  DWORD flags = (uc.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0;
  HINTERNET request = WinHttpOpenRequest(connect, wmethod.c_str(), path.c_str(), nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
  if (!request) {
    err = "WinHttpOpenRequest failed: " + std::to_string(static_cast<long>(GetLastError()));
    WinHttpCloseHandle(connect);
    WinHttpCloseHandle(session);
    return false;
  }

  LPCWSTR headers = body.empty() ? WINHTTP_NO_ADDITIONAL_HEADERS : L"Content-Type: application/json\r\n";
  LPVOID bodyPtr = body.empty() ? WINHTTP_NO_REQUEST_DATA : reinterpret_cast<LPVOID>(const_cast<char*>(body.data()));
  DWORD bodyLen = body.empty() ? 0 : static_cast<DWORD>(body.size());
  if (!WinHttpSendRequest(request, headers, static_cast<DWORD>(-1L), bodyPtr, bodyLen, bodyLen, 0)) {
    err = "WinHttpSendRequest failed: " + std::to_string(static_cast<long>(GetLastError()));
    WinHttpCloseHandle(request);
    WinHttpCloseHandle(connect);
    WinHttpCloseHandle(session);
    return false;
  }
  if (!WinHttpReceiveResponse(request, nullptr)) {
    err = "WinHttpReceiveResponse failed: " + std::to_string(static_cast<long>(GetLastError()));
    WinHttpCloseHandle(request);
    WinHttpCloseHandle(connect);
    WinHttpCloseHandle(session);
    return false;
  }

  DWORD status = 0;
  DWORD statusSize = sizeof(status);
  WinHttpQueryHeaders(request, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX, &status, &statusSize, WINHTTP_NO_HEADER_INDEX);

  WinHttpCloseHandle(request);
  WinHttpCloseHandle(connect);
  WinHttpCloseHandle(session);

  if (status < 200 || status >= 300) {
    err = "Companion HTTP status: " + std::to_string(static_cast<long>(status));
    return false;
  }
  return true;
}

void publishEvent(const json& obj) {
  if (g_cfg.companionTransport == "ws") {
    std::string err;
    if (!sendToCompanionWs(obj, err)) {
      logError("companion ws send failed: " + err);
    }
    return;
  }
  broadcastJson(obj);
}

void tryForwardOnAirBodyToCompanion(const std::string& body) {
  if (!g_cfg.onAirToCompanionEnabled) return;

  const std::string& prefix = g_cfg.onAirToCompanionPrefix;
  if (!prefix.empty()) {
    if (body.size() < prefix.size()) return;
    if (body.compare(0, prefix.size(), prefix) != 0) return;
  }

  std::string payload = body;
  if (!prefix.empty()) payload = body.substr(prefix.size());
  payload = trimLine(payload);
  if (payload.empty()) return;

  std::string err;
  bool ok = false;
  if (g_cfg.companionCommandTransport == "http") {
    ok = sendToCompanionHttp(payload, err);
  } else {
    ok = sendTextToCompanionWs(payload, err);
  }

  if (!ok) {
    logError("onair->companion forward failed transport=" + g_cfg.companionCommandTransport +
             " error=" + err + " payload=" + sanitizeForLog(payload));
    return;
  }
  logInfo("onair->companion forwarded transport=" + g_cfg.companionCommandTransport +
          " payload=" + sanitizeForLog(payload));
}

Config loadConfig(const std::string& filename) {
  Config cfg;
  std::ifstream file(filename);
  if (!file) return cfg;

  try {
    json j;
    file >> j;
    cfg.listenIp = j.value("listen_ip", cfg.listenIp);
    cfg.listenPort = j.value("listen_port", cfg.listenPort);
    cfg.localQueue = j.value("local_queue", cfg.localQueue);
    cfg.onAirMachine = j.value("onair_machine", cfg.onAirMachine);
    cfg.onAirQueue = j.value("onair_queue", cfg.onAirQueue);
    cfg.onAirSubject = j.value("onair_subject", cfg.onAirSubject);
    cfg.pollIntervalMs = j.value("poll_interval_ms", cfg.pollIntervalMs);
    cfg.sendTimeoutMs = j.value("send_timeout_ms", cfg.sendTimeoutMs);
    cfg.tcpNoDelay = j.value("tcp_nodelay", cfg.tcpNoDelay);
    cfg.tcpKeepAlive = j.value("tcp_keepalive", cfg.tcpKeepAlive);
    cfg.socketSendBuf = j.value("socket_send_buf", cfg.socketSendBuf);
    cfg.socketRecvBuf = j.value("socket_recv_buf", cfg.socketRecvBuf);
    cfg.companionTransport = j.value("companion_transport", cfg.companionTransport);
    cfg.companionWsHost = j.value("companion_ws_host", cfg.companionWsHost);
    cfg.companionWsPort = j.value("companion_ws_port", cfg.companionWsPort);
    cfg.companionWsPath = j.value("companion_ws_path", cfg.companionWsPath);
    cfg.companionWsSecure = j.value("companion_ws_secure", cfg.companionWsSecure);
    cfg.companionWsTimeoutMs = j.value("companion_ws_timeout_ms", cfg.companionWsTimeoutMs);
    cfg.onAirToCompanionEnabled = j.value("onair_to_companion_enabled", cfg.onAirToCompanionEnabled);
    cfg.onAirToCompanionPrefix = j.value("onair_to_companion_prefix", cfg.onAirToCompanionPrefix);
    cfg.companionCommandTransport = j.value("companion_command_transport", cfg.companionCommandTransport);
    cfg.companionHttpBase = j.value("companion_http_base", cfg.companionHttpBase);
  } catch (const std::exception& e) {
    std::cerr << "Config parse error: " << e.what() << std::endl;
  }
  return cfg;
}

void tuneSocketLowLatency(SOCKET s) {
  if (g_cfg.tcpNoDelay) {
    BOOL one = TRUE;
    setsockopt(s, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char*>(&one), sizeof(one));
  }
  if (g_cfg.tcpKeepAlive) {
    BOOL one = TRUE;
    setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, reinterpret_cast<const char*>(&one), sizeof(one));
  }
  if (g_cfg.socketSendBuf > 0) {
    int v = g_cfg.socketSendBuf;
    setsockopt(s, SOL_SOCKET, SO_SNDBUF, reinterpret_cast<const char*>(&v), sizeof(v));
  }
  if (g_cfg.socketRecvBuf > 0) {
    int v = g_cfg.socketRecvBuf;
    setsockopt(s, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<const char*>(&v), sizeof(v));
  }
}

void setBridgeState(bool connected, const std::string& err = {}) {
  bool prev = g_bridgeConnected.exchange(connected);
  {
    std::lock_guard<std::mutex> lock(g_stateMutex);
    g_lastBridgeError = err;
  }
  if (prev != connected || !err.empty()) {
    publishEvent({
      {"type", "bridge_state"},
      {"connected", connected},
      {"error", err},
      {"ts_unix_ms", static_cast<long long>(GetTickCount64())}
    });
  }
}

std::shared_ptr<SendRequest> enqueueSendRequest(
  const std::string& command,
  const std::string& toMachine,
  const std::string& toQueue,
  const std::string& subject,
  long notify
) {
  auto req = std::make_shared<SendRequest>();
  req->id = g_messageId.fetch_add(1);
  req->command = command;
  req->toMachine = toMachine;
  req->toQueue = toQueue;
  req->subject = subject;
  req->notify = notify;

  {
    std::lock_guard<std::mutex> lock(g_sendQueueMutex);
    g_sendQueue.push_back(req);
  }
  g_sendQueueCv.notify_one();
  return req;
}

bool waitSendResult(const std::shared_ptr<SendRequest>& req, int timeoutMs) {
  std::unique_lock<std::mutex> lock(req->m);
  return req->cv.wait_for(lock, std::chrono::milliseconds(timeoutMs), [&] { return req->done; });
}

std::shared_ptr<SendRequest> popSendRequest() {
  std::lock_guard<std::mutex> lock(g_sendQueueMutex);
  if (g_sendQueue.empty()) return nullptr;
  auto req = g_sendQueue.front();
  g_sendQueue.pop_front();
  return req;
}

void finishRequest(const std::shared_ptr<SendRequest>& req, bool ok, long slErr, long slRes, const std::string& error) {
  {
    std::lock_guard<std::mutex> lock(req->m);
    req->done = true;
    req->ok = ok;
    req->slErrorCode = slErr;
    req->slResultCode = slRes;
    req->error = error;
  }
  req->cv.notify_all();
}

void processIncomingMessages(const CComQIPtr<ISLMSQueue>& queue) {
  while (g_running.load()) {
    SLMSErrorCode err = mcUnknown;
    HRESULT slResult = E_FAIL;
    SLMSMessageType mt = mtIgnore;
    long messId = 0, replyTo = 0, sentLo = 0, sentHi = 0;
    CComBSTR fromMachine, fromQueue, subject, body;

    HRESULT hr = queue->SLGetMessage(
      &err, &slResult, &mt, &messId, &replyTo, &sentLo, &sentHi,
      &fromMachine, &fromQueue, &subject, &body
    );

  if (FAILED(hr) || err == mcNoMessage) {
    return;
  }

    json msg = {
      {"type", "onair_message"},
      {"message_type", static_cast<int>(mt)},
      {"message_id", messId},
      {"reply_to", replyTo},
      {"sent_lo", sentLo},
      {"sent_hi", sentHi},
      {"from_machine", wideToUtf8(fromMachine.m_str)},
      {"from_queue", wideToUtf8(fromQueue.m_str)},
      {"subject", wideToUtf8(subject.m_str)},
      {"body", wideToUtf8(body.m_str)},
      {"sl_error_code", static_cast<int>(err)},
      {"sl_result_code", static_cast<long long>(slResult)}
    };
    logInfo(
      "onair_message from=" + sanitizeForLog(msg["from_machine"].get<std::string>()) + "/" +
      sanitizeForLog(msg["from_queue"].get<std::string>()) +
      " subject=" + sanitizeForLog(msg["subject"].get<std::string>()) +
      " body=" + sanitizeForLog(msg["body"].get<std::string>())
    );
    tryForwardOnAirBodyToCompanion(msg["body"].get<std::string>());
    publishEvent(msg);
  }
}

void bridgeWorker() {
  HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
  if (FAILED(hr)) {
    setBridgeState(false, "CoInitializeEx failed");
    return;
  }

  CComPtr<ISLMSConnection> conn;
  CComQIPtr<ISLMSQueue> queue;

  auto connectBridge = [&]() -> bool {
    conn.Release();
    queue.Release();

    conn.CoCreateInstance(CLSID_SLMSConnection);
    if (!conn) {
      setBridgeState(false, "Failed to create CLSID_SLMSConnection");
      return false;
    }

    SLMSErrorCode ec = mcUnknown;
    HRESULT rc = E_FAIL;
    CComBSTR qName(utf8ToWide(g_cfg.localQueue).c_str());
    CComPtr<IUnknown> unk;
    HRESULT cHr = conn->CreateQueue(&ec, &rc, qName, 0, 0, 0, 0, &unk);
    if (FAILED(cHr) || ec != mcSuccess || !unk) {
      logError("CreateQueue failed queue=" + g_cfg.localQueue +
               " ec=" + std::to_string(static_cast<long>(ec)) +
               " rc=" + std::to_string(static_cast<long>(rc)) +
               " hr=" + std::to_string(static_cast<long>(cHr)));
      setBridgeState(false, "CreateQueue failed");
      return false;
    }

    queue = unk;
    if (!queue) {
      setBridgeState(false, "Failed to query ISLMSQueue");
      return false;
    }

    setBridgeState(true, {});
    logInfo("bridge connected local_queue=" + g_cfg.localQueue);
    return true;
  };

  while (g_running.load()) {
    if (!queue) {
      if (!connectBridge()) {
        std::this_thread::sleep_for(2s);
        continue;
      }
    }

    // Process outgoing command queue.
    while (g_running.load()) {
      auto req = popSendRequest();
      if (!req) break;

      CComBSTR toMachine(utf8ToWide(req->toMachine.empty() ? g_cfg.onAirMachine : req->toMachine).c_str());
      CComBSTR toQueue(utf8ToWide(req->toQueue.empty() ? g_cfg.onAirQueue : req->toQueue).c_str());
      CComBSTR subj(utf8ToWide(req->subject.empty() ? g_cfg.onAirSubject : req->subject).c_str());
      CComBSTR body(utf8ToWide(req->command).c_str());

      SLMSErrorCode ec = mcUnknown;
      HRESULT rc = E_FAIL;
      HRESULT sHr = queue->SLSendMessage(&ec, &rc, req->id, 0, req->notify, toMachine, toQueue, subj, body);
      const std::string toMachineUtf8 = wideToUtf8(toMachine.m_str);
      const std::string toQueueUtf8 = wideToUtf8(toQueue.m_str);
      const std::string subjUtf8 = wideToUtf8(subj.m_str);
      const std::string bodyUtf8 = wideToUtf8(body.m_str);

      if (FAILED(sHr) || ec != mcSuccess) {
        logError(
          "send failed id=" + std::to_string(req->id) +
          " to=" + sanitizeForLog(toMachineUtf8) + "/" + sanitizeForLog(toQueueUtf8) +
          " subject=" + sanitizeForLog(subjUtf8) +
          " body=" + sanitizeForLog(bodyUtf8) +
          " ec=" + std::to_string(static_cast<long>(ec)) +
          " rc=" + std::to_string(static_cast<long>(rc)) +
          " hr=" + std::to_string(static_cast<long>(sHr))
        );
        finishRequest(req, false, static_cast<long>(ec), static_cast<long>(rc), "SLSendMessage failed");
        // Connection might be broken; re-init on next loop.
        if (FAILED(sHr)) {
          queue.Release();
          conn.Release();
          setBridgeState(false, "Bridge disconnected while sending");
          break;
        }
      } else {
        logInfo(
          "send ok id=" + std::to_string(req->id) +
          " to=" + sanitizeForLog(toMachineUtf8) + "/" + sanitizeForLog(toQueueUtf8) +
          " subject=" + sanitizeForLog(subjUtf8) +
          " body=" + sanitizeForLog(bodyUtf8)
        );
        finishRequest(req, true, static_cast<long>(ec), static_cast<long>(rc), {});
      }
    }

    if (!queue) {
      std::this_thread::sleep_for(200ms);
      continue;
    }

    processIncomingMessages(queue);

    const int waitMs = g_cfg.pollIntervalMs > 1 ? g_cfg.pollIntervalMs : 1;
    std::unique_lock<std::mutex> lk(g_sendQueueMutex);
    g_sendQueueCv.wait_for(lk, std::chrono::milliseconds(waitMs));
  }

  setBridgeState(false, "Bridge stopped");
  CoUninitialize();
}

json makeHealthJson() {
  std::string err;
  {
    std::lock_guard<std::mutex> lock(g_stateMutex);
    err = g_lastBridgeError;
  }
  return {
    {"type", "health"},
    {"bridge_connected", g_bridgeConnected.load()},
    {"error", err},
    {"listen_ip", g_cfg.listenIp},
    {"listen_port", g_cfg.listenPort},
    {"local_queue", g_cfg.localQueue},
    {"default_target", {
      {"machine", g_cfg.onAirMachine},
      {"queue", g_cfg.onAirQueue},
      {"subject", g_cfg.onAirSubject}
    }},
    {"companion", {
      {"transport", g_cfg.companionTransport},
      {"ws_connected", g_companionWs ? g_companionWs->isConnected() : false},
      {"ws_host", g_cfg.companionWsHost},
      {"ws_port", g_cfg.companionWsPort},
      {"ws_path", g_cfg.companionWsPath},
      {"ws_secure", g_cfg.companionWsSecure},
      {"onair_to_companion_enabled", g_cfg.onAirToCompanionEnabled},
      {"onair_to_companion_prefix", g_cfg.onAirToCompanionPrefix},
      {"command_transport", g_cfg.companionCommandTransport},
      {"http_base", g_cfg.companionHttpBase}
    }}
  };
}

void handleClient(SOCKET clientSocket) {
  char recvbuf[4096];
  std::string pending;

  while (g_running.load()) {
    int n = recv(clientSocket, recvbuf, sizeof(recvbuf), 0);
    if (n <= 0) break;
    pending.append(recvbuf, recvbuf + n);

    size_t pos = 0;
    while ((pos = pending.find('\n')) != std::string::npos) {
      std::string line = trimLine(pending.substr(0, pos));
      pending.erase(0, pos + 1);
      if (line.empty()) continue;
      logInfo("tcp recv line=" + sanitizeForLog(line));

      json response;
      response["type"] = "response";
      response["ok"] = false;

      try {
        std::string action = "send";
        std::string command;
        std::string toMachine;
        std::string toQueue;
        std::string subject;
        long notify = 0;
        std::string reqId;

        if (!line.empty() && line[0] == '{') {
          json req = json::parse(line);
          reqId = req.value("id", "");
          if (!reqId.empty()) response["id"] = reqId;
          action = req.value("action", "send");

          if (action == "health") {
            logInfo("tcp action=health");
            sendJson(clientSocket, makeHealthJson());
            continue;
          }

          if (action == "companion_send") {
            std::string payload = req.value("payload", "");
            if (payload.empty()) payload = req.value("command", "");
            if (payload.empty()) {
              response["error"] = "payload or command is required for companion_send";
              sendJson(clientSocket, response);
              continue;
            }

            std::string compErr;
            bool ok = false;
            if (g_cfg.companionCommandTransport == "http") {
              ok = sendToCompanionHttp(payload, compErr);
            } else {
              ok = sendTextToCompanionWs(payload, compErr);
            }
            if (!ok) {
              response["error"] = compErr;
              sendJson(clientSocket, response);
              continue;
            }

            response["ok"] = true;
            response["transport"] = g_cfg.companionCommandTransport;
            sendJson(clientSocket, response);
            continue;
          }

          if (action != "send") {
            response["error"] = "Unsupported action. Use send|health|companion_send";
            sendJson(clientSocket, response);
            continue;
          }

          command = req.value("command", "");
          toMachine = req.value("to_machine", "");
          toQueue = req.value("to_queue", "");
          subject = req.value("subject", "");
          notify = req.value("notify", 0L);
        } else {
          // Plain text compatibility for simple TCP clients.
          command = line;
        }

        if (command.empty()) {
          response["error"] = "command is required";
          sendJson(clientSocket, response);
          continue;
        }

        auto req = enqueueSendRequest(command, toMachine, toQueue, subject, notify);
        logInfo(
          "tcp action=send queued id=" + std::to_string(req->id) +
          " to=" + sanitizeForLog(toMachine.empty() ? g_cfg.onAirMachine : toMachine) + "/" +
          sanitizeForLog(toQueue.empty() ? g_cfg.onAirQueue : toQueue) +
          " subject=" + sanitizeForLog(subject.empty() ? g_cfg.onAirSubject : subject) +
          " body=" + sanitizeForLog(command)
        );
        if (!waitSendResult(req, g_cfg.sendTimeoutMs)) {
          response["error"] = "send timeout";
          logError("send timeout id=" + std::to_string(req->id));
          sendJson(clientSocket, response);
          continue;
        }

        response["ok"] = req->ok;
        response["message_id"] = req->id;
        response["sl_error_code"] = req->slErrorCode;
        response["sl_result_code"] = req->slResultCode;
        if (!req->ok) response["error"] = req->error;
        sendJson(clientSocket, response);
      } catch (const std::exception& e) {
        response["error"] = e.what();
        logError(std::string("request parse/handle error: ") + e.what());
        sendJson(clientSocket, response);
      }
    }
  }

  closesocket(clientSocket);
  removeClient(clientSocket);
}

} // namespace

int main() {
  const std::string exeDir = getExecutableDir();
  const std::string configPath = exeDir + "\\config.json";
  g_cfg = loadConfig(configPath);
  logInfo("Config path: " + configPath);
  if (g_cfg.companionTransport == "ws" || g_cfg.onAirToCompanionEnabled) {
    g_companionWs = std::make_unique<CompanionWsClient>(g_cfg);
  }

  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
    logError("WSAStartup failed");
    return 1;
  }

  SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (listenSocket == INVALID_SOCKET) {
    logError("socket() failed: " + std::to_string(WSAGetLastError()));
    WSACleanup();
    return 1;
  }
  {
    BOOL reuse = TRUE;
    setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&reuse), sizeof(reuse));
  }

  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(static_cast<u_short>(g_cfg.listenPort));
  if (inet_pton(AF_INET, g_cfg.listenIp.c_str(), &addr.sin_addr) != 1) {
    logError("Invalid listen_ip: " + g_cfg.listenIp);
    closesocket(listenSocket);
    WSACleanup();
    return 1;
  }

  if (bind(listenSocket, reinterpret_cast<SOCKADDR*>(&addr), sizeof(addr)) == SOCKET_ERROR) {
    logError("bind() failed: " + std::to_string(WSAGetLastError()));
    closesocket(listenSocket);
    WSACleanup();
    return 1;
  }

  if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
    logError("listen() failed: " + std::to_string(WSAGetLastError()));
    closesocket(listenSocket);
    WSACleanup();
    return 1;
  }

  logInfo("Bridge server listening on " + g_cfg.listenIp + ":" + std::to_string(g_cfg.listenPort));
  logInfo("Default OnAir target: machine=" + g_cfg.onAirMachine
    + " queue=" + g_cfg.onAirQueue
    + " subject=" + g_cfg.onAirSubject);
  logInfo("Companion transport=" + g_cfg.companionTransport +
    (g_cfg.companionTransport == "ws"
      ? (" ws://" + g_cfg.companionWsHost + ":" + std::to_string(g_cfg.companionWsPort) + g_cfg.companionWsPath)
      : " (tcp clients)"));
  logInfo("onair_to_companion_enabled=" + std::string(g_cfg.onAirToCompanionEnabled ? "true" : "false") +
    " prefix=" + g_cfg.onAirToCompanionPrefix +
    " transport=" + g_cfg.companionCommandTransport);

  std::thread worker(bridgeWorker);

  while (g_running.load()) {
    sockaddr_in clientAddr{};
    int clientAddrSize = sizeof(clientAddr);
    SOCKET clientSocket = accept(listenSocket, reinterpret_cast<SOCKADDR*>(&clientAddr), &clientAddrSize);
    if (clientSocket == INVALID_SOCKET) {
      const int e = WSAGetLastError();
      if (e == WSAEINTR) break;
      logError("accept() failed: " + std::to_string(e));
      std::this_thread::sleep_for(100ms);
      continue;
    }

    {
      std::lock_guard<std::mutex> lock(g_clientsMutex);
      g_clients.push_back(clientSocket);
    }
    tuneSocketLowLatency(clientSocket);
    logInfo("tcp client connected");

    sendJson(clientSocket, {
      {"type", "hello"},
      {"protocol", "onair-bridge-jsonl-v1"},
      {"bridge_connected", g_bridgeConnected.load()}
    });
    std::thread(handleClient, clientSocket).detach();
  }

  g_running.store(false);
  g_sendQueueCv.notify_all();
  if (worker.joinable()) worker.join();

  closesocket(listenSocket);
  WSACleanup();
  return 0;
}
