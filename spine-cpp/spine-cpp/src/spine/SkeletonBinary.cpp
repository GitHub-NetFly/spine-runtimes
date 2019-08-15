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


#include <spine/SkeletonBinary.h>

#include <spine/SkeletonData.h>
#include <spine/Atlas.h>
#include <spine/AtlasAttachmentLoader.h>
#include <spine/LinkedMesh.h>
#include <spine/Skin.h>
#include <spine/Attachment.h>
#include <spine/VertexAttachment.h>
#include <spine/Animation.h>
#include <spine/CurveTimeline.h>

#include <spine/ContainerUtil.h>
#include <spine/BoneData.h>
#include <spine/SlotData.h>
#include <spine/IkConstraintData.h>
#include <spine/TransformConstraintData.h>
#include <spine/PathConstraintData.h>
#include <spine/AttachmentType.h>
#include <spine/RegionAttachment.h>
#include <spine/BoundingBoxAttachment.h>
#include <spine/MeshAttachment.h>
#include <spine/PathAttachment.h>
#include <spine/PointAttachment.h>
#include <spine/ClippingAttachment.h>
#include <spine/EventData.h>
#include <spine/AttachmentTimeline.h>
#include <spine/ColorTimeline.h>
#include <spine/TwoColorTimeline.h>
#include <spine/RotateTimeline.h>
#include <spine/TranslateTimeline.h>
#include <spine/ScaleTimeline.h>
#include <spine/ShearTimeline.h>
#include <spine/IkConstraintTimeline.h>
#include <spine/TransformConstraintTimeline.h>
#include <spine/PathConstraintPositionTimeline.h>
#include <spine/PathConstraintSpacingTimeline.h>
#include <spine/PathConstraintMixTimeline.h>
#include <spine/DeformTimeline.h>
#include <spine/DrawOrderTimeline.h>
#include <spine/EventTimeline.h>
#include <spine/Event.h>

using namespace spine;

const int SkeletonBinary::BONE_ROTATE = 0;
const int SkeletonBinary::BONE_TRANSLATE = 1;
const int SkeletonBinary::BONE_SCALE = 2;
const int SkeletonBinary::BONE_SHEAR = 3;

const int SkeletonBinary::SLOT_ATTACHMENT = 0;
const int SkeletonBinary::SLOT_COLOR = 1;
const int SkeletonBinary::SLOT_TWO_COLOR = 2;

const int SkeletonBinary::PATH_POSITION = 0;
const int SkeletonBinary::PATH_SPACING = 1;
const int SkeletonBinary::PATH_MIX = 2;

const int SkeletonBinary::CURVE_LINEAR = 0;
const int SkeletonBinary::CURVE_STEPPED = 1;
const int SkeletonBinary::CURVE_BEZIER = 2;

SkeletonBinary::SkeletonBinary(Atlas *atlasArray) : _attachmentLoader(
		new AtlasAttachmentLoader(atlasArray)), _error(), _scale(1), _ownsLoader(true) {

}

SkeletonBinary::SkeletonBinary(AttachmentLoader *attachmentLoader) : _attachmentLoader(attachmentLoader), _error(),
																	 _scale(1), _ownsLoader(false) {
	assert(_attachmentLoader != NULL);
}

SkeletonBinary::~SkeletonBinary() {
	ContainerUtil::cleanUpVectorOfPointers(_linkedMeshes);
	_linkedMeshes.clear();

	if (_ownsLoader) {
		delete _attachmentLoader;
	}
}

