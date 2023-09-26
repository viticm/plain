/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plain )
 * $Id all.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2023- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2023/07/07 17:50
 * @uses The framework all inlcudes.
 *       This group defines just can define one.
 *             group1: PLAIN_OPEN_ICOP|PLAIN_OPEN_EPOLL
 *
 * ------------------------------------------------------------------------------
 *  Licensing information can be found at the end of the file.
 * ------------------------------------------------------------------------------
*/
#ifndef PLAIN_H_
#define PLAIN_H_

/* basic */
#include "plain/basic/type/variable.h"
#include "plain/basic/copyable.h"
#include "plain/basic/noncopyable.h"
#include "plain/basic/async_logger.h"
#include "plain/basic/base64.h"
#include "plain/basic/global.h"
#include "plain/basic/io.h"
#include "plain/basic/logger.h"
#include "plain/basic/md5.h"
#include "plain/basic/monitor.h"
#include "plain/basic/singleton.h"
#include "plain/basic/utility.h"
#include "plain/basic/time.h"
#include "plain/basic/utility.h"

/* engine */
#include "plain/engine/timer.h"
#include "plain/engine/timer_queue.h"
#include "plain/engine/kernel.h"

/* concurrency */
#include "plain/concurrency/result/awaitable.h"
#include "plain/concurrency/result/basic.h"
#include "plain/concurrency/result/lazy.h"
#include "plain/concurrency/result/lazy_awaitable.h"
#include "plain/concurrency/result/make.h"
#include "plain/concurrency/result/promise.h"
#include "plain/concurrency/result/resume_on.h"
#include "plain/concurrency/result/shared.h"
#include "plain/concurrency/result/shared_awaitable.h"
#include "plain/concurrency/result/when.h"
#include "plain/concurrency/result/generator.h"
#include "plain/concurrency/result/make.h"
#include "plain/concurrency/executor/basic.h"
#include "plain/concurrency/executor/inline.h"
#include "plain/concurrency/executor/manual.h"
#include "plain/concurrency/executor/thread.h"
#include "plain/concurrency/executor/thread_pool.h"
#include "plain/concurrency/executor/worker_thread.h"

/* file */
#include "plain/file/api.h"
#include "plain/file/ini.h"
#include "plain/file/tab.h"
#include "plain/file/library.h"

#endif //PLAIN_H_

/*
MIT License

Copyright (c) 2023 viticm

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
