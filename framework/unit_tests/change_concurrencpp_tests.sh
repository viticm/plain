#!/bin/sh

function copyfiles() {
  local header_dir="${HOME}/develop/github/concurrencpp/test/include"
  local source_dir="${HOME}/develop/github/concurrencpp/test/source"
  cp ${source_dir}/tests/executor_tests ./ -r
  cp ${source_dir}/tests/result_tests ./ -r
  cp ${source_dir}/tests/timer_tests ./ -r
  cp ${header_dir}/infra/assertions.h ./
  mkdir -p ./util
  cp ${header_dir}/utils/custom_exception.h ./util
  cp ${header_dir}/utils/test_generators.h ./util
  cp ${header_dir}/utils/test_ready_lazy_result.h ./util
  cp ${header_dir}/utils/executor_shutdowner.h ./util
  cp ${header_dir}/utils/random.h ./util
  cp ${header_dir}/utils/test_ready_result.h ./util
  cp ${header_dir}/utils/throwing_executor.h ./util
  cp ${header_dir}/utils/wait_context.h ./util
  cp ${header_dir}/utils/test_thread_callbacks.h ./util
}

function replace_class() {
  local file=${1}
  local members=`cat $file | grep -o -E "m_\w*" | sort | uniq`
  echo 'file: '$file
  for member in $members
  do
    local new_name=`echo ${member}_ | sed 's;m_;;g'`
    echo $new_name
    sed -i "s; ${member}; ${new_name};g" $file
    sed -i "s;<${member};<${new_name};g" $file
    sed -i "s;(${member};(${new_name};g" $file
  done
}

copyfiles

curdir=`pwd`
cppfiles=`find ${curdir} -name "*.cpp"`

for file in $cppfiles
do
  newfile=`echo $file | sed 's;.cpp;.cc;g'`
  mv $file $newfile
done

files=`find ${curdir} -name "*.h" -o -name "*.cc"`

for file in $files
do
  sed -i 's;concurrencpp/concurrencpp.h;plain/all.h;g' $file
  sed -i 's;utils/object_observer.h;object_observer.h;g' $file
  sed -i 's;infra/tester.h;gtest/gtest.h;g' $file
  sed -i 's;infra/assertions.h;assertions.h;g' $file
  sed -i 's;CONCURRENCPP_;PLAIN_CORE_TEST_;g' $file
  sed -i 's;_H;_H_;g' $file
  sed -i 's;utils/;util/;g' $file

  sed -i 's;shared_result<;concurrency::result::Shared<;g' $file
  sed -i 's;concurrencpp::details::thread;plain::thread;g' $file
  sed -i 's;concurrencpp::inline_executor;plain::concurrency::executor::Inline;g' $file
  sed -i 's;<inline_executor;<plain::concurrency::executor::Inline;g' $file
  sed -i 's;concurrencpp::manual_executor;plain::concurrency::executor::Manual;g' $file
  sed -i 's;<manual_executor;<plain::concurrency::executor::Manual;g' $file
  sed -i 's;concurrencpp::thread_executor;plain::concurrency::executor::Thread;g' $file
  sed -i 's;<thread_executor;<plain::concurrency::executor::Thread;g' $file
  sed -i 's;concurrencpp::thread_pool_executor;plain::concurrency::executor::ThreadPool;g' $file
  sed -i 's;<thread_pool_executor;<plain::concurrency::executor::ThreadPool;g' $file
  sed -i 's;concurrencpp::worker_thread_executor;plain::concurrency::executor::WorkerThread;g' $file
  sed -i 's;<worker_thread_executor;<plain::concurrency::executor::WorkerThread;g' $file
  sed -i 's;lazy_result<;concurrency::LazyResult<;g' $file
  sed -i 's;result<;concurrency::Result<;g' $file
  sed -i 's;result_promise<;concurrency::ResultPromise<;g' $file
  sed -i 's;shared_result<;concurrency::result::Shared<;g' $file

  sed -i 's;\&\& ; \&\&;g' $file
  sed -i 's;\*\* ; \*\*;g' $file
  sed -i 's;\& ; \&;g' $file
  sed -i 's;\* ; \*;g' $file

  sed -i 's;concurrencpp::details::consts::k_inline_executor_max_concurrency_level;0;g' $file
  sed -i 's;concurrencpp::details::consts::k_worker_thread_executor_name;"worker thread";g' $file
  sed -i 's;concurrencpp::details::consts::k_inline_executor_name;"inline";g' $file
  sed -i 's;concurrencpp::details::consts::k_manual_executor_name;"manual";g' $file
  sed -i 's;concurrencpp::details::consts::k_thread_pool_executor_name;"thread pool";g' $file
  sed -i 's;concurrencpp::details::consts::k_worker_thread_max_concurrency_level;1;g' $file
  sed -i 's;concurrencpp::details::consts::k_manual_executor_max_concurrency_level;std::numeric_limits<int>::max();g' $file
  sed -i 's;concurrencpp::timer_queue;plain::TimerQueue;g' $file
  sed -i 's;concurrencpp::executor;plain::concurrency::executor::Basic;g' $file
  sed -i 's;concurrencpp::task;plain::concurrency::Task;g' $file
  sed -i 's;concurrencpp::details::make_executor_worker_name;concurrency::executor::detail::make_executor_worker_name;g' $file
  sed -i 's;executor->name;executor->name_;' $file

  sed -i 's;result_status::value;concurrency::ResultStatus::Value;g' $file
  sed -i 's;result_status::idle;concurrency::ResultStatus::Idle;g' $file
  sed -i 's;result_status::exception;concurrency::ResultStatus::Exception;g' $file

  sed -i 's;null_result;concurrency::result::null;g' $file

  sed -i 's;concurrencpp;plain;g' $file
  sed -i 's;    ;  ;g' $file
  sed -i 's;  private:;private:;g' $file
  sed -i 's;  public:;public:;g' $file
  sed -i 's;details::;detail::;g' $file
  sed -i 's;::details;::detail;g' $file

  sed -i 's;assert_equal(;ASSERT_EQ(;g' $file
  sed -i 's;assert_not_equal(;ASSERT_NE(;g' $file
  sed -i 's;assert_true(;ASSERT_TRUE(;g' $file
  sed -i 's;assert_false(;ASSERT_FALSE(;g' $file
  sed -i 's;assert_bigger(;ASSERT_GT(;g' $file
  sed -i 's;assert_smaller(;ASSERT_LT(;g' $file
  sed -i 's;assert_bigger_equal(;ASSERT_GE(;g' $file
  sed -i 's;assert_smaller_equal(;ASSERT_LE(;g' $file

  sed -i 's;plain/executors/executor.h;plain/concurrency/executor/basic.h;g' $file

  sed -i 's;plain::errors::runtime_shutdown;std::runtime_error;g' $file
  sed -i 's;errors::runtime_shutdown;std::runtime_error;g' $file
  sed -i 's;plain::errors::broken_task;std::runtime_error;g' $file
  sed -i 's;errors::broken_task;std::runtime_error;g' $file
  sed -i 's;plain::detail::consts::k_thread_executor_max_concurrency_level;std::numeric_limits<int>::max();g'

  replace_class $file
done