SkeletonData *SkeletonBinary::readSkeletonData(const unsigned char *binary, const int length) {
	bool nonessential;
	SkeletonData *skeletonData;

	DataInput *input = new DataInput();
	input->cursor = binary;
	input->end = binary + length;

	_linkedMeshes.clear();

	skeletonData = new SkeletonData();

	char *skeletonData_hash = readString(input);
	skeletonData->_hash.own(skeletonData_hash);

	char *skeletonData_version = readString(input);
	skeletonData->_version.own(skeletonData_version);

	skeletonData->_x = readFloat(input);
	skeletonData->_y = readFloat(input);
	skeletonData->_width = readFloat(input);
	skeletonData->_height = readFloat(input);

	nonessential = readBoolean(input);

	if (nonessential) {
		/* Skip images path, audio path & fps */
		skeletonData->_fps = readFloat(input);
		skeletonData->_imagesPath.own(readString(input));
		skeletonData->_audioPath.own(readString(input));
	}

	int numStrings = readVarint(input, true);
	for (int i = 0; i < numStrings; i++)
		skeletonData->_strings.add(readString(input));

	/* Bones. */
	int numBones = readVarint(input, true);
	skeletonData->_bones.setSize(numBones, 0);
	for (int i = 0; i < numBones; ++i) {
		const char *name = readString(input);
		BoneData *parent = i == 0 ? 0 : skeletonData->_bones[readVarint(input, true)];
		BoneData *data = new BoneData(i, String(name, true), parent);
		data->_rotation = readFloat(input);
		data->_x = readFloat(input) * _scale;
		data->_y = readFloat(input) * _scale;
		data->_scaleX = readFloat(input);
		data->_scaleY = readFloat(input);
		data->_shearX = readFloat(input);
		data->_shearY = readFloat(input);
		data->_length = readFloat(input) * _scale;
		data->_transformMode = static_cast<TransformMode>(readVarint(input, true));
		data->_skinRequired = readBoolean(input);
		if (nonessential) {
			/* Skip bone color. */
			readInt(input);
		}
		skeletonData->_bones[i] = data;
	}

	/* Slots. */
	int slotsCount = readVarint(input, true);
	skeletonData->_slots.setSize(slotsCount, 0);
	for (int i = 0; i < slotsCount; ++i)
	{
		String slotName(readString(input), true);
		BoneData *boneData = skeletonData->_bones[readVarint(input, true)];
		SlotData *slotData = new SlotData(i, slotName.buffer(), *boneData);

		readColor(input, slotData->getColor());
		unsigned char r = readByte(input);
		unsigned char g = readByte(input);
		unsigned char b = readByte(input);
		unsigned char a = readByte(input);
		if (!(r == 0xff && g == 0xff && b == 0xff && a == 0xff)) {
			slotData->getDarkColor().set(r / 255.0f, g / 255.0f, b / 255.0f, 1);
			slotData->setHasDarkColor(true);
		}
		slotData->_attachmentName = readStringRef(input, skeletonData);
		slotData->_blendMode = static_cast<BlendMode>(readVarint(input, true));
		skeletonData->_slots[i] = slotData;
	}

	/* IK constraints. */
	int ikConstraintsCount = readVarint(input, true);
	skeletonData->_ikConstraints.setSize(ikConstraintsCount, 0);
	for (int i = 0; i < ikConstraintsCount; ++i) {
		const char *name = readString(input);
		IkConstraintData *data = new  IkConstraintData(String(name, true));
		data->setOrder(readVarint(input, true));
		data->setSkinRequired(readBoolean(input));
		int bonesCount = readVarint(input, true);
		data->_bones.setSize(bonesCount, 0);
		for (int ii = 0; ii < bonesCount; ++ii) {
			data->_bones[ii] = skeletonData->_bones[readVarint(input, true)];
		}
		data->_target = skeletonData->_bones[readVarint(input, true)];
		data->_mix = readFloat(input);
		data->_softness = readFloat(input) * _scale;
		data->_bendDirection = readSByte(input);
		data->_compress = readBoolean(input);
		data->_stretch = readBoolean(input);
		data->_uniform = readBoolean(input);
		skeletonData->_ikConstraints[i] = data;
	}

	/* Transform constraints. */
	int transformConstraintsCount = readVarint(input, true);
	skeletonData->_transformConstraints.setSize(transformConstraintsCount, 0);
	for (int i = 0; i < transformConstraintsCount; ++i) {
		const char *name = readString(input);
		TransformConstraintData *data = new  TransformConstraintData(String(name, true));
		data->setOrder(readVarint(input, true));
		data->setSkinRequired(readBoolean(input));
		int bonesCount = readVarint(input, true);
		data->_bones.setSize(bonesCount, 0);
		for (int ii = 0; ii < bonesCount; ++ii) {
			data->_bones[ii] = skeletonData->_bones[readVarint(input, true)];
		}
		data->_target = skeletonData->_bones[readVarint(input, true)];
		data->_local = readBoolean(input);
		data->_relative = readBoolean(input);
		data->_offsetRotation = readFloat(input);
		data->_offsetX = readFloat(input) * _scale;
		data->_offsetY = readFloat(input) * _scale;
		data->_offsetScaleX = readFloat(input);
		data->_offsetScaleY = readFloat(input);
		data->_offsetShearY = readFloat(input);
		data->_rotateMix = readFloat(input);
		data->_translateMix = readFloat(input);
		data->_scaleMix = readFloat(input);
		data->_shearMix = readFloat(input);
		skeletonData->_transformConstraints[i] = data;
	}

	/* Path constraints */
	int pathConstraintsCount = readVarint(input, true);
	skeletonData->_pathConstraints.setSize(pathConstraintsCount, 0);
	for (int i = 0; i < pathConstraintsCount; ++i) {
		const char *name = readString(input);
		PathConstraintData *data = new  PathConstraintData(String(name, true));
		data->setOrder(readVarint(input, true));
		data->setSkinRequired(readBoolean(input));
		int bonesCount = readVarint(input, true);
		data->_bones.setSize(bonesCount, 0);
		for (int ii = 0; ii < bonesCount; ++ii) {
			data->_bones[ii] = skeletonData->_bones[readVarint(input, true)];
		}
		data->_target = skeletonData->_slots[readVarint(input, true)];
		data->_positionMode = static_cast<PositionMode>(readVarint(input, true));
		data->_spacingMode = static_cast<SpacingMode>(readVarint(input, true));
		data->_rotateMode = static_cast<RotateMode>(readVarint(input, true));
		data->_offsetRotation = readFloat(input);
		data->_position = readFloat(input);
		if (data->_positionMode == PositionMode_Fixed) data->_position *= _scale;
		data->_spacing = readFloat(input);
		if (data->_spacingMode == SpacingMode_Length || data->_spacingMode == SpacingMode_Fixed)
			data->_spacing *= _scale;
		data->_rotateMix = readFloat(input);
		data->_translateMix = readFloat(input);
		skeletonData->_pathConstraints[i] = data;
	}

	/* Default skin. */
	TSharedPtr<Skin> defaultSkin = readSkin(input, true, skeletonData, nonessential);
	if (defaultSkin) {
		skeletonData->_defaultSkin = defaultSkin;
		skeletonData->_skins.Add(defaultSkin);
	}

	/* Skins. */
	for (size_t i = 0, n = (size_t)readVarint(input, true); i < n; ++i) {
		skeletonData->_skins.Add(readSkin(input, false, skeletonData, nonessential));
	}

	/* Linked meshes. */
	for (int i = 0, n = _linkedMeshes.size(); i < n; ++i) {
		LinkedMesh *linkedMesh = _linkedMeshes[i];
		TSharedPtr<Skin> skin = linkedMesh->_skin.length() == 0 ? skeletonData->getDefaultSkin() : skeletonData->findSkin(
				linkedMesh->_skin.buffer());
		if (!skin )
		{
			delete input;
			delete skeletonData;
			setError("Skin not found: ", linkedMesh->_skin.buffer());
			return NULL;
		}
		TSharedPtr<Attachment> parent = skin->getAttachment(linkedMesh->_slotIndex, linkedMesh->_parent.buffer());
		if (!parent ) {
			delete input;
			delete skeletonData;
			setError("Parent mesh not found: ", linkedMesh->_parent.buffer());
			return NULL;
		}
		linkedMesh->_mesh->_deformAttachment = linkedMesh->_inheritDeform ? static_cast<VertexAttachment*>(parent.Get()) : linkedMesh->_mesh;
		linkedMesh->_mesh->setParentMesh(static_cast<MeshAttachment *>(parent.Get()));
		linkedMesh->_mesh->updateUVs();
		_attachmentLoader->configureAttachment(linkedMesh->_mesh->AsShared());
	}
	ContainerUtil::cleanUpVectorOfPointers(_linkedMeshes);
	_linkedMeshes.clear();

	/* Events. */
	int eventsCount = readVarint(input, true);
	skeletonData->_events.setSize(eventsCount, 0);
	for (int i = 0; i < eventsCount; ++i) {
		const char *name = readStringRef(input, skeletonData);
		EventData *eventData = new  EventData(String(name));
		eventData->_intValue = readVarint(input, false);
		eventData->_floatValue = readFloat(input);
		eventData->_stringValue.own(readString(input));
		eventData->_audioPath.own(readString(input)); // skip audio path
		if (!eventData->_audioPath.isEmpty()) {
			eventData->_volume = readFloat(input);
			eventData->_balance = readFloat(input);
		}
		skeletonData->_events[i] = eventData;
	}

	/* Animations. */
	int animationsCount = readVarint(input, true);
	
	skeletonData->_animations.SetNum(animationsCount);
	for (int i = 0; i < animationsCount; ++i) {
		String name(readString(input), true);
		Animation *animation = readAnimation(name, input, skeletonData);
		if (!animation) {
			delete input;
			delete skeletonData;
			return NULL;
		}
		skeletonData->_animations[i] = MakeShareable(animation);
	}

	delete input;
	return skeletonData;
}

