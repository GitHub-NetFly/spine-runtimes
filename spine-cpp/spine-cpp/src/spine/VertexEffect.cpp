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



#include <spine/VertexEffect.h>
#include <spine/MathUtil.h>
#include <spine/Skeleton.h>

using namespace spine;

JitterVertexEffect::JitterVertexEffect(float jitterX, float jitterY): _jitterX(jitterX), _jitterY(jitterY) {
}

void JitterVertexEffect::begin(Skeleton &skeleton) {
	SP_UNUSED(skeleton);
}

void JitterVertexEffect::transform(float &x, float &y, float &u, float &v, Color &light, Color &dark) {
	SP_UNUSED(u);
	SP_UNUSED(v);
	SP_UNUSED(light);
	SP_UNUSED(dark);
	float jitterX = _jitterX;
	float jitterY = _jitterY;
	x += MathUtil::randomTriangular(-jitterX, jitterX);
	y += MathUtil::randomTriangular(-jitterX, jitterY);
}

void JitterVertexEffect::end() {
}

void JitterVertexEffect::setJitterX(float jitterX) {
	_jitterX = jitterX;
}

float JitterVertexEffect::getJitterX() {
	return _jitterX;
}

void JitterVertexEffect::setJitterY(float jitterY) {
	_jitterY = jitterY;
}

float JitterVertexEffect::getJitterY() {
	return _jitterY;
}

SwirlVertexEffect::SwirlVertexEffect(float radius, Interpolation &interpolation):
	_centerX(0),
	_centerY(0),
	_radius(radius),
	_angle(0),
	_worldX(0),
	_worldY(0),
	_interpolation(interpolation) {
}

void SwirlVertexEffect::begin(Skeleton &skeleton) {
	_worldX = skeleton.getX() + _centerX;
	_worldY = skeleton.getY() + _centerY;
}

void SwirlVertexEffect::transform(float &positionX, float &positionY, float &u, float &v, Color &light, Color &dark) {
	SP_UNUSED(u);
	SP_UNUSED(v);
	SP_UNUSED(light);
	SP_UNUSED(dark);

	float x = positionX - _worldX;
	float y = positionY - _worldY;
	float dist = (float)MathUtil::sqrt(x * x + y * y);
	if (dist < _radius) {
		float theta = _interpolation.interpolate(0, _angle, (_radius - dist) / _radius);
		float cos = MathUtil::cos(theta), sin = MathUtil::sin(theta);
		positionX = cos * x - sin * y + _worldX;
		positionY = sin * x + cos * y + _worldY;
	}
}

void SwirlVertexEffect::end() {

}

void SwirlVertexEffect::setCenterX(float centerX) {
	_centerX = centerX;
}

float SwirlVertexEffect::getCenterX() {
	return _centerX;
}

void SwirlVertexEffect::setCenterY(float centerY) {
	_centerY = centerY;
}

float SwirlVertexEffect::getCenterY() {
	return _centerY;
}

void SwirlVertexEffect::setRadius(float radius) {
	_radius = radius;
}

float SwirlVertexEffect::getRadius() {
	return _radius;
}

void SwirlVertexEffect::setAngle(float angle) {
	_angle = angle * MathUtil::Deg_Rad;
}

float SwirlVertexEffect::getAngle() {
	return _angle;
}

void SwirlVertexEffect::setWorldX(float worldX) {
	_worldX = worldX;
}

float SwirlVertexEffect::getWorldX() {
	return _worldX;
}

void SwirlVertexEffect::setWorldY(float worldY) {
	_worldY = worldY;
}

float SwirlVertexEffect::getWorldY() {
	return _worldY;
}
