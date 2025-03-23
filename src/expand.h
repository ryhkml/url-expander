#ifndef EXPAND_H
#define EXPAND_H

// Source: https://curl.se/libcurl/c/CURLOPT_CONNECTTIMEOUT.html
#define DEFAULT_CONNECT_TIMEOUT 10L
// Source: https://curl.se/libcurl/c/CURLOPT_TIMEOUT.html
#define DEFAULT_TIMEOUT 30L
// Source: https://curl.se/libcurl/c/CURLOPT_MAXREDIRS.html
#define DEFAULT_MAX_REDIRS 16
// Source: https://curl.se/libcurl/c/CURLOPT_USERAGENT.html
#define DEFAULT_USER_AGENT \
    "Mozilla/5.0 (Windows NT 10.0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/134.0.0.0 Safari/537.36"

#define GREEN "\033[1;32m"
#define YELLOW "\033[1;33m"
#define RED "\033[1;31m"
#define RESET "\033[0m"

void expand(const char *short_url, long max_redirs, const char *user_agent, const char *cookie);
void init_curl();
void cleanup_curl();

#endif  // EXPAND_H