SkeletonData *SkeletonBinary::readSkeletonDataFile(const String &path) {
	int length;
	SkeletonData *skeletonData;
	const char *binary = SpineExtension::readFile(path.buffer(), &length);
	if (length == 0 || !binary) {
		setError("Unable to read skeleton file: ", path.buffer());
		return NULL;
	}
	skeletonData = readSkeletonData((unsigned char *) binary, length);
	SpineExtension::free(binary, __FILE__, __LINE__);
	return skeletonData;
}

void SkeletonBinary::setError(const char *value1, const char *value2) {
	char message[256];
	int length;
	strcpy(message, value1);
	length = (int) strlen(value1);
	if (value2) {
		strncat(message + length, value2, 255 - length);
	}

	_error = String(message);
}

char *SkeletonBinary::readString(DataInput *input) {
	int length = readVarint(input, true);
	char *string;
	if (length == 0) {
		return NULL;
	}
	string = SpineExtension::alloc<char>(length, __FILE__, __LINE__);
	memcpy(string, input->cursor, length - 1);
	input->cursor += length - 1;
	string[length - 1] = '\0';
	return string;
}

char* SkeletonBinary::readStringRef(DataInput* input, SkeletonData* skeletonData) {
	int index = readVarint(input, true);
	return index == 0 ? nullptr : skeletonData->_strings[index - 1];
}

float SkeletonBinary::readFloat(DataInput *input) {
	union {
		int intValue;
		float floatValue;
	} intToFloat;

	intToFloat.intValue = readInt(input);

	return intToFloat.floatValue;
}

unsigned char SkeletonBinary::readByte(DataInput *input) {
	return *input->cursor++;
}

signed char SkeletonBinary::readSByte(DataInput *input) {
	return (signed char) readByte(input);
}

bool SkeletonBinary::readBoolean(DataInput *input) {
	return readByte(input) != 0;
}

