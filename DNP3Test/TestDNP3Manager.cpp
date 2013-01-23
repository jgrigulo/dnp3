
//
// Licensed to Green Energy Corp (www.greenenergycorp.com) under one or
// more contributor license agreements. See the NOTICE file distributed
// with this work for additional information regarding copyright ownership.
// Green Energy Corp licenses this file to you under the Apache License,
// Version 2.0 (the "License"); you may not use this file except in
// compliance with the License.  You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// This file was forked on 01/01/2013 by Automatak, LLC and modifications
// have been made to this file. Automatak, LLC licenses these modifications to
// you under the GNU Affero General Public License Version 3.0
// (the "Additional License"). You may not use these modifications except in
// compliance with the additional license. You may obtain a copy of the Additional
// License at
//
// http://www.gnu.org/licenses/agpl.html
//
// Contact Automatak, LLC for a commercial license to these modifications
//
#include <boost/test/unit_test.hpp>
#include <APLTestTools/TestHelpers.h>

#include <DNP3/DNP3Manager.h>
#include <DNP3/IChannel.h>

#include <APL/LogToStdio.h>
#include <APL/SimpleDataObserver.h>

#include <thread>

using namespace apl;
using namespace apl::dnp;

BOOST_AUTO_TEST_SUITE(DNP3ManagerTestSuite)

BOOST_AUTO_TEST_CASE(ConstructionDestruction)
{		
	//DNP3Manager mgr(std::thread::hardware_concurrency());
	DNP3Manager mgr(1);
	mgr.AddLogSubscriber(LogToStdio::Inst());

	auto pChannel = mgr.AddTCPClient("client", LEV_INFO, 5000, "127.0.0.1", 20000);	
	auto pMaster = pChannel->AddMaster("master", LEV_INFO, PrintingDataObserver::Inst(), MasterStackConfig());	
}

BOOST_AUTO_TEST_CASE(ManualChannelShutdown)
{
	DNP3Manager mgr(std::thread::hardware_concurrency());
	mgr.AddTCPClient("client", LEV_INFO, 5000, "127.0.0.1", 20000)->Shutdown();
}



BOOST_AUTO_TEST_SUITE_END()

