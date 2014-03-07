/**
 * Licensed to Green Energy Corp (www.greenenergycorp.com) under one or
 * more contributor license agreements. See the NOTICE file distributed
 * with this work for additional information regarding copyright ownership.
 * Green Energy Corp licenses this file to you under the Apache License,
 * Version 2.0 (the "License"); you may not use this file except in
 * compliance with the License.  You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * This project was forked on 01/01/2013 by Automatak, LLC and modifications
 * may have been made to this file. Automatak, LLC licenses these modifications
 * to you under the terms of the License.
 */
#include <boost/test/unit_test.hpp>

#include "TestHelpers.h"
#include "AsyncPhysBaseTest.h"
#include "Exception.h"

using namespace opendnp3;
using namespace openpal;

BOOST_AUTO_TEST_SUITE(PhysicalLayerAsyncBaseSuite)

BOOST_AUTO_TEST_CASE(ClosedState)
{
	AsyncPhysBaseTest t;
	uint8_t buff;
	WriteBuffer wb(&buff, 1);

	t.phys.AsyncClose();
	BOOST_REQUIRE(t.log.PopOneEntry(LogLevel::Error));
	t.upper.SendDown("00");
	BOOST_REQUIRE(t.log.PopOneEntry(LogLevel::Error));
	t.phys.AsyncRead(wb);
	BOOST_REQUIRE(t.log.PopOneEntry(LogLevel::Error));
	t.phys.SignalOpenFailure();
	BOOST_REQUIRE(t.log.PopOneEntry(LogLevel::Error));
	t.phys.SignalOpenSuccess();
	BOOST_REQUIRE(t.log.PopOneEntry(LogLevel::Error));
	t.phys.SignalSendSuccess();
	BOOST_REQUIRE(t.log.PopOneEntry(LogLevel::Error));
	t.phys.SignalSendFailure();
	BOOST_REQUIRE(t.log.PopOneEntry(LogLevel::Error));

}

BOOST_AUTO_TEST_CASE(OpenCloseNotification)
{
	AsyncPhysBaseTest t;
	const size_t NUM = 3;

	for(size_t i = 1; i <= NUM; ++i) {
		t.phys.AsyncOpen();
		t.phys.SignalOpenSuccess();
		BOOST_REQUIRE_EQUAL(t.upper.GetState().mNumLayerUp, i);
		t.phys.AsyncClose();

		t.phys.SignalOpenFailure();
		BOOST_REQUIRE(t.log.PopOneEntry(LogLevel::Error));
		t.phys.SignalOpenSuccess();
		BOOST_REQUIRE(t.log.PopOneEntry(LogLevel::Error));
		t.phys.SignalSendSuccess();
		BOOST_REQUIRE(t.log.PopOneEntry(LogLevel::Error));
		t.phys.SignalSendFailure();
		BOOST_REQUIRE(t.log.PopOneEntry(LogLevel::Error));

		BOOST_REQUIRE_EQUAL(t.phys.NumClose(), i);
		BOOST_REQUIRE_EQUAL(t.upper.GetState().mNumLayerDown, i);
	}
}

BOOST_AUTO_TEST_CASE(ReadState)
{
	AsyncPhysBaseTest t;
	t.phys.AsyncOpen();
	t.phys.SignalOpenSuccess();
	t.adapter.StartRead(); //start a read

	t.phys.SignalOpenFailure();
	BOOST_REQUIRE(t.log.PopOneEntry(LogLevel::Error));
	t.phys.SignalOpenSuccess();
	BOOST_REQUIRE(t.log.PopOneEntry(LogLevel::Error));
	t.phys.SignalSendSuccess();
	BOOST_REQUIRE(t.log.PopOneEntry(LogLevel::Error));
	t.phys.SignalSendFailure();
	BOOST_REQUIRE(t.log.PopOneEntry(LogLevel::Error));

	t.phys.TriggerRead("00");
	t.upper.BufferEqualsHex("00");
}