int SkeletonBinary::readInt(DataInput *input) {
	int result = readByte(input);
	result <<= 8;
	result |= readByte(input);
	result <<= 8;
	result |= readByte(input);
	result <<= 8;
	result |= readByte(input);
	return result;
}

void SkeletonBinary::readColor(DataInput *input, Color &color) {
	color.r = readByte(input) / 255.0f;
	color.g = readByte(input) / 255.0f;
	color.b = readByte(input) / 255.0f;
	color.a = readByte(input) / 255.0f;
}

int SkeletonBinary::readVarint(DataInput *input, bool optimizePositive) {
	unsigned char b = readByte(input);
	int value = b & 0x7F;
	if (b & 0x80) {
		b = readByte(input);
		value |= (b & 0x7F) << 7;
		if (b & 0x80) {
			b = readByte(input);
			value |= (b & 0x7F) << 14;
			if (b & 0x80) {
				b = readByte(input);
				value |= (b & 0x7F) << 21;
				if (b & 0x80) value |= (readByte(input) & 0x7F) << 28;
			}
		}
	}

	if (!optimizePositive) {
		value = (((unsigned int) value >> 1) ^ -(value & 1));
	}

	return value;
}

TSharedPtr<Skin> SkeletonBinary::readSkin(DataInput *input, bool defaultSkin, SkeletonData *skeletonData, bool nonessential)
{
	TSharedPtr<Skin> SkinPtr;
	int slotCount = 0;
	if (defaultSkin) 
	{
		slotCount = readVarint(input, true);
		if (slotCount == 0)
		{
			return nullptr;
		}

		SkinPtr = MakeShared<Skin>("default");
	}
	else 
	{
		SkinPtr = MakeShared<Skin>(readStringRef(input, skeletonData));

		for (int i = 0, n = readVarint(input, true); i < n; i++)
			SkinPtr->getBones().add(skeletonData->_bones[readVarint(input, true)]);

		for (int i = 0, n = readVarint(input, true); i < n; i++)
			SkinPtr->getConstraints().add(skeletonData->_ikConstraints[readVarint(input, true)]);

		for (int i = 0, n = readVarint(input, true); i < n; i++)
			SkinPtr->getConstraints().add(skeletonData->_transformConstraints[readVarint(input, true)]);

		for (int i = 0, n = readVarint(input, true); i < n; i++)
			SkinPtr->getConstraints().add(skeletonData->_pathConstraints[readVarint(input, true)]);
		slotCount = readVarint(input, true);
	}

	for (int i = 0; i < slotCount; ++i) 
	{
		int slotIndex = readVarint(input, true);
		for (int ii = 0, nn = readVarint(input, true); ii < nn; ++ii) {
			String name(readStringRef(input, skeletonData));
			TSharedPtr<Attachment> attachment = readAttachment(input, SkinPtr.Get(), slotIndex, name, skeletonData, nonessential);
			if (attachment) SkinPtr->setAttachment(slotIndex, name.buffer(), attachment);
		}
	}
	return SkinPtr;
}

