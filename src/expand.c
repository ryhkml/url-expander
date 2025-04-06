#include "expand.h"

#include <ctype.h>
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct memory {
    char *response;
    size_t size;
};

static char *mstrdup(const char *value) {
    if (!value) return NULL;
    size_t len = strlen(value) + 1;
    char *new_value = malloc(len);
    if (!new_value) return NULL;
    memcpy(new_value, value, len);
    return new_value;
}

static char *mstrcasestr(const char *haystack, const char *needle) {
    if (!haystack || !needle) return NULL;
    if (!*needle) return (char *)haystack;
    while (*haystack) {
        const char *h = haystack;
        const char *n = needle;
        while (*h && *n && (tolower((unsigned char)*h) == tolower((unsigned char)*n))) {
            h++;
            n++;
        }
        if (!*n) return (char *)haystack;
        haystack++;
    }
    return NULL;
}

// Callback function to write response data into memory
// Source: https://curl.se/libcurl/c/CURLOPT_WRITEFUNCTION.html
static size_t write_cb(void *contents, size_t size, size_t nmemb, void *clientp) {
    size_t realsize = size * nmemb;
    struct memory *mem = (struct memory *)clientp;

    char *ptr = realloc(mem->response, mem->size + realsize + 1);
    if (!ptr) return 0;

    mem->response = ptr;
    memcpy(&(mem->response[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->response[mem->size] = '\0';

    return realsize;
}

static char *extract_meta_refresh_url(const char *html) {
    if (!html) return NULL;
    char *meta = mstrcasestr(html, "<meta http-equiv=\"refresh\"");
    if (!meta) return NULL;
    char *url_start = mstrcasestr(meta, "url=");
    if (url_start) {
        url_start += 4;
        char *url_end = strchr(url_start, '"');
        if (url_end) {
            size_t len = url_end - url_start;
            char *url = malloc(len + 1);
            if (url) {
                memcpy(url, url_start, len);
                url[len] = '\0';
                return url;
            }
        }
    }
    return NULL;
}

void expand(const char *short_url, const struct url_expander_opt *opt_ptr) {
    CURL *curl = curl_easy_init();
    if (!curl) {
        printf("Failed initialize curl\n");
        return;
    }

    char *current_url = mstrdup(short_url);
    if (!current_url) {
        curl_easy_cleanup(curl);
        return;
    }

    const char *HEADER_LIST[] = {
        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8",
        "Accept-Language: en-US,en;q=0.5",
        "Cache-Control: no-cache, no-store, must-revalidate",
        "Connection: keep-alive",
        "Expires: 0",
        NULL,
    };
    struct curl_slist *headers = NULL;
    for (int i = 0; HEADER_LIST[i] != NULL; i++) {
        headers = curl_slist_append(headers, HEADER_LIST[i]);
    }

    struct memory chunk = {
        .response = malloc(1),
        .size = 0,
    };

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);
    curl_easy_setopt(curl, CURLOPT_NOBODY, 0L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, opt_ptr->max_redirs);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, opt_ptr->user_agent);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, DEFAULT_CONNECT_TIMEOUT);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, DEFAULT_TIMEOUT);
    curl_easy_setopt(curl, CURLOPT_DOH_URL, "https://cloudflare-dns.com/dns-query");
    if (opt_ptr->cookie) {
        // Add suffix for ".new" (4 characters) + '\0' = 5
        size_t len_cookie = strlen(opt_ptr->cookie) + 5;
        char *new_cookie = malloc(len_cookie);
        if (new_cookie) {
            snprintf(new_cookie, len_cookie, "%s.new", opt_ptr->cookie);
            curl_easy_setopt(curl, CURLOPT_COOKIEFILE, opt_ptr->cookie);
            curl_easy_setopt(curl, CURLOPT_COOKIEJAR, new_cookie);
            free(new_cookie);
        }
    }
    if (opt_ptr->verbose) curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    int count_redirs = 0;
    long status_code;
    CURLcode res;

    printf("%s--> Accessing%s %s\n", YELLOW, RESET, current_url);

    while (count_redirs < opt_ptr->max_redirs) {
        chunk.response = NULL;
        chunk.size = 0;

        curl_easy_setopt(curl, CURLOPT_URL, current_url);
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "Error response. %s\n", curl_easy_strerror(res));
            break;
        }

        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status_code);
        if (status_code >= 300 && status_code < 400) {
            char *redir_url = NULL;
            res = curl_easy_getinfo(curl, CURLINFO_REDIRECT_URL, &redir_url);
            if (res == CURLE_OK && redir_url) {
                printf("%s--> %u%s %s\n", YELLOW, (unsigned int)status_code, RESET, redir_url);
                free(current_url);
                current_url = mstrdup(redir_url);
                count_redirs++;
            } else {
                printf("There was an error\n");
                break;
            }
        } else if (status_code >= 200 && status_code < 300 && chunk.response) {
            char *meta_url = extract_meta_refresh_url(chunk.response);
            if (meta_url) {
                printf("%s--> Extracting meta refresh%s %s\n", YELLOW, RESET, meta_url);
                free(current_url);
                current_url = meta_url;
                count_redirs++;
            } else {
                printf("%s--> Redirect to%s %s\n", GREEN, RESET, current_url);
                break;
            }
        } else {
            printf("%s--> Redirect to%s %s\n", GREEN, RESET, current_url);
            break;
        }

        if (chunk.response) {
            free(chunk.response);
            chunk.response = NULL;
        }
    }

    if (count_redirs == opt_ptr->max_redirs)
        printf("Max redirects (%ld) reached. Possible redirect loop\n", opt_ptr->max_redirs);

    free(current_url);
    if (chunk.response) free(chunk.response);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
}

void init_curl() { curl_global_init(CURL_GLOBAL_DEFAULT); }

void cleanup_curl() { curl_global_cleanup(); }
