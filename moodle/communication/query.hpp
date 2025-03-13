#pragma once

#include <curl/curl.h>
#include <iostream>
#include <string>

/* server responce process callback */
static std::size_t write_callback(void *contents, std::size_t size,
                                  std::size_t nmemb, void *userp) {
  ((std::string *)userp)->append((char *)contents, size * nmemb);
  return size * nmemb;
}

bool send_query(const std::string &server_url, const std::string &query,
                std::string &response) {
  /* initialize context */
  auto *curl = curl_easy_init();
  if (!curl) {
    return false;
  }

  /* header */
  struct curl_slist *headers = NULL;
  headers = curl_slist_append(headers, "Accept: application/json");
  headers = curl_slist_append(headers, "Content-Type: application/json");
  headers = curl_slist_append(headers, "charset: utf-8");

  /* query setup */
  curl_easy_setopt(curl, CURLOPT_URL, server_url.c_str());
  curl_easy_setopt(curl, CURLOPT_POST, 1L);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, query.c_str());
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

  /* post query */
  auto res = curl_easy_perform(curl);

  /* status */
  if (res != CURLE_OK) {
    std::cerr << "[-] error (post query failed): " << curl_easy_strerror(res)
              << std::endl;

    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
    return false;
  }

  /* release resources */
  curl_easy_cleanup(curl);
  curl_slist_free_all(headers);
  return true;
}