TSharedPtr<Attachment> SkeletonBinary::readAttachment(DataInput *input, Skin *skin, int slotIndex, const String &attachmentName,
										   SkeletonData *skeletonData, bool nonessential) {
	String name(readStringRef(input, skeletonData));
	if (name.isEmpty()) name = attachmentName;

	AttachmentType type = static_cast<AttachmentType>(readByte(input));
	switch (type) {
		case AttachmentType_Region: {
			String path(readStringRef(input, skeletonData));
			if (path.isEmpty()) path = name;
			TSharedPtr<RegionAttachment> region = _attachmentLoader->newRegionAttachment(*skin, name.buffer(), path.buffer());
			region->_path = path.buffer();
			region->_rotation = readFloat(input);
			region->_x = readFloat(input) * _scale;
			region->_y = readFloat(input) * _scale;
			region->_scaleX = readFloat(input);
			region->_scaleY = readFloat(input);
			region->_width = readFloat(input) * _scale;
			region->_height = readFloat(input) * _scale;
			readColor(input, region->getColor());
			region->updateOffset();
			_attachmentLoader->configureAttachment(region);
			return region;
		}
		case AttachmentType_Boundingbox: {
			int vertexCount = readVarint(input, true);
			TSharedPtr<BoundingBoxAttachment> box = _attachmentLoader->newBoundingBoxAttachment(*skin, name.buffer());
			readVertices(input, static_cast<VertexAttachment *>(box.Get()), vertexCount);
			if (nonessential) {
				/* Skip color. */
				readInt(input);
			}
			_attachmentLoader->configureAttachment(box);
			return box;
		}
		case AttachmentType_Mesh: {
			int vertexCount;
			TSharedPtr<MeshAttachment> mesh;
			String path(readStringRef(input, skeletonData));
			if (path.isEmpty()) path = name;

		   mesh = _attachmentLoader->newMeshAttachment(*skin, name.buffer(), path.buffer());
			mesh->_path = path;
			readColor(input, mesh->getColor());
			vertexCount = readVarint(input, true);
			readFloatArray(input, vertexCount << 1, 1, mesh->getRegionUVs());
			readShortArray(input, mesh->getTriangles());
			readVertices(input, mesh.Get(), vertexCount);
			mesh->updateUVs();
			mesh->_hullLength = readVarint(input, true) << 1;
			if (nonessential) {
				readShortArray(input, mesh->getEdges());
				mesh->_width = readFloat(input) * _scale;
				mesh->_height = readFloat(input) * _scale;
			} else {
				mesh->_width = 0;
				mesh->_height = 0;
			}
			_attachmentLoader->configureAttachment(mesh);
			return mesh;
		}
		case AttachmentType_Linkedmesh: {
			String path(readStringRef(input, skeletonData));
			if (path.isEmpty()) path = name;

			TSharedPtr<MeshAttachment> mesh = _attachmentLoader->newMeshAttachment(*skin, name.buffer(), path.buffer());
			mesh->_path = path;
			readColor(input, mesh->getColor());
			String skinName(readStringRef(input, skeletonData));
			String parent(readStringRef(input, skeletonData));
			bool inheritDeform = readBoolean(input);
			if (nonessential) {
				mesh->_width = readFloat(input) * _scale;
				mesh->_height = readFloat(input) * _scale;
			}

			LinkedMesh *linkedMesh = new  LinkedMesh(mesh.Get(), String(skinName), slotIndex,
																		String(parent), inheritDeform);
			_linkedMeshes.add(linkedMesh);
			return mesh;
		}
		case AttachmentType_Path: {
			TSharedPtr<PathAttachment> path = _attachmentLoader->newPathAttachment(*skin, name.buffer());
			path->_closed = readBoolean(input);
			path->_constantSpeed = readBoolean(input);
			int vertexCount = readVarint(input, true);
			readVertices(input, static_cast<VertexAttachment *>(path.Get()), vertexCount);
			int lengthsLength = vertexCount / 3;
			path->_lengths.setSize(lengthsLength, 0);
			for (int i = 0; i < lengthsLength; ++i) {
				path->_lengths[i] = readFloat(input) * _scale;
			}
			if (nonessential) {
				/* Skip color. */
				readInt(input);
			}
			_attachmentLoader->configureAttachment(path);
			return path;
		}
		case AttachmentType_Point: {
			TSharedPtr<PointAttachment> point = _attachmentLoader->newPointAttachment(*skin, name.buffer());
			point->_rotation = readFloat(input);
			point->_x = readFloat(input) * _scale;
			point->_y = readFloat(input) * _scale;

			if (nonessential) {
				/* Skip color. */
				readInt(input);
			}
			_attachmentLoader->configureAttachment(point);
			return point;
		}
		case AttachmentType_Clipping: {
			int endSlotIndex = readVarint(input, true);
			int vertexCount = readVarint(input, true);
			TSharedPtr<ClippingAttachment> clip = _attachmentLoader->newClippingAttachment(*skin, name.buffer());
			readVertices(input, static_cast<VertexAttachment *>(clip.Get()), vertexCount);
			clip->_endSlot = skeletonData->_slots[endSlotIndex];
			if (nonessential) {
				/* Skip color. */
				readInt(input);
			}
			_attachmentLoader->configureAttachment(clip);
			return clip;
		}
	}
	return NULL;
}

void SkeletonBinary::readVertices(DataInput *input, VertexAttachment *attachment, int vertexCount) {
	float scale = _scale;
	int verticesLength = vertexCount << 1;
	attachment->setWorldVerticesLength(vertexCount << 1);

	if (!readBoolean(input)) {
		readFloatArray(input, verticesLength, scale, attachment->getVertices());
		return;
	}

	Vector<float> &vertices = attachment->getVertices();
	Vector<int32> &bones = attachment->getBones();
	vertices.ensureCapacity(verticesLength * 3 * 3);
	bones.ensureCapacity(verticesLength * 3);

	for (int i = 0; i < vertexCount; ++i) {
		int boneCount = readVarint(input, true);
		bones.add(boneCount);
		for (int ii = 0; ii < boneCount; ++ii) {
			bones.add(readVarint(input, true));
			vertices.add(readFloat(input) * scale);
			vertices.add(readFloat(input) * scale);
			vertices.add(readFloat(input));
		}
	}
}

void SkeletonBinary::readFloatArray(DataInput *input, int n, float scale, Vector<float> &array) {
	array.setSize(n, 0);

	int i;
	if (scale == 1) {
		for (i = 0; i < n; ++i) {
			array[i] = readFloat(input);
		}
	} else {
		for (i = 0; i < n; ++i) {
			array[i] = readFloat(input) * scale;
		}
	}
}

void SkeletonBinary::readShortArray(DataInput *input, Vector<unsigned short> &array) {
	int n = readVarint(input, true);
	array.setSize(n, 0);

	int i;
	for (i = 0; i < n; ++i) {
		array[i] = readByte(input) << 8;
		array[i] |= readByte(input);
	}
}

