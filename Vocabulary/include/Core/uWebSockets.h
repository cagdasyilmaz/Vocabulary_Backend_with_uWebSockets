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

#pragma once

#include "Logger.h"
#include "../../../Vocabulary/include/Core/Helper.h"

#include <thread>
#include <cstddef>

using json = nlohmann::json;

namespace Vocabulary {

    class uWebSockets {
    public:
        explicit uWebSockets(int port_num, unsigned int num_of_threads = std::thread::hardware_concurrency());

        void run();

    private:
        // Function to handle GET requests
        static void handleGetRequest(uWS::HttpResponse<false>*, uWS::HttpRequest*);

        // Function to handle POST requests
        static void handlePostRequest(uWS::HttpResponse<false>*, uWS::HttpRequest*);

        // Function to handle OPTIONS requests
        static void handleOptionsRequest(uWS::HttpResponse<false>*);

        int port_num;
        unsigned int num_of_threads;
        std::vector<std::thread*> threads;
    };
}