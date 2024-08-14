/****************************************************************************
 * MIT License
 *
 * Copyright (c) 2024 İsmail Çağdaş Yılmaz
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 ****************************************************************************/

#include "../../../Vocabulary/include/Core/uWebSockets.h"

namespace Vocabulary
{
    uWebSockets::uWebSockets(int port_num, unsigned int num_of_threads)
            : port_num{port_num}, num_of_threads{num_of_threads}
    {
        threads.resize(num_of_threads);
    }

    void uWebSockets::handleGetRequest(uWS::HttpResponse<false> *res, uWS::HttpRequest *req)
    {
        setDefaultHeaders(res);

        std::string_view query = req->getQuery();
        auto queryParams = parseQueryString(query);

        if(!query.empty()) {
            std::string username = queryParams["username"];
            std::string type = queryParams["type"];

            VOCABULARY_CLIENT_INFO("A GET message is received for URI {}, username: {}, type: {}.",
                                   req->getUrl(), username, type);

            if(mapStringToVocabularyType.count(type)) {
                VocabularyType vocabularyType = mapStringToVocabularyType[type];

                std::size_t number_of_words = get_number_of_words(vocabularyType);

                std::vector<int> indexes = VocabularyDatabaseSQLite::getInstance().get_word_indexes(username, type);

                // Using initializer lists
                json size_and_indexes = {
                        {"size", number_of_words},
                        {"OptionIndex_1", indexes[0]},
                        {"OptionIndex_2", indexes[1]},
                        {"OptionIndex_3", indexes[2]},
                        {"OptionIndex_4", indexes[3]},
                };

                std::string responseString = size_and_indexes.dump();
                res->writeHeader("Content-Type", "application/json");
                res->end(responseString);
            }
            else{
                VOCABULARY_CLIENT_ERROR("This vocabulary type is not implemented in backend yet!");
                res->writeStatus("500 Internal Server Error");
                res->end("This vocabulary type is not implemented in backend yet!", true);
            }

        } else {
            res->writeStatus("500 Internal Server Error");
            res->end("Query parameter is absent!", true);
        }
    }

    void uWebSockets::handlePostRequest(uWS::HttpResponse<false>* res, uWS::HttpRequest* req) {
        setDefaultHeaders(res);

        // Attach an abort handler to ensure response is ended properly
        res->onAborted([res]() {
            VOCABULARY_CLIENT_ERROR("Connection aborted by client!");
        });

        std::size_t length = std::stoul(std::string(req->getHeader("content-length")));
        //VOCABULARY_CLIENT_INFO("Content-Length: {}.", length);
        if (length == 0)
        {
            VOCABULARY_CLIENT_ERROR("Not Acceptable, empty request body!");
            res->writeStatus("411 Length Required")->end("The request could not be understood by the server due to length of the content.");
            return;
        }

        // Create a buffer to store the incoming data
        std::string buffer;
        /* Move it to storage of lambda */
        res->onData([req, res, buffer = std::move(buffer)](std::string_view data, bool last) mutable {
            /* Mutate the captured data */
            buffer.append(data.data(), data.length());

            if (last) {
                // Parse the JSON data
                json post_data = json::parse(buffer);

                // Extract the individual fields
                std::string username = post_data.at("username").get<std::string>();
                std::string type = post_data.at("type").get<std::string>();
                std::string option = post_data.at("mode").get<std::string>();
                std::size_t index = post_data.at("index").get<std::size_t>();

                // Log the received data
                VOCABULARY_CLIENT_INFO("A POST message is received for URI: {}, username: {}, type: {}, mode: {}, index: {}",
                                       req->getUrl(), username, type, option, index);

                VocabularyType vocabularyType = mapStringToVocabularyType[type];

                VocabularyDatabaseSQLite::getInstance().update(username, type, option, index);

                if (option == "2" || option == "4") {
                    int index_unordered = ThreadSafe_JSON_Users::getInstance().check_and_read(username, type, option, index, get_number_of_words(vocabularyType));

                    if (index_unordered == -1) {
                        ThreadSafe_JSON_Users::getInstance().write_file();
                        index_unordered = ThreadSafe_JSON_Users::getInstance().read(username, type, option, index);
                    }
                    index = index_unordered;
                }

                std::string j_string = get_word_as_json(vocabularyType, index);

                if (j_string.empty()) {
                    VOCABULARY_CLIENT_ERROR("This vocabulary type is not implemented in backend yet!");
                    res->writeStatus("500 Internal Server Error")->end("This vocabulary type is not implemented in backend yet!", true);
                    return;
                }

                res->writeHeader("Content-Type", "application/json");
                res->end(j_string);
            }
        });
    }

    void uWebSockets::handleOptionsRequest(uWS::HttpResponse<false>* res) {
        setDefaultHeaders(res);
        res->end();
    }

    void uWebSockets::run() {
        std::transform(threads.begin(), threads.end(), threads.begin(), [this](std::thread* /*t*/) {
            return new std::thread([this]() {
                auto app = uWS::App();

                // Register all paths for OPTIONS, POST, and GET requests
                for (const auto &path : paths) {
                    app.options(path, [this](uWS::HttpResponse<false> *res, uWS::HttpRequest *req) {
                        Vocabulary::uWebSockets::handleOptionsRequest(res);
                    });

                    app.post(path, [this](uWS::HttpResponse<false> *res, uWS::HttpRequest *req) {
                        Vocabulary::uWebSockets::handlePostRequest(res, req);
                    });

                    app.get(path, [this](uWS::HttpResponse<false> *res, uWS::HttpRequest *req) {
                        Vocabulary::uWebSockets::handleGetRequest(res, req);
                    });
                }

                app.listen(port_num, [this](auto *token) {
                    std::stringstream ss;
                    ss << std::this_thread::get_id();
                    if (token) {
                        VOCABULARY_CORE_INFO("Server is listening on port {} with thread ID: {}.", port_num, ss.str());

                    } else {
                        VOCABULARY_CORE_ERROR("Failed to listen on port {} with thread ID: {}.", port_num, ss.str());
                    }
                }).run();

            });
        });

        std::for_each(threads.begin(), threads.end(), [](std::thread *t) {
            t->join();
        });
    }


}