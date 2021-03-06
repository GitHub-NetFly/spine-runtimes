﻿/******************************************************************************
* Spine Runtimes Software License v2.5
*
* Copyright (c) 2013-2016, Esoteric Software
* All rights reserved.
*
* You are granted a perpetual, non-exclusive, non-sublicensable, and
* non-transferable license to use, install, execute, and perform the Spine
* Runtimes software and derivative works solely for personal or internal
* use. Without the written permission of Esoteric Software (see Section 2 of
* the Spine Software License Agreement), you may not (a) modify, translate,
* adapt, or develop new applications using the Spine Runtimes or otherwise
* create derivative works or improvements of the Spine Runtimes or (b) remove,
* delete, alter, or obscure any trademarks or any copyright, trademark, patent,
* or other intellectual property or proprietary rights notices on or in the
* Software, including any copy thereof. Redistributions in binary or source
* form must include this license and terms.
*
* THIS SOFTWARE IS PROVIDED BY ESOTERIC SOFTWARE "AS IS" AND ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
* EVENT SHALL ESOTERIC SOFTWARE BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES, BUSINESS INTERRUPTION, OR LOSS OF
* USE, DATA, OR PROFITS) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
* IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/



#include <spine/Event.h>


spine::Event::Event(float time, const spine::EventData &data) :
		_data(data),
		_time(time),
		_intValue(0),
		_floatValue(0),
		_stringValue(),
		_volume(1),
		_balance(0)
{
}

spine::Event::Event() :
	_time(0),
	_intValue(0),
	_floatValue(0),
	_stringValue(),
	_volume(1),
	_balance(0)
{

}

const spine::EventData &spine::Event::getData() {
	return _data;
}

float spine::Event::getTime() {
	return _time;
}

int spine::Event::getIntValue() {
	return _intValue;
}

void spine::Event::setIntValue(int inValue) {
	_intValue = inValue;
}

float spine::Event::getFloatValue() {
	return _floatValue;
}

void spine::Event::setFloatValue(float inValue) {
	_floatValue = inValue;
}

const spine::String &spine::Event::getStringValue() {
	return _stringValue;
}

void spine::Event::setStringValue(const spine::String &inValue) {
	_stringValue = inValue;
}


float spine::Event::getVolume() {
	return _volume;
}

void spine::Event::setVolume(float inValue) {
	_volume = inValue;
}

float spine::Event::getBalance() {
	return _balance;
}

void spine::Event::setBalance(float inValue) {
	_balance = inValue;
}
