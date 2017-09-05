/*
 * Copyright (c) 2017, Respective Authors.
 *
 * The MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include <cstdlib>
#include <iostream>
//#include <boost/test/included/unit_test.hpp>

#include <boost/test/impl/compiler_log_formatter.ipp>
#include <boost/test/impl/junit_log_formatter.ipp>
#include <boost/test/impl/debug.ipp>
#include <boost/test/impl/decorator.ipp>
#include <boost/test/impl/framework.ipp>
#include <boost/test/impl/execution_monitor.ipp>
#include <boost/test/impl/plain_report_formatter.ipp>
#include <boost/test/impl/progress_monitor.ipp>
#include <boost/test/impl/results_collector.ipp>
#include <boost/test/impl/results_reporter.ipp>
#include <boost/test/impl/test_framework_init_observer.ipp>
#include <boost/test/impl/test_tools.ipp>
#include <boost/test/impl/test_tree.ipp>
#include <boost/test/impl/unit_test_log.ipp>

#include <boost/test/impl/unit_test_monitor.ipp>
#include <boost/test/impl/unit_test_parameters.ipp>
#include <boost/test/impl/xml_log_formatter.ipp>
#include <boost/test/impl/xml_report_formatter.ipp>


//#include <boost/test/impl/unit_test_main.ipp>







extern uint32_t EOS_TESTING_GENESIS_TIMESTAMP;

boost::unit_test::test_suite* init_unit_test_suite(int argc, char* argv[]) {
   std::srand(time(NULL));
   std::cout << "Random number generator seeded to " << time(NULL) << std::endl;
   const char* genesis_timestamp_str = getenv("EOS_TESTING_GENESIS_TIMESTAMP");
   if( genesis_timestamp_str != nullptr )
   {
      EOS_TESTING_GENESIS_TIMESTAMP = std::stoul( genesis_timestamp_str );
   }
   std::cout << "EOS_TESTING_GENESIS_TIMESTAMP is " << EOS_TESTING_GENESIS_TIMESTAMP << std::endl;
   return nullptr;
}
