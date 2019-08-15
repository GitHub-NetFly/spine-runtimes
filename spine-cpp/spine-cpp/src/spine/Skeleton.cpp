/******************************************************************************
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



#include <spine/Skeleton.h>

#include <spine/SkeletonData.h>
#include <spine/Bone.h>
#include <spine/Slot.h>
#include <spine/IkConstraint.h>
#include <spine/PathConstraint.h>
#include <spine/TransformConstraint.h>
#include <spine/Skin.h>
#include <spine/Attachment.h>

#include <spine/BoneData.h>
#include <spine/SlotData.h>
#include <spine/IkConstraintData.h>
#include <spine/TransformConstraintData.h>
#include <spine/PathConstraintData.h>
#include <spine/RegionAttachment.h>
#include <spine/MeshAttachment.h>
#include <spine/PathAttachment.h>

#include <spine/ContainerUtil.h>

using namespace spine;

Skeleton::Skeleton(TSharedRef<SkeletonData> InSkeletonData) :
		_data(InSkeletonData),
		_skin(NULL),
		_color(1, 1, 1, 1),
		_time(0),
		_scaleX(1),
		_scaleY(1),
		_x(0),
		_y(0) {
	_bones.ensureCapacity(_data->getBones().size());
	for (int32 i = 0; i < _data->getBones().size(); ++i) {
		BoneData *data = _data->getBones()[i];

		Bone *bone;
		if (data->getParent() == NULL) {
			bone = new Bone(*data, *this, NULL);
		} else {
			Bone *parent = _bones[data->getParent()->getIndex()];
			bone = new Bone(*data, *this, parent);
			parent->getChildren().add(bone);
		}

		_bones.add(bone);
	}

	_slots.Reserve(_data->getSlots().size());

	_drawOrder.Reserve(_data->getSlots().size());
	for (int32 i = 0; i < _data->getSlots().size(); ++i) {
		SlotData *data = _data->getSlots()[i];

		Bone *bone = _bones[data->getBoneData().getIndex()];
		TSharedPtr<Slot> slot = MakeShared<Slot>(*data, *bone);

		_slots.Add(slot);
		_drawOrder.Add(slot);
	}

	_ikConstraints.ensureCapacity(_data->getIkConstraints().size());
	for (int32 i = 0; i < _data->getIkConstraints().size(); ++i) {
		IkConstraintData *data = _data->getIkConstraints()[i];

		IkConstraint *constraint = new IkConstraint(*data, *this);

		_ikConstraints.add(constraint);
	}

	_transformConstraints.ensureCapacity(_data->getTransformConstraints().size());
	for (int32 i = 0; i < _data->getTransformConstraints().size(); ++i) {
		TransformConstraintData *data = _data->getTransformConstraints()[i];

		TransformConstraint *constraint = new TransformConstraint(*data, *this);

		_transformConstraints.add(constraint);
	}

	_pathConstraints.ensureCapacity(_data->getPathConstraints().size());
	for (int32 i = 0; i < _data->getPathConstraints().size(); ++i) {
		PathConstraintData *data = _data->getPathConstraints()[i];

		PathConstraint *constraint = new PathConstraint(*data, *this);

		_pathConstraints.add(constraint);
	}

	updateCache();
}

Skeleton::~Skeleton() {
	ContainerUtil::cleanUpVectorOfPointers(_bones);
	
	ContainerUtil::cleanUpVectorOfPointers(_ikConstraints);
	ContainerUtil::cleanUpVectorOfPointers(_transformConstraints);
	ContainerUtil::cleanUpVectorOfPointers(_pathConstraints);
}

void Skeleton::updateCache() {
	_updateCache.clear();
	_updateCacheReset.clear();

	for (size_t i = 0, n = _bones.size(); i < n; ++i) {
		Bone* bone = _bones[i];
		bone->_sorted = bone->_data.isSkinRequired();
		bone->_active = !bone->_sorted;
	}

	if (_skin) {
		Vector<BoneData*>& skinBones = _skin->getBones();
		for (size_t i = 0, n = skinBones.size(); i < n; i++) {
			Bone* bone = _bones[skinBones[i]->getIndex()];
			do {
				bone->_sorted = false;
				bone->_active = true;
				bone = bone->_parent;
			} while (bone);
		}
	}

	int32 ikCount = _ikConstraints.size();
	int32 transformCount = _transformConstraints.size();
	int32 pathCount = _pathConstraints.size();

	int32 constraintCount = ikCount + transformCount + pathCount;

	int32 i = 0;
	continue_outer:
	for (; i < constraintCount; ++i) {
		for (int32 ii = 0; ii < ikCount; ++ii) {
			IkConstraint *constraint = _ikConstraints[ii];
			if (constraint->getData().getOrder() == i) {
				sortIkConstraint(constraint);
				i++;
				goto continue_outer;
			}
		}

		for (int32 ii = 0; ii < transformCount; ++ii) {
			TransformConstraint *constraint = _transformConstraints[ii];
			if (constraint->getData().getOrder() == i) {
				sortTransformConstraint(constraint);
				i++;
				goto continue_outer;
			}
		}

		for (int32 ii = 0; ii < pathCount; ++ii) {
			PathConstraint *constraint = _pathConstraints[ii];
			if (constraint->getData().getOrder() == i) {
				sortPathConstraint(constraint);
				i++;
				goto continue_outer;
			}
		}
	}

	int32 n = _bones.size();
	for (i = 0; i < n; ++i) {
		sortBone(_bones[i]);
	}
}

void Skeleton::printUpdateCache() {
	for (int32 i = 0; i < _updateCache.size(); i++) {
		Updatable *updatable = _updateCache[i];
		if (updatable->getRTTI().isExactly(Bone::rtti)) {
			printf("bone %s\n", TCHAR_TO_UTF8(*((Bone *)updatable)->getData().getName().ToString()));
		} else if (updatable->getRTTI().isExactly(TransformConstraint::rtti)) {
			printf("transform constraint %s\n", ((TransformConstraint *) updatable)->getData().getName().buffer());
		} else if (updatable->getRTTI().isExactly(IkConstraint::rtti)) {
			printf("ik constraint %s\n", ((IkConstraint *) updatable)->getData().getName().buffer());
		} else if (updatable->getRTTI().isExactly(PathConstraint::rtti)) {
			printf("path constraint %s\n", ((PathConstraint *) updatable)->getData().getName().buffer());
		}
	}
}

void Skeleton::updateWorldTransform() {
	for (int32 i = 0, n = _updateCacheReset.size(); i < n; ++i) {
		Bone *boneP = _updateCacheReset[i];
		Bone &bone = *boneP;
		bone._ax = bone._x;
		bone._ay = bone._y;
		bone._arotation = bone._rotation;
		bone._ascaleX = bone._scaleX;
		bone._ascaleY = bone._scaleY;
		bone._ashearX = bone._shearX;
		bone._ashearY = bone._shearY;
		bone._appliedValid = true;
	}

	for (int32 i = 0, n = _updateCache.size(); i < n; ++i) {
		_updateCache[i]->update();
	}
}

void Skeleton::setToSetupPose() {
	setBonesToSetupPose();
	setSlotsToSetupPose();
}

void Skeleton::setBonesToSetupPose() {
	for (int32 i = 0, n = _bones.size(); i < n; ++i) {
		_bones[i]->setToSetupPose();
	}

	for (int32 i = 0, n = _ikConstraints.size(); i < n; ++i) {
		IkConstraint *constraintP = _ikConstraints[i];
		IkConstraint &constraint = *constraintP;

		constraint._bendDirection = constraint._data._bendDirection;
		constraint._compress = constraint._data._compress;
		constraint._stretch = constraint._data._stretch;
		constraint._mix = constraint._data._mix;
		constraint._softness = constraint._data._softness;
	}

	for (int32 i = 0, n = _transformConstraints.size(); i < n; ++i) {
		TransformConstraint *constraintP = _transformConstraints[i];
		TransformConstraint &constraint = *constraintP;
		TransformConstraintData &constraintData = constraint._data;

		constraint._rotateMix = constraintData._rotateMix;
		constraint._translateMix = constraintData._translateMix;
		constraint._scaleMix = constraintData._scaleMix;
		constraint._shearMix = constraintData._shearMix;
	}

	for (int32 i = 0, n = _pathConstraints.size(); i < n; ++i) {
		PathConstraint *constraintP = _pathConstraints[i];
		PathConstraint &constraint = *constraintP;
		PathConstraintData &constraintData = constraint._data;

		constraint._position = constraintData._position;
		constraint._spacing = constraintData._spacing;
		constraint._rotateMix = constraintData._rotateMix;
		constraint._translateMix = constraintData._translateMix;
	}
}

void Skeleton::setSlotsToSetupPose()
{
	_drawOrder.Empty();

	for (int32 i = 0, n = _slots.Num(); i < n; ++i) {
		_drawOrder.Add(_slots[i]);
	}

	for (int32 i = 0, n = _slots.Num(); i < n; ++i) {
		_slots[i]->setToSetupPose();
	}
}

Bone *Skeleton::findBone(const FName &boneName)
{
	return ContainerUtil::findWithDataName(_bones, boneName);
}

int Skeleton::findBoneIndex(const FName &boneName) {
	return ContainerUtil::findIndexWithDataName(_bones, boneName);
}

TSharedPtr<Slot> Skeleton::findSlot(const FString &slotName)
{
	auto FoundPtr = _slots.FindByPredicate([&slotName](TSharedPtr<Slot> InSlot) { return InSlot->getData().getName() == slotName;  });

	return FoundPtr ? *FoundPtr : nullptr;
}

int32 Skeleton::findSlotIndex(const FString &slotName)
{

	for (int32 i = 0, len = _slots.Num(); i < len; ++i) 
	{
		if (_slots[i]->getData().getName() == slotName)
		{
			return i;
		}
	}

	return INDEX_NONE;
}

void Skeleton::setSkin(const FString &skinName) 
{
	TSharedPtr<Skin> foundSkin = _data->findSkin(skinName);

	assert(foundSkin != NULL);

	setSkin(foundSkin);
}

void Skeleton::setSkin(TSharedPtr<Skin> newSkin)
{
	if (_skin== newSkin)
	{
		return;
	}

	if (newSkin)
	{
		if (_skin)
		{
			Skeleton &thisRef = *this;
			newSkin->attachAll(thisRef, *_skin);
		}
		else 
		{
			for (int32 i = 0, n = _slots.Num(); i < n; ++i) 
			{
				TSharedPtr<Slot> slotP = _slots[i];
				Slot &slot = *slotP;
				const FString &name = slot._data.getAttachmentName();
				if (!name.IsEmpty()) 
				{
					TSharedPtr<Attachment> attachment = newSkin->getAttachment(i, name);
					if (attachment) {
						slot.setAttachment(attachment.Get());
					}
				}
			}
		}
	}

	_skin = newSkin;
	updateCache();
}

TSharedPtr<Attachment> Skeleton::getAttachment(const FString &slotName, const FString &attachmentName) {
	return getAttachment(_data->findSlotIndex(slotName), attachmentName);
}

TSharedPtr<Attachment> Skeleton::getAttachment(int slotIndex, const FString &attachmentName) {
	assert(attachmentName.length() > 0);

	if (_skin ) 
	{
		TSharedPtr<Attachment> attachment = _skin->getAttachment(slotIndex, attachmentName);
		if (attachment )
		{
			return attachment;
		}
	}

	return _data->getDefaultSkin()  ? _data->getDefaultSkin()->getAttachment(slotIndex, attachmentName) : NULL;
}

void Skeleton::setAttachment(const FString &slotName, const FString &attachmentName) {
	assert(slotName.length() > 0);

	for (int32 i = 0, n = _slots.Num(); i < n; ++i) {
		TSharedPtr<Slot> slot = _slots[i];
		if (slot->_data.getName() == slotName) {
			TSharedPtr<Attachment> attachment ;
			if (!attachmentName.IsEmpty()) 
			{
				attachment = getAttachment(i, attachmentName);

				assert(attachment != NULL);
			}

			slot->setAttachment(attachment.Get());

			return;
		}
	}

	printf("Slot not found: %s", TCHAR_TO_UTF8(*slotName));

	assert(false);
}

IkConstraint *Skeleton::findIkConstraint(const String &constraintName) {
	assert(constraintName.length() > 0);

	for (int32 i = 0, n = _ikConstraints.size(); i < n; ++i) {
		IkConstraint *ikConstraint = _ikConstraints[i];
		if (ikConstraint->_data.getName() == constraintName) {
			return ikConstraint;
		}
	}
	return NULL;
}

TransformConstraint *Skeleton::findTransformConstraint(const String &constraintName) {
	assert(constraintName.length() > 0);

	for (int32 i = 0, n = _transformConstraints.size(); i < n; ++i) {
		TransformConstraint *transformConstraint = _transformConstraints[i];
		if (transformConstraint->_data.getName() == constraintName) {
			return transformConstraint;
		}
	}

	return NULL;
}

PathConstraint *Skeleton::findPathConstraint(const String &constraintName) {
	assert(constraintName.length() > 0);

	for (int32 i = 0, n = _pathConstraints.size(); i < n; ++i) {
		PathConstraint *constraint = _pathConstraints[i];
		if (constraint->_data.getName() == constraintName) {
			return constraint;
		}
	}

	return NULL;
}

void Skeleton::update(float delta) {
	_time += delta;
}

void Skeleton::getBounds(float &outX, float &outY, float &outWidth, float &outHeight, Vector<float> &outVertexBuffer) {
	float minX = std::numeric_limits<float>::max();
	float minY = std::numeric_limits<float>::max();
	float maxX = std::numeric_limits<float>::min();
	float maxY = std::numeric_limits<float>::min();

	for (size_t i = 0; i < _drawOrder.Num(); ++i) {
		TSharedPtr<Slot> slot = _drawOrder[i];
		if (!slot->_bone._active) continue;
		size_t verticesLength = 0;
		Attachment *attachment = slot->getAttachment();

		if (attachment != NULL && attachment->getRTTI().instanceOf(RegionAttachment::rtti)) {
			RegionAttachment *regionAttachment = static_cast<RegionAttachment *>(attachment);

			verticesLength = 8;
			if (outVertexBuffer.size() < 8) {
				outVertexBuffer.setSize(8, 0);
			}
			regionAttachment->computeWorldVertices(slot->getBone(), outVertexBuffer, 0);
		} else if (attachment != NULL && attachment->getRTTI().instanceOf(MeshAttachment::rtti)) {
			MeshAttachment *mesh = static_cast<MeshAttachment *>(attachment);

			verticesLength = mesh->getWorldVerticesLength();
			if (outVertexBuffer.size() < verticesLength) {
				outVertexBuffer.setSize(verticesLength, 0);
			}

			mesh->computeWorldVertices(*slot, 0, verticesLength, outVertexBuffer, 0);
		}

		for (int32 ii = 0; ii < verticesLength; ii += 2) {
			float vx = outVertexBuffer[ii];
			float vy = outVertexBuffer[ii + 1];

			minX = MathUtil::min(minX, vx);
			minY = MathUtil::min(minY, vy);
			maxX = MathUtil::max(maxX, vx);
			maxY = MathUtil::max(maxY, vy);
		}
	}

	outX = minX;
	outY = minY;
	outWidth = maxX - minX;
	outHeight = maxY - minY;
}

Bone *Skeleton::getRootBone() {
	return _bones.size() == 0 ? NULL : _bones[0];
}

TSharedRef<SkeletonData> Skeleton::getData() {
	return _data;
}

Vector<Bone *> &Skeleton::getBones() {
	return _bones;
}

Vector<Updatable *> &Skeleton::getUpdateCacheList() {
	return _updateCache;
}

TArray<TSharedPtr<Slot>> &Skeleton::getSlots() {
	return _slots;
}

TArray<TSharedPtr<Slot>>  &Skeleton::getDrawOrder() {
	return _drawOrder;
}

Vector<IkConstraint *> &Skeleton::getIkConstraints() {
	return _ikConstraints;
}

Vector<PathConstraint *> &Skeleton::getPathConstraints() {
	return _pathConstraints;
}

Vector<TransformConstraint *> &Skeleton::getTransformConstraints() {
	return _transformConstraints;
}

TSharedPtr<Skin> Skeleton::getSkin() {
	return _skin;
}

Color &Skeleton::getColor() {
	return _color;
}

float Skeleton::getTime() {
	return _time;
}

void Skeleton::setTime(float inValue) {
	_time = inValue;
}

void Skeleton::setPosition(float x, float y) {
	_x = x;
	_y = y;
}

float Skeleton::getX() {
	return _x;
}

void Skeleton::setX(float inValue) {
	_x = inValue;
}

float Skeleton::getY() {
	return _y;
}

void Skeleton::setY(float inValue) {
	_y = inValue;
}

float Skeleton::getScaleX() {
	return _scaleX;
}

void Skeleton::setScaleX(float inValue) {
	_scaleX = inValue;
}

float Skeleton::getScaleY() {
	return _scaleY * (Bone::isYDown() ? -1 : 1);
}

void Skeleton::setScaleY(float inValue) {
	_scaleY = inValue;
}

void Skeleton::sortIkConstraint(IkConstraint *constraint) {
	constraint->_active = constraint->_target->_active && (!constraint->_data.isSkinRequired() || (_skin && _skin->_constraints.contains(&constraint->_data)));
	if (!constraint->_active) return;

	Bone *target = constraint->getTarget();
	sortBone(target);

	Vector<Bone *> &constrained = constraint->getBones();
	Bone *parent = constrained[0];
	sortBone(parent);

	if (constrained.size() > 1) {
		Bone *child = constrained[constrained.size() - 1];
		if (!_updateCache.contains(child)) _updateCacheReset.add(child);
	}

	_updateCache.add(constraint);

	sortReset(parent->getChildren());
	constrained[constrained.size() - 1]->_sorted = true;
}

void Skeleton::sortPathConstraint(PathConstraint *constraint) {
	constraint->_active = constraint->_target->_bone._active && (!constraint->_data.isSkinRequired() || (_skin && _skin->_constraints.contains(&constraint->_data)));
	if (!constraint->_active) return;

	Slot *slot = constraint->getTarget();
	int slotIndex = slot->getData().getIndex();
	Bone &slotBone = slot->getBone();
	if (_skin ) sortPathConstraintAttachment(_skin.Get(), slotIndex, slotBone);
	if (_data->_defaultSkin  && _data->_defaultSkin != _skin)
		sortPathConstraintAttachment(_data->_defaultSkin.Get(), slotIndex, slotBone);
	for (int32 ii = 0, nn = _data->_skins.Num(); ii < nn; ii++)
		sortPathConstraintAttachment(_data->_skins[ii].Get(), slotIndex, slotBone);

	Attachment *attachment = slot->getAttachment();
	if (attachment != NULL && attachment->getRTTI().instanceOf(PathAttachment::rtti))
		sortPathConstraintAttachment(attachment, slotBone);

	Vector<Bone *> &constrained = constraint->getBones();
	int32 boneCount = constrained.size();
	for (int32 i = 0; i < boneCount; ++i) {
		sortBone(constrained[i]);
	}

	_updateCache.add(constraint);

	for (int32 i = 0; i < boneCount; i++)
		sortReset(constrained[i]->getChildren());
	for (int32 i = 0; i < boneCount; i++)
		constrained[i]->_sorted = true;
}

void Skeleton::sortTransformConstraint(TransformConstraint *constraint) {
	constraint->_active = constraint->_target->_active && (!constraint->_data.isSkinRequired() || (_skin && _skin->_constraints.contains(&constraint->_data)));
	if (!constraint->_active) return;

	sortBone(constraint->getTarget());

	Vector<Bone *> &constrained = constraint->getBones();
	int32 boneCount = constrained.size();
	if (constraint->_data.isLocal()) {
		for (int32 i = 0; i < boneCount; i++) {
			Bone *child = constrained[i];
			sortBone(child->getParent());
			if (!_updateCache.contains(child)) _updateCacheReset.add(child);
		}
	} else {
		for (int32 i = 0; i < boneCount; ++i) {
			sortBone(constrained[i]);
		}
	}

	_updateCache.add(constraint);

	for (int32 i = 0; i < boneCount; ++i)
		sortReset(constrained[i]->getChildren());
	for (int32 i = 0; i < boneCount; ++i)
		constrained[i]->_sorted = true;
}

void Skeleton::sortPathConstraintAttachment(Skin *skin, int32 slotIndex, Bone &slotBone) {
	Skin::AttachmentMap::Entries attachments = skin->getAttachments();

	while (attachments.hasNext()) {
		Skin::AttachmentMap::Entry entry = attachments.next();
		if (entry._slotIndex == slotIndex) {
			TSharedPtr<Attachment> value = entry._attachment;
			sortPathConstraintAttachment(value.Get(), slotBone);
		}
	}
}

void Skeleton::sortPathConstraintAttachment(Attachment *attachment, Bone &slotBone) {
	if (attachment == NULL || !attachment->getRTTI().instanceOf(PathAttachment::rtti)) return;
	Vector<int32> &pathBones = static_cast<PathAttachment *>(attachment)->getBones();
	if (pathBones.size() == 0)
		sortBone(&slotBone);
	else {
		for (int32 i = 0, n = pathBones.size(); i < n;) {
			int32 nn = pathBones[i++];
			nn += i;
			while (i < nn) {
				sortBone(_bones[pathBones[i++]]);
			}
		}
	}
}

void Skeleton::sortBone(Bone *bone) {
	if (bone->_sorted) return;
	Bone *parent = bone->_parent;
	if (parent != NULL) sortBone(parent);
	bone->_sorted = true;
	_updateCache.add(bone);
}

void Skeleton::sortReset(Vector<Bone *> &bones) {
	for (int32 i = 0, n = bones.size(); i < n; ++i) {
		Bone *bone = bones[i];
		if (!bone->_active) continue;
		if (bone->_sorted) sortReset(bone->getChildren());
		bone->_sorted = false;
	}
}