Animation *SkeletonBinary::readAnimation(const String &name, DataInput *input, SkeletonData *skeletonData) {
	Vector<Timeline *> timelines;
	float scale = _scale;
	float duration = 0;

	// Slot timelines.
	for (int i = 0, n = readVarint(input, true); i < n; ++i) {
		int slotIndex = readVarint(input, true);
		for (int ii = 0, nn = readVarint(input, true); ii < nn; ++ii) {
			unsigned char timelineType = readByte(input);
			int frameCount = readVarint(input, true);
			switch (timelineType) {
				case SLOT_ATTACHMENT: {
					AttachmentTimeline *timeline = new AttachmentTimeline(frameCount);
					timeline->_slotIndex = slotIndex;
					for (int frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
						float time = readFloat(input);
						String attachmentName(readStringRef(input, skeletonData));
						timeline->setFrame(frameIndex, time, attachmentName.buffer());
					}
					timelines.add(timeline);
					duration = MathUtil::max(duration, timeline->_frames[frameCount - 1]);
					break;
				}
				case SLOT_COLOR: {
					ColorTimeline *timeline = new ColorTimeline(frameCount);
					timeline->_slotIndex = slotIndex;
					for (int frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
						float time = readFloat(input);
						int color = readInt(input);
						float r = ((color & 0xff000000) >> 24) / 255.0f;
						float g = ((color & 0x00ff0000) >> 16) / 255.0f;
						float b = ((color & 0x0000ff00) >> 8) / 255.0f;
						float a = ((color & 0x000000ff)) / 255.0f;
						timeline->setFrame(frameIndex, time, r, g, b, a);
						if (frameIndex < frameCount - 1) {
							readCurve(input, frameIndex, timeline);
						}
					}
					timelines.add(timeline);
					duration = MathUtil::max(duration, timeline->_frames[(frameCount - 1) * ColorTimeline::ENTRIES]);
					break;
				}
				case SLOT_TWO_COLOR: {
					TwoColorTimeline *timeline = new TwoColorTimeline(frameCount);
					timeline->_slotIndex = slotIndex;
					for (int frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
						float time = readFloat(input);
						int color = readInt(input);
						float r = ((color & 0xff000000) >> 24) / 255.0f;
						float g = ((color & 0x00ff0000) >> 16) / 255.0f;
						float b = ((color & 0x0000ff00) >> 8) / 255.0f;
						float a = ((color & 0x000000ff)) / 255.0f;
						int color2 = readInt(input); // 0x00rrggbb
						float r2 = ((color2 & 0x00ff0000) >> 16) / 255.0f;
						float g2 = ((color2 & 0x0000ff00) >> 8) / 255.0f;
						float b2 = ((color2 & 0x000000ff)) / 255.0f;

						timeline->setFrame(frameIndex, time, r, g, b, a, r2, g2, b2);
						if (frameIndex < frameCount - 1) {
							readCurve(input, frameIndex, timeline);
						}
					}
					timelines.add(timeline);
					duration = MathUtil::max(duration, timeline->_frames[(frameCount - 1) * TwoColorTimeline::ENTRIES]);
					break;
				}
				default: {
					ContainerUtil::cleanUpVectorOfPointers(timelines);
					setError("Invalid timeline type for a slot: ", TCHAR_TO_UTF8(*skeletonData->_slots[slotIndex]->_name));
					return NULL;
				}
			}
		}
	}

	// Bone timelines.
	for (int i = 0, n = readVarint(input, true); i < n; ++i) {
		int boneIndex = readVarint(input, true);
		for (int ii = 0, nn = readVarint(input, true); ii < nn; ++ii) {
			unsigned char timelineType = readByte(input);
			int frameCount = readVarint(input, true);
			switch (timelineType) {
				case BONE_ROTATE: {
					RotateTimeline *timeline = new RotateTimeline(frameCount);
					timeline->_boneIndex = boneIndex;
					for (int frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
						float time = readFloat(input);
						float degrees = readFloat(input);
						timeline->setFrame(frameIndex, time, degrees);
						if (frameIndex < frameCount - 1) {
							readCurve(input, frameIndex, timeline);
						}
					}
					timelines.add(timeline);
					duration = MathUtil::max(duration, timeline->_frames[(frameCount - 1) * RotateTimeline::ENTRIES]);
					break;
				}
				case BONE_TRANSLATE:
				case BONE_SCALE:
				case BONE_SHEAR: {
					TranslateTimeline *timeline;
					float timelineScale = 1;
					if (timelineType == BONE_SCALE) {
						timeline = new ScaleTimeline(frameCount);
					} else if (timelineType == BONE_SHEAR) {
						timeline = new ShearTimeline(frameCount);
					} else {
						timeline = new TranslateTimeline(frameCount);
						timelineScale = scale;
					}
					timeline->_boneIndex = boneIndex;
					for (int frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
						float time = readFloat(input);
						float x = readFloat(input) * timelineScale;
						float y = readFloat(input) * timelineScale;
						timeline->setFrame(frameIndex, time, x, y);
						if (frameIndex < frameCount - 1) {
							readCurve(input, frameIndex, timeline);
						}
					}
					timelines.add(timeline);
					duration = MathUtil::max(duration,
											 timeline->_frames[(frameCount - 1) * TranslateTimeline::ENTRIES]);
					break;
				}
				default: {
					ContainerUtil::cleanUpVectorOfPointers(timelines);
					setError("Invalid timeline type for a bone: ", TCHAR_TO_UTF8(*skeletonData->_bones[boneIndex]->_name.ToString()));
					return NULL;
				}
			}
		}
	}

	// IK timelines.
	for (int i = 0, n = readVarint(input, true); i < n; ++i) {
		int index = readVarint(input, true);
		int frameCount = readVarint(input, true);
		IkConstraintTimeline *timeline = new IkConstraintTimeline(frameCount);
		timeline->_ikConstraintIndex = index;
		for (int frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
			float time = readFloat(input);
			float mix = readFloat(input);
			float softness = readFloat(input) * _scale;
			signed char bendDirection = readSByte(input);
			bool compress = readBoolean(input);
			bool stretch = readBoolean(input);
			timeline->setFrame(frameIndex, time, mix, softness, bendDirection, compress, stretch);
			if (frameIndex < frameCount - 1) {
				readCurve(input, frameIndex, timeline);
			}
		}
		timelines.add(timeline);
		duration = MathUtil::max(duration, timeline->_frames[(frameCount - 1) * IkConstraintTimeline::ENTRIES]);
	}

	// Transform constraint timelines.
	for (int i = 0, n = readVarint(input, true); i < n; ++i) {
		int index = readVarint(input, true);
		int frameCount = readVarint(input, true);
		TransformConstraintTimeline *timeline = new TransformConstraintTimeline(frameCount);
		timeline->_transformConstraintIndex = index;
		for (int frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
			float time = readFloat(input);
			float rotateMix = readFloat(input);
			float translateMix = readFloat(input);
			float scaleMix = readFloat(input);
			float shearMix = readFloat(input);
			timeline->setFrame(frameIndex, time, rotateMix, translateMix, scaleMix, shearMix);
			if (frameIndex < frameCount - 1) {
				readCurve(input, frameIndex, timeline);
			}
		}
		timelines.add(timeline);
		duration = MathUtil::max(duration, timeline->_frames[(frameCount - 1) * TransformConstraintTimeline::ENTRIES]);
	}

	// Path constraint timelines.
	for (int i = 0, n = readVarint(input, true); i < n; ++i) {
		int index = readVarint(input, true);
		PathConstraintData *data = skeletonData->_pathConstraints[index];
		for (int ii = 0, nn = readVarint(input, true); ii < nn; ++ii) {
			int timelineType = readSByte(input);
			int frameCount = readVarint(input, true);
			switch (timelineType) {
				case PATH_POSITION:
				case PATH_SPACING: {
					PathConstraintPositionTimeline *timeline;
					float timelineScale = 1;
					if (timelineType == PATH_SPACING) {
						timeline = new PathConstraintSpacingTimeline(frameCount);

						if (data->_spacingMode == SpacingMode_Length || data->_spacingMode == SpacingMode_Fixed) {
							timelineScale = scale;
						}
					} else {
						timeline = new PathConstraintPositionTimeline(frameCount);

						if (data->_positionMode == PositionMode_Fixed) {
							timelineScale = scale;
						}
					}
					timeline->_pathConstraintIndex = index;
					for (int frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
						float time = readFloat(input);
						float value = readFloat(input) * timelineScale;
						timeline->setFrame(frameIndex, time, value);
						if (frameIndex < frameCount - 1) {
							readCurve(input, frameIndex, timeline);
						}
					}
					timelines.add(timeline);
					duration = MathUtil::max(duration, timeline->_frames[(frameCount - 1) *
																		 PathConstraintPositionTimeline::ENTRIES]);
					break;
				}
				case PATH_MIX: {
					PathConstraintMixTimeline *timeline = new PathConstraintMixTimeline(frameCount);

					timeline->_pathConstraintIndex = index;
					for (int frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
						float time = readFloat(input);
						float rotateMix = readFloat(input);
						float translateMix = readFloat(input);
						timeline->setFrame(frameIndex, time, rotateMix, translateMix);
						if (frameIndex < frameCount - 1) {
							readCurve(input, frameIndex, timeline);
						}
					}
					timelines.add(timeline);
					duration = MathUtil::max(duration,
											 timeline->_frames[(frameCount - 1) * PathConstraintMixTimeline::ENTRIES]);
					break;
				}
			}
		}
	}

	// Deform timelines.
	for (int i = 0, n = readVarint(input, true); i < n; ++i) {
		TSharedPtr<Skin> skin = skeletonData->_skins[readVarint(input, true)];
		for (int ii = 0, nn = readVarint(input, true); ii < nn; ++ii) {
			int slotIndex = readVarint(input, true);
			for (int iii = 0, nnn = readVarint(input, true); iii < nnn; iii++) {
				const char *attachmentName = readStringRef(input, skeletonData);
				TSharedPtr<Attachment> baseAttachment = skin->getAttachment(slotIndex, attachmentName);

				if (!baseAttachment) {
					ContainerUtil::cleanUpVectorOfPointers(timelines);
					setError("Attachment not found: ", attachmentName);
					return NULL;
				}


				TSharedPtr<VertexAttachment> attachment = StaticCastSharedPtr<VertexAttachment> (baseAttachment);

				bool weighted = attachment->_bones.size() > 0;
				Vector<float> &vertices = attachment->_vertices;
				int32 deformLength = weighted ? vertices.size() / 3 * 2
											: vertices.size();

				int32 frameCount = (int32)readVarint(input, true);

				DeformTimeline *timeline = new DeformTimeline(frameCount);

				timeline->_slotIndex = slotIndex;
				timeline->_attachment = attachment.Get();

				for (int32 frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
					float time = readFloat(input);
					Vector<float> deform;
					int32 end = (int32)readVarint(input, true);
					if (end == 0) {
						if (weighted) {
							deform.setSize(deformLength, 0);
							for (int32 iiii = 0; iiii < deformLength; ++iiii) {
								deform[iiii] = 0;
							}
						} else {
							deform.clearAndAddAll(vertices);
						}
					} else {
						deform.setSize(deformLength, 0);
						int32 start = (int32)readVarint(input, true);
						end += start;
						if (scale == 1) {
							for (int32 v = start; v < end; ++v) {
								deform[v] = readFloat(input);
							}
						} else {
							for (int32 v = start; v < end; ++v) {
								deform[v] = readFloat(input) * scale;
							}
						}

						if (!weighted) {
							for (int32 v = 0, vn = deform.size(); v < vn; ++v) {
								deform[v] += vertices[v];
							}
						}
					}

					timeline->setFrame(frameIndex, time, deform);
					if (frameIndex < frameCount - 1) {
						readCurve(input, frameIndex, timeline);
					}
				}

				timelines.add(timeline);
				duration = MathUtil::max(duration, timeline->_frames[frameCount - 1]);
			}
		}
	}

	// Draw order timeline.
	int32 drawOrderCount = (int32)readVarint(input, true);
	if (drawOrderCount > 0) {
		DrawOrderTimeline *timeline = new DrawOrderTimeline(drawOrderCount);

		int32 slotCount = skeletonData->_slots.size();
		for (int32 i = 0; i < drawOrderCount; ++i) {
			float time = readFloat(input);
			int32 offsetCount = (int32)readVarint(input, true);

			Vector<int> drawOrder;
			drawOrder.setSize(slotCount, 0);
			for (int ii = (int)slotCount - 1; ii >= 0; --ii) {
				drawOrder[ii] = -1;
			}

			Vector<int> unchanged;
			unchanged.setSize(slotCount - offsetCount, 0);
			int32 originalIndex = 0, unchangedIndex = 0;
			for (int32 ii = 0; ii < offsetCount; ++ii) {
				int32 slotIndex = (int32)readVarint(input, true);
				// Collect unchanged items.
				while (originalIndex != slotIndex) {
					unchanged[unchangedIndex++] = originalIndex++;
				}
				// Set changed items.
				int32 index = originalIndex;
				drawOrder[index + (int32)readVarint(input, true)] = originalIndex++;
			}

			// Collect remaining unchanged items.
			while (originalIndex < slotCount) {
				unchanged[unchangedIndex++] = originalIndex++;
			}

			// Fill in unchanged items.
			for (int ii = (int)slotCount - 1; ii >= 0; --ii) {
				if (drawOrder[ii] == -1) {
					drawOrder[ii] = unchanged[--unchangedIndex];
				}
			}
			timeline->setFrame(i, time, drawOrder);
		}
		timelines.add(timeline);
		duration = MathUtil::max(duration, timeline->_frames[drawOrderCount - 1]);
	}

	// Event timeline.
	int eventCount = readVarint(input, true);
	if (eventCount > 0) {
		EventTimeline *timeline = new EventTimeline(eventCount);

		for (int i = 0; i < eventCount; ++i) {
			float time = readFloat(input);
			EventData *eventData = skeletonData->_events[readVarint(input, true)];
			Event *event = new Event(time, *eventData);

			event->_intValue = readVarint(input, false);
			event->_floatValue = readFloat(input);
			bool freeString = readBoolean(input);
			const char *event_stringValue = freeString ? readString(input) : eventData->_stringValue.buffer();
			event->_stringValue = String(event_stringValue);
			if (freeString) {
				SpineExtension::free(event_stringValue, __FILE__, __LINE__);
			}

			if (!eventData->_audioPath.isEmpty()) {
				event->_volume = readFloat(input);
				event->_balance = readFloat(input);
			}
			timeline->setFrame(i, event);
		}

		timelines.add(timeline);
		duration = MathUtil::max(duration, timeline->_frames[eventCount - 1]);
	}

	return new Animation(String(name), timelines, duration);
}

void SkeletonBinary::readCurve(DataInput *input, int frameIndex, CurveTimeline *timeline) {
	switch (readByte(input)) {
		case CURVE_STEPPED: {
			timeline->setStepped(frameIndex);
			break;
		}
		case CURVE_BEZIER: {
			float cx1 = readFloat(input);
			float cy1 = readFloat(input);
			float cx2 = readFloat(input);
			float cy2 = readFloat(input);
			timeline->setCurve(frameIndex, cx1, cy1, cx2, cy2);
			break;
		}
	}
}