BOOST_AUTO_TEST_CASE(CloseWhileReading)
{
	AsyncPhysBaseTest t;
	t.phys.AsyncOpen();
	t.phys.SignalOpenSuccess();
	t.adapter.StartRead();

	t.phys.AsyncClose();
	BOOST_REQUIRE_EQUAL(t.phys.NumClose(), 1);
	BOOST_REQUIRE_EQUAL(t.upper.GetState().mNumLayerDown, 0); //layer shouldn't go down until the outstanding read comes back
	t.phys.SignalReadFailure();
	BOOST_REQUIRE_EQUAL(t.upper.GetState().mNumLayerDown, 1);
}

BOOST_AUTO_TEST_CASE(WriteState)
{
	AsyncPhysBaseTest t;
	t.phys.AsyncOpen();
	t.phys.SignalOpenSuccess();

	t.upper.SendDown("00");
	BOOST_REQUIRE_EQUAL(t.phys.Size(), 1);

	t.phys.SignalOpenFailure();
	BOOST_REQUIRE(t.log.PopOneEntry(LogLevel::Error));
	t.phys.SignalOpenSuccess();
	BOOST_REQUIRE(t.log.PopOneEntry(LogLevel::Error));
	t.phys.TriggerRead("");
	BOOST_REQUIRE(t.log.PopOneEntry(LogLevel::Error));
	t.phys.SignalReadFailure();
	BOOST_REQUIRE(t.log.PopOneEntry(LogLevel::Error));
}

BOOST_AUTO_TEST_CASE(CloseWhileWriting)
{
	AsyncPhysBaseTest t;
	t.phys.AsyncOpen();
	t.phys.SignalOpenSuccess();

	t.upper.SendDown("00");
	t.phys.AsyncClose();
	BOOST_REQUIRE_EQUAL(t.phys.NumClose(), 1);
	BOOST_REQUIRE_EQUAL(t.upper.GetState().mNumLayerDown, 0); //layer shouldn't go down until the outstanding write comes back
	t.phys.SignalSendFailure();
	BOOST_REQUIRE_EQUAL(t.upper.GetState().mNumLayerDown, 1);
}

BOOST_AUTO_TEST_CASE(CloseWhileReadingWriting)
{
	AsyncPhysBaseTest t;
	t.phys.AsyncOpen();
	t.phys.SignalOpenSuccess();

	t.upper.SendDown("00");
	t.adapter.StartRead();
	t.phys.AsyncClose();
	BOOST_REQUIRE_EQUAL(t.phys.NumClose(), 1);
	BOOST_REQUIRE_EQUAL(t.upper.GetState().mNumLayerDown, 0);
	t.phys.SignalSendFailure();
	BOOST_REQUIRE_EQUAL(t.upper.GetState().mNumLayerDown, 0);
	t.phys.SignalReadFailure();
	BOOST_REQUIRE_EQUAL(t.upper.GetState().mNumLayerDown, 1);
}

BOOST_AUTO_TEST_CASE(CloseWhileWritingReading)
{
	AsyncPhysBaseTest t;
	t.phys.AsyncOpen();
	t.phys.SignalOpenSuccess();

	t.upper.SendDown("00");
	t.adapter.StartRead();
	t.phys.AsyncClose();
	BOOST_REQUIRE_EQUAL(t.phys.NumClose(), 1);
	BOOST_REQUIRE_EQUAL(t.upper.GetState().mNumLayerDown, 0);
	t.phys.SignalReadFailure();
	BOOST_REQUIRE_EQUAL(t.upper.GetState().mNumLayerDown, 0);
	t.phys.SignalSendFailure();
	BOOST_REQUIRE_EQUAL(t.upper.GetState().mNumLayerDown, 1);
}

BOOST_AUTO_TEST_CASE(CloseWhileOpening)
{
	AsyncPhysBaseTest t;

	t.phys.AsyncOpen();
	t.phys.AsyncClose();
	BOOST_REQUIRE(t.phys.IsOpening());
	BOOST_REQUIRE(t.phys.IsClosing());

	/* this could happen for some layers, but we
	   still need to return an open failure to the handler */
	t.phys.SignalOpenSuccess();

	BOOST_REQUIRE_EQUAL(0, t.upper.GetState().mNumLayerUp);
	BOOST_REQUIRE_EQUAL(1, t.adapter.GetNumOpenFailure());
}

BOOST_AUTO_TEST_SUITE_END()

